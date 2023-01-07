#include "process_.h"
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <utility>

Process::Process() : hwnd(nullptr), pid(0), handle(nullptr)
{
}

Process::~Process()
{
    if (IsValid()) {
        CloseHandle(handle);
    }
}

__stdcall int Process::SearchPvzWindowFn(HWND hwnd, LPARAM param)
{
    auto [out, strict] = *(std::pair<HWND*, bool>*)param;
    char buf[256];
    GetClassNameA(hwnd, buf, sizeof(buf) - 1);
    if (strcmp(buf, "MainWindow") != 0)
    {
        return true;
    }
    int len = GetWindowTextA(hwnd, buf, sizeof(buf) - 1);
    if (len < 18 || (strict && len > 18))
    {
        return true;
    }
    if (strncmp(buf, "Plants vs. Zombies", 18) == 0)
    {
        *out = hwnd;
        return false;
    }
    return true;
}

void Process::SearchPvzWindow()
{
    std::pair<HWND*, bool> args = {&hwnd, true};
    EnumWindows(SearchPvzWindowFn, (LPARAM)&args);
    if (!hwnd)
    {
        std::pair<HWND*, bool> args = {&hwnd, false};
        EnumWindows(SearchPvzWindowFn, (LPARAM)&args);
    }
}

bool Process::OpenWindow()
{
    if (IsValid()) {
        CloseHandle(handle);
    }
    Process::SearchPvzWindow();

    while (hwnd == nullptr) {
        MessageBoxW(NULL, L"您是否未打开游戏? (注意必须是英文原版，Steam 版也是不可以的！)", L"Warning", MB_ICONWARNING);
        Process::SearchPvzWindow();
    }

    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != 0)
    {
        handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    }

    if (ReadMemory<uint32_t>(0x4140c5) != 0x0019b337)
    {
        MessageBoxW(NULL, L"您使用的游戏版本不是英文原版，请到下载安装包的链接下载本框架所支持的英文原版", L"Error", MB_ICONERROR);
        return false;
    }

    auto address = ReadMemory<uintptr_t>(0x6a9ec0);
    auto game_ui = ReadMemory<int>(address + 0x7fc);
    while (game_ui == 2 || game_ui == 3)
    {
        MessageBoxW(NULL, L"检测到游戏窗口在选卡或战斗界面，这种行为可能会导致注入失败，请在游戏主界面进行注入", L"Warning", MB_ICONWARNING);
        game_ui = ReadMemory<int>(address + 0x7fc);
    }

    return hwnd;
}

void Process::ManageDLL(const wchar_t* path)
{
    std::filesystem::path dllPath = path;
    if (!std::filesystem::exists(path)) {
        MessageBoxW(NULL, L"DLL 不存在", L"Error", MB_ICONERROR);
        return;
    }

    char buf[256];
    int len = GetWindowTextA(hwnd, buf, sizeof(buf) - 1);
    if(len > 18)
    {
        std::string oldDllName(buf + 20, buf + len - 1);
        oldDllName += ".dll";
        EjectDLL(oldDllName.c_str());
    }

    if (!InjectDLL(dllPath.c_str())) {
        MessageBoxW(NULL, L"DLL 注入失败，失败可能原因如下\n 1. 计算机开启了杀软，此行为被杀软拦截\n 2. DLL 所在路径需要管理员权限才可以进行文件的生成和复制", L"Error", MB_ICONERROR);
        return;
    }

    std::wstring new_title = L"Plants vs. Zombies (" + dllPath.stem().wstring() + L")";
    SetWindowTextW(hwnd, new_title.c_str());
}

DWORD Process::EjectDLL(const char* szDllName)
{
    // Copy from Internet =_=
    BOOL bMore = FALSE, bFound = FALSE;
    HANDLE hSnapshot, hProcess, hThread;
    HMODULE hModule = NULL;
    MODULEENTRY32 me = {sizeof(me)};
    LPTHREAD_START_ROUTINE pThreadProc;
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    bMore = Module32First(hSnapshot, &me);
    for (; bMore; bMore = Module32Next(hSnapshot, &me)) {
        if (!strcmp(me.szModule, szDllName) || !strcmp(me.szExePath, szDllName)) {
            bFound = TRUE;
            break;
        }
    }
    if (!bFound) {
        CloseHandle(hSnapshot);
        return FALSE;
    }
    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid))) {
        wprintf(L"OpenProcess(%d) failed!!! [%d]\n,", pid, GetLastError());
        return FALSE;
    }
    hModule = GetModuleHandle("Kernel32.dll");
    pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "FreeLibrary");
    hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    CloseHandle(hSnapshot);
    return TRUE;
}

DWORD Process::InjectDLL(const wchar_t* pszLibFile)
{
    // Calculate the number of bytes needed for the DLL's pathname
    DWORD dwSize = (lstrlenW(pszLibFile) + 1) * sizeof(wchar_t);
    if (handle == NULL) {
        wprintf(L"[-] Error: Could not open process for PID (%d).\n", pid);
        return FALSE;
    }

    // Allocate space in the remote process for the pathname
    LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(handle, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (pszLibFileRemote == NULL) {
        wprintf(L"[-] Error: Could not allocate memory inside PID (%d).\n", pid);
        return FALSE;
    }

    // Copy the DLL's pathname to the remote process address space
    DWORD n = WriteProcessMemory(handle, pszLibFileRemote, (PVOID)pszLibFile, dwSize, NULL);
    if (n == 0) {
        wprintf(L"[-] Error: Could not write any bytes into the PID [%d] address space.\n", pid);
        return FALSE;
    }

    // // Get the real address of LoadLibraryW in Kernel32.dll
    // LPTHREAD_START_ROUTINE pfnThreadRtn = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    // if (pfnThreadRtn == NULL) {
    //     wprintf(L"[-] Error: Could not find LoadLibraryW function inside kernel32.dll library.\n");
    //     return FALSE;
    // }

    // Create a remote thread that calls LoadLibraryW(DLLPathname)
    HANDLE hThread = CreateRemoteThread(handle, NULL, 0, LPTHREAD_START_ROUTINE(LoadLibraryW), pszLibFileRemote, 0, NULL);
    if (hThread == NULL) {
        wprintf(L"[-] Error: Could not create the Remote Thread.\n");
        return FALSE;
    }

    // Wait for the remote thread to terminate
    WaitForSingleObject(hThread, INFINITE);

    // Free the remote memory that contained the DLL's pathname and close Handles
    if (pszLibFileRemote != NULL) {
        VirtualFreeEx(handle, pszLibFileRemote, 0, MEM_RELEASE);
    }

    if (hThread != NULL) {
        CloseHandle(hThread);
    }

    return TRUE;
}

void Process::Write(uintptr_t addr, size_t len, uint8_t* data)
{
    WriteProcessMemory(handle, (void*)addr, data, len, nullptr);
}

bool Process::IsValid()
{
    if (handle == nullptr) {
        return false;
    }
    DWORD exit_code;
    GetExitCodeProcess(handle, &exit_code);
    bool valid = (exit_code == STILL_ACTIVE);

#ifdef _DEBUG
    if (!valid)
        std::cout << "Not Valid" << std::endl;
#endif

    return valid;
}

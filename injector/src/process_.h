#pragma once
#include <cstdint>
#include <initializer_list>
#include <Windows.h>

class Process {
private:
    HWND hwnd;
    DWORD pid;
    HANDLE handle;
    static __stdcall int SearchPvzWindowFn(HWND, LPARAM);
    void SearchPvzWindow();

public:
    Process();
    ~Process();

    bool OpenWindow();
    bool IsValid();

    DWORD InjectDLL(const wchar_t*);
    DWORD EjectDLL(const char*);
    void ManageDLL(const wchar_t*);
    void ExamineEnvironment();

    void Write(uintptr_t addr, size_t len, uint8_t* data);

    template <typename T, typename... Args>
    T ReadMemory(Args... args)
    {
        std::initializer_list<uintptr_t> lst = {static_cast<uintptr_t>(args)...};
        uintptr_t buff = 0;
        T result = T();
        for (auto it = lst.begin(); it != lst.end(); ++it)
            if (it != lst.end() - 1)
                ReadProcessMemory(handle, (const void*)(buff + *it), &buff, sizeof(buff), nullptr);
            else
                ReadProcessMemory(handle, (const void*)(buff + *it), &result, sizeof(result), nullptr);
        return result;
    }
};

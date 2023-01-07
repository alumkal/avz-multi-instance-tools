#include <filesystem>
#include <string>
#include "Windows.h"
#include "process_.h"

namespace fs = std::filesystem;

std::wstring get_filename()
{
    for (int i = 1; ; ++i)
    {
        std::wstring filename = L"temp\\" + std::to_wstring(i) + L".dll";
        if (!fs::exists(filename))
            return filename;
    }
}

void inject(const wchar_t* fn)
{
    fs::path path = fs::absolute(fn);
    Process process;
    if (process.OpenWindow())
    {
        process.ManageDLL(path.c_str());
    }
}

int main()
{
    int argc;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    fs::current_path(fs::path(argv[0]).parent_path());
    if (argc == 1)
    {
        std::error_code _;
        fs::create_directory("temp", _);
        for (auto file : fs::directory_iterator("temp"))
        {
            fs::remove(file, _);
        }
        std::wstring filename = get_filename();
        fs::copy("libavz.dll", filename, _);
        inject(filename.c_str());
    }
    for (int i = 1; i < argc; ++i)
    {
        inject(argv[i]);
    }
    return 0;
}

/*
MIT License

Copyright (c) 2023 Reisen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#Requires AutoHotkey v2.0
#SingleInstance Force
^Q::ExitApp(0)

ReadMemory(hwnd, address) {
    pid := WinGetPID("ahk_id " hwnd)
    handle := DllCall("OpenProcess", "Int", 16, "Int", 0, "UInt", pid)
    value := 0
    for x in address {
        buf := Buffer(4)
        DllCall("ReadProcessMemory", "UInt", handle, "UInt", value + x, "Ptr", buf, "UInt", 4, "UInt", 0)
        value := NumGet(Buf, "UInt")
    }
    return value
}

Check() {
    SetTitleMatchMode("RegEx")
    windows := WinGetList("Plants vs\. Zombies( \(.*\))?")
    SetTitleMatchMode(3)
    for hwnd in windows
        if ReadMemory(hwnd, [0x6a9ec0, 0x7fc]) = 4 {
            title := WinGetTitle("ahk_id " hwnd)
            if StrLen(title) > 18
                name := SubStr(title, 21, -1) " "
            else
                name := ""
            flag := ReadMemory(hwnd, [0x6a9ec0, 0x768, 0x160, 0x6c]) * 2
            FileAppend(Format("[{1}] {2}restart at {3}f`n", A_Now, name, flag), "log.txt")
            Sleep(1500 * ReadMemory(hwnd, [0x6a9ec0, 0x454]))
            WinActivate(title)
            Click("700 10")
            Sleep(100)
            Click("400 450")
            Sleep(100)
            Click("280 370")
            break
        }
}

SetTimer(Check, 1000)

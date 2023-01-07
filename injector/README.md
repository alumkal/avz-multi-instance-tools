# AvZ 多实例注入器

AvZ 自带 injector 的修改版，允许 AvZ DLL 被同时注入到多个 PvZ 实例中并行运行。它可以用于注入现有 DLL，也可以作为 AvZ（不限版本）的自带 injector 的直接替换。 

可以用于对冲关脚本的并行验证，~~也可以在直播挂机的时候用，充分利用空间。~~

## 使用方法

- 直接运行：模仿 AvZ 自带 injector 的行为，注入 `libavz.dll`
- 注入现有 DLL（图形化）：在文件资源管理器内将一个或多个 DLL 文件拖动到注入器 `.exe` 文件上
- 注入现有 DLL（命令行）：`injector.exe path\to\a.dll path\to\b.dll`

注入时如果存在未被注入的 PvZ 窗口，则注入到其中一个；若不存在，则随机覆盖一个。多次重复注入即可将 DLL 注入到每个窗口中。

## 编译方法

`C:\path\to\avz2\MinGW\bin\clang++.exe -std=c++20 -m32 -static -O2 src\*.cpp`。

`bin/` 下有预编译的二进制文件。

## 许可

Copyright © 2020-2022  vector-wlc, Reisen
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

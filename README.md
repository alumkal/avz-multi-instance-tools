# AvZ 多开工具

本项目包含两个工具：

1. AvZ 多实例注入器

2. 自动 SL

具体介绍见各自文件夹中的 `README.md`。

**注意，大多数修改器无法同时对多个 PvZ 实例生效，因此需要的修改器功能要在 AvZ 脚本中实现。**

一些常用的修改：

```cpp
// 存档只读
*(uint8_t*)0x482149 = 0x2e;
*(uint8_t*)0x54b267 = 0x70;

// 开雾
*(uint16_t*)0x41a68d = 0xd231;
```

更多功能的代码可在[PvZ Tools 源代码](https://github.com/lmintlcx/pvztools)中查找。

# Litematic_V7_To_V6
在尽可能保留方块、方块实体、实体等其它信息的情况下，  
降低投影数据版本(投影降级)，从 V7(MC1.20.5+) 转换到 V6(MC1.20.4-)   

本项目代码参考了[投影Mod](https://github.com/sakura-ryoko/litematica)的部分代码  
使用的NBT库为：[NBT_CPP](https://github.com/chenjunfu2/NBT_CPP/)  
其它库依赖：[zlib](https://github.com/madler/zlib)和[xxhash](https://github.com/Cyan4973/xxHash)  

## 使用方法
将一个或多个需要降低数据版本的投影文件
- 拖拽到程序上，然后松开
- 作为启动命令参数输入，回车
  
即可完成  
  
## 1.20.x-Mod版本
详情请见：[litematica-extra](https://github.com/shuangshun/litematica-extra)  
  
## 跨平台
**Release仅提供Windows x64构建版本，Linux用户请自行构建**  
**其他平台暂时不考虑兼容，如有需求请自行尝试**  
  
`Linux`请使用`Clang`或`GCC`通过`CMakeLists.txt`构建  
`Windows`可使用`MSVC`通过`*.sln`构建，或使用与`Linux`相同的构建方式  
  
## Star History
[![Star History Chart](https://api.star-history.com/image?repos=chenjunfu2/Litematic_V7_To_V6&type=date&legend=top-left)](https://www.star-history.com/?repos=chenjunfu2%2FLitematic_V7_To_V6&type=date&legend=top-left)

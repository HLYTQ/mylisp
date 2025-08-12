# Lisp

## 依赖

1. GCC编译器，至少需要支持C++17，MSVC需要在visual studio installer下载跨平台套件，并且删除Token_str数组和引用到它的debug_token函数

2. CMake，一个C++构建工具

## 编译

项目工程路径中不能含有中文名称

根目录下执行：

```bash
mkdir build
cd build
cmake ..
make
```
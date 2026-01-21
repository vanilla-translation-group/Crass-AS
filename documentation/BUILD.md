## 依赖项

- Visual Studio
- CMake > 3.10
- Strawberry Perl

## 环境准备

以下步骤均假定在命令行中操作。

### ZLib & OpenSSL

首先准备好 Perl 环境，如果使用的是 Strawberry Perl 的 portable 包，需要先设置好环境变量 `SET PATH=%PATH%;path\to\strawberryperl\perl\bin;path\to\strawberryperl\c\bin`（这也同时包括了 nasm 汇编器）

接着按[这个 repo](https://github.com/kiyolee/openssl1_0-win-build) 里的步骤操作，先编译 zlib，再编译 openssl。

cd 到 build-VS2022-MT 文件夹中，执行 `msbuild openssl1_0.sln /p:Configuration=Release` 即可。注意，如果因为 perl 环境未安装而编译失败，在第二次编译前需 rm Release/\_work/\*.def，否则 libeay 会缺失导出符号并导致 ssleay 编译失败。

编译完成后把 libz.dll 和 libeay.dll 放到 build 文件夹下，libz.lib 和 libeay.lib 放到 common/lib 文件夹下。

### BZip2

基本同上。

### GLS3

EntisGLS 插件的特殊依赖。

由于协议原因，无法直接分发包括头文件在内的库本身，请使用 `common/GLS3/setup.sh` 来下载相关文件。

需将 `Cotopha/Library/win32/vs2019/gls.lib` 拷贝到 common/lib 下。

## 编译

```
> cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCRASS_VERSION=0.5.0.0 .
> nmake
```

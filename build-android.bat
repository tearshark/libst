@echo off

REM 因为使用了C++14的原因，NDK里的GCC版本被锁定到了4.9，导致不支持C++14. 只好切换成clang。
REM 请修改‘NDK_TOOLCHAIN_VERSION=clang3.6’是使用本地NDK里存在的clang版本
"%ANDROID_NDK_ROOT%\ndk-build" NDK_TOOLCHAIN_VERSION=clang3.6 -B NDK_DEBUG=0 NDK_PROJECT_PATH=%~dp0

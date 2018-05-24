@echo off
cls

REM 因为使用了C++14的原因，NDK里的GCC版本被锁定到了4.9，导致不支持C++14. 只好切换成clang。
REM 请修改‘NDK_TOOLCHAIN_VERSION=clang3.6’是使用本地NDK里存在的clang版本

"%ANDROID_NDK_ROOT%\ndk-build" NDK_TOOLCHAIN_VERSION=clang3.6 -B NDK_DEBUG=0 NDK_PROJECT_PATH=%~dp0

REM mkdir .\libs\Android\armeabi
REM xcopy .\obj\local\armeabi\libvfxbase.a .\libs\Android\armeabi\ /Y

REM mkdir .\libs\Android\armeabi-v7a
REM xcopy .\obj\local\armeabi-v7a\libvfxbase.a .\libs\Android\armeabi-v7a\ /Y

REM mkdir .\libs\Android\x86
REM xcopy .\obj\local\x86\libvfxbase.a .\libs\Android\x86\ /Y

REM mkdir .\libs\Android\mips
REM xcopy .\obj\local\mips\libvfxbase.a .\libs\Android\mips\ /Y

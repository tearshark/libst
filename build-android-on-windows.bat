@echo off

REM call %ROOT_DIR%\build-android.bat

SET ROOT_DIR=%~dp0

mkdir %ROOT_DIR%\libs\Android\armeabi
xcopy %ROOT_DIR%\obj\local\armeabi\*.a  %ROOT_DIR%\libs\Android\armeabi\ /Y /I /Q

mkdir %ROOT_DIR%\libs\Android\armeabi-v7a
xcopy %ROOT_DIR%\obj\local\armeabi-v7a\*.a  %ROOT_DIR%\libs\Android\armeabi-v7a\ /Y /I /Q

mkdir %ROOT_DIR%\libs\Android\arm64-v8a
xcopy %ROOT_DIR%\obj\local\arm64-v8a\*.a  %ROOT_DIR%\libs\Android\arm64-v8a\ /Y /I /Q

mkdir %ROOT_DIR%\libs\Android\x86
xcopy %ROOT_DIR%\obj\local\x86\*.a  %ROOT_DIR%\libs\Android\x86\ /Y /I /Q

mkdir %ROOT_DIR%\libs\Android\x86_64
xcopy %ROOT_DIR%\obj\local\x86_64\*.a  %ROOT_DIR%\libs\Android\x86_64\ /Y /I /Q

mkdir %ROOT_DIR%\libs\Android\mips
xcopy %ROOT_DIR%\obj\local\mips\*.a  %ROOT_DIR%\libs\Android\mips\ /Y /I /Q

mkdir %ROOT_DIR%\libs\Android\mips64
xcopy %ROOT_DIR%\obj\local\mips64\*.a  %ROOT_DIR%\libs\Android\mips64\ /Y /I /Q

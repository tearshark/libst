APP_ABI := all

#使用默认最小的C++运行库，这样生成的应用体积小，内存占用小，但部分功能将无法支持
#APP_STL := system

#使用 GNU libstdc++ 作为静态库,可以使用异常
#APP_STL := gnustl_static

#使用STLport作为静态库，这项是Android开发网极力推荐的,不支持异常和RTTI
#APP_STL := stlport_static

#STLport 作为动态库，这个可能产生兼容性和部分低版本的Android固件，目前不推荐使用
#APP_STL := stlport_shared

#APP_STL := c++_shared
APP_STL := c++_static

APP_CPPFLAGS := -D__GXX_EXPERIMENTAL_CXX0X__ -std=c++1y -frtti -fvisibility=hidden -fvisibility-inlines-hidden -fexceptions -Wno-error=format-security -fsigned-char -Os $(CPPFLAGS)

PP_DEBUG := $(strip $(NDK_DEBUG))
ifeq ($(APP_DEBUG),1)
  APP_OPTIM := debug
else
  APP_OPTIM := release
endif

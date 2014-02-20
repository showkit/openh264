include build/platform-x86-common.mk
ASM = nasm
SHAREDLIBSUFFIX = dll
CFLAGS += -DMT_ENABLED -MMD -MP
LDFLAGS +=
ifeq ($(ENABLE64BIT), Yes)
ASMFLAGS += -f win64
ASMFLAGS_PLATFORM = -DWIN64
CFLAGS += -DWIN64
CC = x86_64-w64-mingw32-gcc
CXX = x86_64-w64-mingw32-g++
AR = x86_64-w64-mingw32-ar
else
ASMFLAGS += -f win32 -DPREFIX
CFLAGS += -DWIN32
endif
EXEEXT = .exe


ARCH = arm-none-eabi
CC = ${ARCH}-gcc
CPP = ${ARCH}-g++
AS = ${ARCH}-as
LD = ${ARCH}-ld
AR = ${ARCH}-ar
OBJCOPY = ${ARCH}-objcopy

TOOLS_VERSION = $(shell $(CC) -dumpversion)

ifndef PLATFORM
PLATFORM = qemu
endif

ifdef RELEASE
CFLAGS = -O3 -std=gnu99 -Werror -DPLATFORM=$(PLATFORM) -DRASPBERRY_PI
ASFLAGS =
else
CFLAGS = -O0 -g -std=gnu99 -Werror -DPLATFORM=$(PLATFORM) -DRASPBERRY_PI
ASFLAGS = -g
endif

CFLAGS_FOR_TARGET = -mcpu=arm1176jzf-s
ASFLAGS_FOR_TARGET = -mcpu=arm1176jzf-s
LDFLAGS = -nostdlib -static --error-unresolved-symbols

SYSLIBS = /usr/local/lib/gcc/$(ARCH)/$(TOOLS_VERSION)/libgcc.a
NEWLIB_DIR = newlib/usr/$(ARCH)
NEWLIB_LIBC = $(NEWLIB_DIR)/lib/libc.a 
NEWLIB_LIBM = $(NEWLIB_DIR)/lib/libm.a

MODULES := bsp bsp/generic bsp/$(PLATFORM) lib lambda
SRC_DIR := $(addprefix src/,$(MODULES))
INC_DIR := $(addsuffix /include,$(SRC_DIR))

ASRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
AOBJ     := $(ASRC:.s=.o)
CSRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
COBJ     := $(CSRC:.c=.o)

INCLUDES := -Isrc $(addprefix -I,$(SRC_DIR) $(INC_DIR)) -I$(NEWLIB_DIR)/include

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.s $(SRC_DIR)

%.o: %.c
	$(CC) $(CFLAGS_FOR_TARGET) $(INCLUDES) $(CFLAGS) -c -o $*.o $<

%.o: %.s
	$(AS) $(ASFLAGS_FOR_TARGET) $(INCLUDES) $(ASFLAGS) -o $*.o $<

OBJ = $(AOBJ) $(COBJ) $(CPPOBJ)

bin/kernel.img: bin/kernel.elf
	${OBJCOPY} -O binary $< $@

bin/kernel.elf: lambdapi.ld $(OBJ) $(SYSLIBS)
	${LD} ${LDFLAGS} -T lambdapi.ld $(OBJ) $(SYSLIBS) $(NEWLIB_LIBC) $(NEWLIB_LIBM) -o $@

install-newlib:
	newlib/build-newlib.sh

clean:
	rm -f bin/*.elf bin/*.img $(OBJ)

help:
	@echo
	@echo make install-newlib
	@echo make clean
	@echo make [RELEASE=1] [PLATFORM='(raspi|qemu)']
	@echo

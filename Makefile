ARCH = arm-none-eabi
CC = ${ARCH}-gcc
CPP = ${ARCH}-g++
AS = ${ARCH}-as
LD = ${ARCH}-ld
AR = ${ARCH}-ar
OBJCOPY = ${ARCH}-objcopy
PLATFORM = qemu
#PLATFORM = raspi

GCCVERSION = `$(CC) -dumpversion`

FLAGS = -O0 -g -Werror -DPLATFORM=$(PLATFORM)
CFLAGS = $(FLAGS) -std=c99
CPPFLAGS = $(FLAGS) 
CFLAGS_FOR_TARGET = -mcpu=arm1176jzf-s
ASFLAGS = -g
ASFLAGS_FOR_TARGET = -mcpu=arm1176jzf-s
LDFLAGS = -nostdlib -static --error-unresolved-symbols

SYSLIBS = /usr/local/lib/gcc/$(ARCH)/$(GCCVERSION)/libgcc.a

MODULES := bsp bsp/generic bsp/$(PLATFORM) lib lambda
SRC_DIR := $(addprefix src/,$(MODULES))
OBJ_DIR := obj

ASRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
AOBJ     := $(ASRC:.s=.o)
CSRC     := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
COBJ     := $(CSRC:.c=.o)
CPPSRC   := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
CPPOBJ   := $(CPPSRC:.cpp=.o)

INCLUDES  := -Isrc $(addprefix -I,$(SRC_DIR))

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.s $(SRC_DIR)

%.o: %.c
	$(CC) $(CFLAGS_FOR_TARGET) $(INCLUDES) $(CFLAGS) -c -o $*.o $<

%.o: %.cpp
	$(CPP) $(CFLAGS_FOR_TARGET) $(INCLUDES) $(CPPFLAGS) -c -o $*.o $<

%.o: %.s
	$(AS) $(ASFLAGS_FOR_TARGET) $(INCLUDES) $(ASFLAGS) -o $*.o $<

OBJ = $(AOBJ) $(COBJ) $(CPPOBJ)

bin/kernel.img: bin/kernel.elf
	${OBJCOPY} -O binary $< $@

bin/kernel.elf: lambdapi.ld $(OBJ)
	${LD} ${LDFLAGS} -T lambdapi.ld $(OBJ) ${SYSLIBS} -o $@

clean:
	rm -f bin/*.elf bin/*.img $(OBJ)


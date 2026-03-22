ifndef TARGET_COMPILE
    TARGET_COMPILE := 
endif

ifndef KP_DIR
    KP_DIR = ../..
endif

# Enforce Clang/LLD since OnePlus 9 Pro kernel is compiled with Clang 11.0.2
CC = $(TARGET_COMPILE)clang
LD = $(TARGET_COMPILE)ld.lld

CFLAGS += -O2 -fno-builtin -nostdlib -mgeneral-regs-only
LDFLAGS += -r

INCLUDE_DIRS := . include patch/include linux/include linux/arch/arm64/include linux/tools/arch/arm64/include

INCLUDE_FLAGS := $(foreach dir,$(INCLUDE_DIRS),-I$(KP_DIR)/kernel/$(dir))

objs := performance.o

all: performance.kpm

performance.kpm: ${objs}
	${LD} ${LDFLAGS} -o $@ $^

%.o: %.c
	${CC} $(CFLAGS) $(INCLUDE_FLAGS) -c -O2 -o $@ $<

.PHONY: clean
clean:
	rm -rf *.kpm
	find . -name "*.o" | xargs rm -f

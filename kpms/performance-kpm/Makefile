ifndef TARGET_COMPILE
    TARGET_COMPILE := aarch64-linux-gnu-
endif

ifndef KP_DIR
    KP_DIR = ../..
endif

# Fallback to GCC since Clang/LLD may generate unsupported ARM64 relocations for KPM loader
CC = $(TARGET_COMPILE)gcc
LD = $(TARGET_COMPILE)ld

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

ifndef TARGET_COMPILE
    TARGET_COMPILE := aarch64-linux-gnu-
endif

ifndef KP_DIR
    KP_DIR = ../..
endif

TARGET = performance

# If CC/LD are passed from the command line, they will override these defaults.
CC ?= $(TARGET_COMPILE)gcc
LD ?= $(TARGET_COMPILE)ld

# Use Clang-compatible flags if CC contains clang, else GCC defaults
CFLAGS += -O2
ifneq (,$(findstring clang,$(CC)))
    # Clang specific flags for kernel modules (avoids floating point and unsupported relocs)
    CFLAGS += -mgeneral-regs-only -nostdlib -fno-builtin -fpic
else
    CFLAGS += -mgeneral-regs-only -nostdlib -fno-builtin
endif

INCLUDE_DIRS := . include patch/include linux/include linux/arch/arm64/include linux/tools/arch/arm64/include
INCLUDE_FLAGS := $(foreach dir,$(INCLUDE_DIRS),-I$(KP_DIR)/kernel/$(dir))

objs := $(TARGET).o

all: $(TARGET).kpm

$(TARGET).kpm: ${objs}
	# Link the KPM using relocatable output (-r)
	${LD} -r -o $@ $^

%.o: %.c
	${CC} $(CFLAGS) $(INCLUDE_FLAGS) -T$(TARGET).lds -c -o $@ $<

.PHONY: clean
clean:
	rm -rf *.kpm
	find . -name "*.o" | xargs rm -f
ifndef TARGET_COMPILE
    TARGET_COMPILE := aarch64-linux-gnu-
endif

ifndef KP_DIR
    KP_DIR = ../..
endif

TARGET = performance

CC = $(TARGET_COMPILE)gcc
LD = $(TARGET_COMPILE)ld

INCLUDE_DIRS := . include patch/include linux/include linux/arch/arm64/include linux/tools/arch/arm64/include
INCLUDE_FLAGS := $(foreach dir,$(INCLUDE_DIRS),-I$(KP_DIR)/kernel/$(dir))

objs := $(TARGET).o

all: $(TARGET).kpm

$(TARGET).kpm: ${objs}
	${CC} -r -o $@ $^
	# APatch KernelPatch check_kpm() strictly requires ELFOSABI_NONE (0x00)
	# Ubuntu's aarch64-linux-gnu-gcc writes ELFOSABI_GNU (0x03), causing load failed: -8 (ENOEXEC)
	# This single command byte-patches offset 7 of the ELF header to 0x00, unconditionally fixing the error.
	printf '\x00' | dd of=$@ bs=1 seek=7 conv=notrunc

%.o: %.c
	${CC} $(CFLAGS) $(INCLUDE_FLAGS) -T$(TARGET).lds -c -O2 -o $@ $<

.PHONY: clean
clean:
	rm -rf *.kpm
	find . -name "*.o" | xargs rm -f
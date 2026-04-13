INCLUDES_DIR = src/kernel/include

CXX = i386-elf-g++
AS  = nasm
LD  = i386-elf-g++
OBJCOPY = i386-elf-objcopy

SRC_DIR = src/kernel
INC_DIR = src/include
BUILD_DIR = build

CXXFLAGS = -ffreestanding -m32 -g -Wall -Wextra \
           -fno-exceptions -fno-rtti -fno-use-cxa-atexit \
           -I$(INC_DIR)

ASFLAGS = -f elf
LDFLAGS = -ffreestanding -m32 -g -nostdlib -nostartfiles -Ttext 0x10000

OBJS = $(BUILD_DIR)/kernel_entry.o \
       $(BUILD_DIR)/kernel.o \
       $(BUILD_DIR)/printf.o \
       $(BUILD_DIR)/gdtc.o \
       $(BUILD_DIR)/gdts.o \
       $(BUILD_DIR)/utils.o \
	   $(BUILD_DIR)/idtc.o \
	   $(BUILD_DIR)/idts.o

all: $(BUILD_DIR)/SolsticeOS.iso

$(BUILD_DIR)/gdtc.o: $(SRC_DIR)/gdt/gdt.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/idtc.o: $(SRC_DIR)/idt/idt.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils/utils.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# kernel ovveride
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/printf.o: $(SRC_DIR)/misc/printf.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# compile assembly files
$(BUILD_DIR)/gdts.o: $(SRC_DIR)/gdt/gdt.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/idts.o: $(SRC_DIR)/idt/idt.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel_entry.o: $(SRC_DIR)/kernel_entry.asm
	$(AS) $(ASFLAGS) $< -o $@

# link kernel
$(BUILD_DIR)/complete_kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(shell $(CXX) -print-libgcc-file-name)

# create iso image
$(BUILD_DIR)/SolsticeOS.iso: $(BUILD_DIR)/complete_kernel.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/complete_kernel.elf $(BUILD_DIR)/kernel.bin
	$(AS) -f bin src/bootloader/boot.asm -o $(BUILD_DIR)/boot.bin
	$(AS) -f bin src/other/zeroes.asm -o $(BUILD_DIR)/zeroes.bin
	# concatenate all to create final binary
	cat $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/zeroes.bin > $(BUILD_DIR)/SolsticeOS.bin
	# make iso
	mkisofs -o $@ $(BUILD_DIR)/SolsticeOS.bin

clean:
	rm -rf $(BUILD_DIR)
	mkdir -p build

.PHONY: all clean
CC = g++
LD = g++
OBJCOPY = objcopy
NASM = nasm
EFI_CC = x86_64-w64-mingw32-g++ # windows-target cross compiler for uefi

CFLAGS = -ffreestanding -m64 -g -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-pic -fno-pie -fno-stack-protector -Isrc/include
LDFLAGS = -ffreestanding -m64 -g -nostdlib -nostartfiles -Ttext 0x100000 -no-pie

# asset source and object paths
FONT_SRC = src/include/FreeSans.sfn
FONT_RAW = build/FreeSans.sfn
FONT_OBJ = build/font.o

# primary compilation targets and modules list
OBJS = build/kernel_entry.o build/kernel.o build/printf.o build/console.o build/gdtc.o build/gdts.o build/utils.o build/idtc.o build/idts.o $(FONT_OBJ)

all: prepare build/kernel.bin build/BOOTX64.EFI # trigger build chain

# ensure build directory exists before any compilation happens
prepare:
	mkdir -p build

build/kernel_entry.o: src/kernel/kernel_entry.asm
	$(NASM) -f elf64 $< -o $@

build/kernel.o: src/kernel/kernel.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/printf.o: src/kernel/misc/printf.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/console.o: src/kernel/misc/console.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/gdtc.o: src/kernel/gdt/gdt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/gdts.o: src/kernel/gdt/gdt.s
	$(NASM) -f elf64 $< -o $@

build/utils.o: src/kernel/utils/utils.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/idtc.o: src/kernel/idt/idt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/idts.o: src/kernel/idt/idt.s
	$(NASM) -f elf64 $< -o $@

$(FONT_OBJ): $(FONT_SRC)
	gzip -dc $< > $(FONT_RAW)
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $(FONT_RAW) $@

build/complete_kernel.elf: $(OBJS)
	g++ -T src/kernel/linker.ld -ffreestanding -m64 -nostdlib -no-pie -o $@ $^

build/kernel.bin: build/complete_kernel.elf
	$(OBJCOPY) -O binary $< $@

build/BOOTX64.EFI: src/bootloader/bootloader.cpp
	$(EFI_CC) -ffreestanding -m64 -fshort-wchar -mno-red-zone \
	-fno-exceptions -fno-rtti -Wall -Wextra -O2 -nostdlib \
	-Wl,-subsystem,10 \
	-e efi_main \
	-o $@ $<

# finalize deployment folder tree for qemu simulation
setup_iso: all
	mkdir -p build/iso/EFI/BOOT
	cp build/BOOTX64.EFI build/iso/EFI/BOOT/
	cp build/kernel.bin build/iso/

# simulate system boot sequence
run: setup_iso
	qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd -drive file=fat:rw:build/iso,format=raw,media=disk -serial stdio

debug: setup_iso
	qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd -drive file=fat:rw:build/iso,format=raw,media=disk -serial stdio -s -S -debugcon file:debug.log -global isa-debugcon.iobase=0x402

clean: # purge build artifacts
	rm -rf build
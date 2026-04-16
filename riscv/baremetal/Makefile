ARCH		= rv32i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zmmul1p0_zaamo1p0_zalrsc1p0 # rv32imafd
ABI		= ilp32d
CC		= riscv32-unknown-elf-gcc -mabi=$(ABI) -march=$(ARCH)
AS		= riscv32-unknown-elf-as -march=$(ARCH)
CCLD		= riscv32-unknown-elf-gcc -march=$(ARCH) -mabi=$(ABI)
LD		= riscv32-unknown-elf-ld -b elf32-littleriscv -m elf32lriscv
OBJDUMP		= riscv32-unknown-elf-objcopy -I elf32-littleriscv 

BIN		= hello.bin loop.bin


ALL	: $(BIN) pointer sizes

pointer	: pointer.c

hello.bin: hello.elf
	$(OBJDUMP) -S -O binary $< $@

hello.elf: hello.o | test.elf.ld
	$(LD) -T test.elf.ld -o $@ $^

hello.o: hello.s
	$(AS) -o $@ $<

entry.o: entry.s
	$(AS) -o $@ $<

printf.o: printf.c printf.h
	$(CC) -o $@ -c $<

loop.bin: loop.elf
	$(OBJDUMP) -S -O binary $< $@

loop.elf: entry.o printf.o loop.o
	$(CCLD) -Wl,-Ttest.elf.ld -lm -lgcc -static  -nostartfiles -ffreestanding -o $@ $^

loop.o: loop.s
	$(AS) -o $@ $<

loop.s: loop.c
	$(CC) -o $@ -S $<


clean:
	rm $(BIN) *.o *~

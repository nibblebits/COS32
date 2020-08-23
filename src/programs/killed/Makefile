all:
	nasm -f elf ./killed.asm -o ./killed.o
	i686-elf-gcc -g -T ./linker.ld -o ./killed.elf -ffreestanding -O0 -nostdlib -fpic  -g ./killed.o

clean:
	rm -rf ./killed.elf
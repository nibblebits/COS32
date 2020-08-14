all:
	nasm -f elf ./helloworld.asm -o ./helloworld.o
	i686-elf-gcc -g -T ./linker.ld -o ./helloworld.elf -ffreestanding -O0 -nostdlib -fpic  -g ./helloworld.o
	nasm -f bin ./helloworld2.asm -o ./helloworld2.bin

clean:
	rm -rf ./helloworld.bin
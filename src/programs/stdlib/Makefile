OBJECTS=./build/start.asm.o ./build/cos32.asm.o
FLAGS = --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	
./build/start.asm.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.asm.o

./build/cos32.asm.o: ./src/cos32.asm
	nasm -f elf ./src/cos32.asm -o ./build/cos32.asm.o

all: ${OBJECTS}
	i686-elf-ld -m elf_i386 -relocatable ${OBJECTS} -o ./cos32.stdlib.o

clean:
	rm -rf ./cos32.stdlib.o
	rm -rf ${OBJECTS}
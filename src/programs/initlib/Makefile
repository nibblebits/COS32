OBJECTS=./build/start.asm.o ./build/start.o
FLAGS = --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
INCLUDES = -I ./ -I ../stdlib/src
./build/start.asm.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.asm.o

./build/start.o: ./src/start.c
	i686-elf-gcc $(INCLUDES) ${FLAGS} -c ./src/start.c -o ./build/start.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


all: ${OBJECTS}
	i686-elf-ld -m elf_i386 -relocatable ${OBJECTS} -o ./initlib.o 

clean:
	rm -rf ./initlib.o
	rm -rf ${OBJECTS}
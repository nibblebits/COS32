OBJECTS=./build/testprogram.o
FLAGS =  --verbose --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	

./build/testprogram.o: ./src/testprogram.asm
	nasm -f elf ./src/testprogram.asm -o ./build/testprogram.o


all: ${OBJECTS}
	i686-elf-ld  -nostdlib -g -T ./linker.ld -o ./testlibprogram.elf -O0  -g -L../testlib  ${OBJECTS} -l:testlib.so

clean:
	rm -rf ./testlibprogram.elf
	rm -rf ${OBJECTS}
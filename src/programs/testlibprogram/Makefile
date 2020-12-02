OBJECTS=./build/testprogram.o
FLAGS =  --verbose --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	

./build/testprogram.o: ./src/testprogram.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -L../testlib  -l:testlib.so -c ./src/testprogram.c -o ./build/testprogram.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


all: ${OBJECTS}
	i686-elf-ld  -g -T ./linker.ld -o ./testlibprogram.elf -O0 -nostdlib -g -L../testlib  ${OBJECTS} -l:testlib.so

clean:
	rm -rf ./testlibprogram.elf
	rm -rf ${OBJECTS}
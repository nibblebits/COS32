OBJECTS=./build/crash.o
INCLUDES = -I./ -I../stdlib/src
FLAGS =  --verbose --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	
./build/crash.o: ./src/crash.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/crash.c -o ./build/crash.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

all: ${OBJECTS}
	i686-elf-ld -g -T ./linker.ld -o ./crash.elf -O0 -nostdlib -g ../stdlib/cos32.stdlib.o ${OBJECTS}
	
clean:
	rm -rf ${OBJECTS}
	rm -rf ./crash.elf
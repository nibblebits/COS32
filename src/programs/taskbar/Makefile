OBJECTS=./build/taskbar.o
INCLUDES = -I./ -I../stdlib/src
FLAGS =  --verbose --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	
./build/taskbar.o: ./src/taskbar.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/taskbar.c -o ./build/taskbar.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

all: ${OBJECTS}
	i686-elf-ld -g -T ./linker.ld -o ./taskbar.elf -O0 -nostdlib -g ${OBJECTS} ../stdlib/cos32.stdlib.o
	
clean:
	rm -rf ${OBJECTS}
	rm -rf ./taskbar.elf
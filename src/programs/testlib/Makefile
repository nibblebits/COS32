OBJECTS=./build/testlib.o
FLAGS = -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	

./build/testlib.o: ./src/testlib.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/testlib.c -o ./build/testlib.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

all: ${OBJECTS}
	i686-elf-ld -g -pie --dynamic-linker -Wl,-shared  --export-dynamic -o ./testlib.so -O0 -nostdlib -g ${OBJECTS}

clean:
	rm -rf ./testlib.so
	rm -rf ${OBJECTS}
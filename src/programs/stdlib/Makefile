OBJECTS=./build/start.asm.o ./build/cos32.asm.o ./build/cos32.o ./build/stdio.o ./build/stdlib.o
FLAGS = --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
	
./build/start.asm.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.asm.o

./build/cos32.asm.o: ./src/cos32.asm
	nasm -f elf ./src/cos32.asm -o ./build/cos32.asm.o


./build/stdio.o: ./src/stdio.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/stdio.c -o ./build/stdio.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/stdlib.o: ./src/stdlib.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/stdlib.c -o ./build/stdlib.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/cos32.o: ./src/cos32.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/cos32.c -o ./build/cos32.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


all: ${OBJECTS}
	i686-elf-ld -m elf_i386 -relocatable ${OBJECTS} -o ./cos32.stdlib.o

clean:
	rm -rf ./cos32.stdlib.o
	rm -rf ${OBJECTS}
OBJECTS=./build/cos32.asm.o ./build/cos32.o ./build/stdio.o ./build/string.o ./build/stdlib.o
FLAGS = --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

./build/cos32.asm.o: ./src/cos32.asm
	nasm -f elf ./src/cos32.asm -o ./build/cos32.asm.o

./build/stdio.o: ./src/stdio.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/stdio.c -o ./build/stdio.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/stdlib.o: ./src/stdlib.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/stdlib.c -o ./build/stdlib.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/string.o: ./src/string.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/string.c -o ./build/string.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/cos32.o: ./src/cos32.c
	i686-elf-gcc $(INCLUDES)  ${FLAGS} -c ./src/cos32.c -o ./build/cos32.o -fno-common -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g



all: ${OBJECTS}
	i686-elf-ld -g -pie --dynamic-linker -Wl,-shared  --export-dynamic -o ./stdlib.so -O0 -nostdlib -g ${OBJECTS}

clean:
	rm -rf ./stdlib.so 
	rm -rf ${OBJECTS}
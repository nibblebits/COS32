FILES = ./build/kernel.asm.o ./build/kernel.o ./build/task/tss.asm.o ./build/gdt/gdt.asm.o ./build/gdt/gdt.o ./build/task/task.asm.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o ./build/memory/idt/idt.asm.o ./build/memory/idt/idt.o ./build/io/io.o  ./build/disk/disk.o ./build/fs/file.o ./build/fs/fat/fat16.o ./build/memory/heap.o ./build/memory/kheap.o ./build/memory/memory.o ./build/string/string.o
FLAGS = -g
INCLUDES = -I./src
all: ./bin/kernel.bin ./bin/boot.bin ${FILES}
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	sudo mount -t vfat ./bin/os.bin /mnt/d
	sudo cp ./src/status.h /mnt/d
	sudo cp ./src/types.h /mnt/d
	sudo cp ./img.jpg /mnt/d
	sudo umount /mnt/d
	sudo chmod 777 ./bin/os.bin

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./bin/kernel.bin: ${FILES}
	i686-elf-ld  -g -relocatable ${FILES} -o ./build/kernelfull.o
	i686-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O2 -nostdlib -g ./build/kernelfull.o

./build/gdt/gdt.o: ./src/gdt/gdt.c ./src/gdt/gdt.h
	i686-elf-gcc $(INCLUDES) -I./src/gdt ${FLAGS} -c ./src/gdt/gdt.c -o ./build/gdt/gdt.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm ./src/gdt/gdt.h
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/gdt/gdt.asm.o

./build/task/tss.asm.o: ./src/task/tss.asm ./src/task/task.h
	nasm -f elf -g ./src/task/tss.asm -o ./build/task/tss.asm.o

./build/task/task.asm.o: ./src/task/task.asm ./src/task/task.h
	nasm -f elf -g ./src/task/task.asm -o ./build/task/task.asm.o

./build/memory/idt/idt.asm.o: ./src/memory/idt/idt.asm ./src/memory/idt/idt.h
	nasm -f elf -g ./src/memory/idt/idt.asm -o ./build/memory/idt/idt.asm.o

./build/memory/idt/idt.o: ./src/memory/idt/idt.c ./src/memory/idt/idt.h
	i686-elf-gcc $(INCLUDES) -I./src/memory/idt ${FLAGS} -c ./src/memory/idt/idt.c -o ./build/memory/idt/idt.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm ./src/memory/paging/paging.h
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c ./src/memory/paging/paging.h
	i686-elf-gcc $(INCLUDES) -I./src/memory/paging  ${FLAGS} -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/io/io.o: ./src/io/io.c ./src/io/io.h
	i686-elf-gcc $(INCLUDES) -I./src/io ${FLAGS} -c ./src/io/io.c -o ./build/io/io.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/disk/disk.o: ./src/disk/disk.c ./src/disk/disk.h
	i686-elf-gcc $(INCLUDES) -I./src/disk ${FLAGS} -c ./src/disk/disk.c -o ./build/disk/disk.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/memory/heap.o: ./src/memory/heap.c ./src/memory/heap.h
	i686-elf-gcc $(INCLUDES) -I./src/memory ${FLAGS} -c ./src/memory/heap.c -o ./build/memory/heap.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/memory/kheap.o: ./src/memory/kheap.c ./src/memory/kheap.h
	i686-elf-gcc $(INCLUDES) -I./src/memory ${FLAGS} -c ./src/memory/kheap.c -o ./build/memory/kheap.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/memory/memory.o: ./src/memory/memory.c ./src/memory/memory.h
	i686-elf-gcc $(INCLUDES) -I./src/memory ${FLAGS} -c ./src/memory/memory.c -o ./build/memory/memory.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/string/string.o: ./src/string/string.c ./src/string/string.h
	i686-elf-gcc $(INCLUDES) -I./src/string ${FLAGS} -c ./src/string/string.c -o ./build/string/string.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/fs/file.o: ./src/fs/file.c ./src/fs/file.h
	i686-elf-gcc $(INCLUDES) -I./src/fs ${FLAGS} -c ./src/fs/file.c -o ./build/fs/file.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c ./src/fs/fat/fat16.h
	i686-elf-gcc $(INCLUDES) -I./src/fs/fat ${FLAGS} -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/kernel.o: ./src/kernel.c ./src/kernel.h
	i686-elf-gcc $(INCLUDES) ${FLAGS} -c ./src/kernel.c -o ./build/kernel.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

clean:
	rm -rf ${FILES}
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
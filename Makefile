

FILES = ./build/kernel.asm.o  ./build/keyboard/listener.o ./build/keyboard/listeners/fkeylistener.o ./build/keyboard/listeners/scrollkeylistener.o ./build/keyboard/keyboard.o ./build/keyboard/classic.o  ./build/timer/pit.o ./build/task/task.o ./build/task/process.o ./build/kernel.o ./build/gdt/gdt.o  ./build/memory/array.o ./build/memory/paging/paging.o ./build/idt/idt.o ./build/io/io.o  ./build/disk/disk.o ./build/fs/pparser.o ./build/fs/file.o ./build/fs/fat/fat16.o ./build/video/video.o  ./build/video/font/font.o ./build/video/font/formats/psffont.o ./build/video/rectangle.o ./build/memory/memory.o ./build/string/string.o ./build/loader/library.o ./build/loader/formats/elf/elf.o ./build/loader/formats/elf/elfloader.o  ./build/memory/heap.o ./build/memory/kheap.o ./build/disk/streamer.o ./build/isr80h/isr80h.o ./build/isr80h/io.o ./build/isr80h/process.o ./build/isr80h/isrkernel.o ./build/isr80h/video.o ./build/isr80h/font.o ./build/memory/registers.asm.o ./build/task/tss.asm.o ./build/gdt/gdt.asm.o ./build/task/task.asm.o ./build/memory/paging/paging.asm.o ./build/idt/idt.asm.o
FLAGS =  --freestanding -falign-jumps -falign-functions -falign-labels -falign-loops  -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
BUILD_NUMBER_FILE=build-number.txt

INCLUDES = -I./src
all: ./bin/kernel.bin ./bin/boot.bin ${FILES} programs 
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	sudo mount -t vfat ./bin/os.bin /mnt/d
	sudo cp ./src/programs/helloworld/helloworld.elf /mnt/d/hello.e
	sudo cp ./src/programs/helloworld/helloworld2.bin /mnt/d/start.b
	sudo cp ./src/programs/taskbar/taskbar.elf /mnt/d/taskbar.e
	sudo mkdir /mnt/d/bin	
	sudo cp ./src/programs/crash/crash.elf /mnt/d/bin/crash.e
	sudo cp ./src/programs/shell/shell.elf /mnt/d/bin/shell.e
	sudo cp ./src/programs/testlibprogram/testlibprogram.elf /mnt/d/test.e
	sudo cp ./src/programs/testlib/testlib.so /mnt/d/testlib.so
	sudo cp ./src/programs/stdlib/stdlib.so /mnt/d/stdlib.so

	sudo mkdir /mnt/d/fonts
	sudo cp ./fonts/plfont.psf /mnt/d/fonts/plfont.psf

	sudo umount /mnt/d
	sudo chmod 777 ./bin/os.bin
	# Update build number
	echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./bin/kernel.bin: ${FILES}  $(BUILD_NUMBER_FILE) 
	i686-elf-ld  -m elf_i386 -relocatable $(BUILD_NUMBER_LDFLAGS) ${FILES} -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS)  -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib -fpic  -g ./build/kernelfull.o

./build/gdt/gdt.o: ./src/gdt/gdt.c ./src/gdt/gdt.h
	i686-elf-gcc $(INCLUDES) -I./src/gdt ${FLAGS} -c ./src/gdt/gdt.c -o ./build/gdt/gdt.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm ./src/gdt/gdt.h
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/gdt/gdt.asm.o

./build/task/tss.asm.o: ./src/task/tss.asm ./src/task/task.h
	nasm -f elf -g ./src/task/tss.asm -o ./build/task/tss.asm.o


./build/video/video.o: ./src/video/video.c ./src/video/video.h 
	i686-elf-gcc  $(INCLUDES) -I./src/video ${FLAGS} -c ./src/video/video.c -o ./build/video/video.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/video/font/font.o: ./src/video/font/font.c ./src/video/font/font.h 
	i686-elf-gcc  $(INCLUDES) -I./src/video -I./src/video/font ${FLAGS} -c ./src/video/font/font.c -o ./build/video/font/font.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/video/font/formats/psffont.o: ./src/video/font/formats/psffont.c ./src/video/font/formats/psffont.h 
	i686-elf-gcc  $(INCLUDES) -I./src/video -I./src/video/font -I./src/video/font/formats ${FLAGS} -c ./src/video/font/formats/psffont.c -o ./build/video/font/formats/psffont.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/video/rectangle.o: ./src/video/rectangle.c ./src/video/rectangle.h 
	i686-elf-gcc  $(INCLUDES) -I./src/video ${FLAGS} -c ./src/video/rectangle.c -o ./build/video/rectangle.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/timer/pit.o: ./src/timer/pit.c ./src/timer/pit.h
	i686-elf-gcc  $(INCLUDES) -I./src/timer ${FLAGS} -c ./src/timer/pit.c -o ./build/timer/pit.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/task/task.o: ./src/task/task.c ./src/task/task.h
	i686-elf-gcc $(INCLUDES) -I./src/task ${FLAGS} -c ./src/task/task.c -o ./build/task/task.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/task/task.asm.o: ./src/task/task.asm ./src/task/task.h
	nasm -f elf -g ./src/task/task.asm -o ./build/task/task.asm.o

./build/task/process.o: ./src/task/process.c ./src/task/process.h ./src/task/task.h
	i686-elf-gcc $(INCLUDES) -I./src/task ${FLAGS} -c ./src/task/process.c -o ./build/task/process.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/memory/registers.asm.o: ./src/memory/registers.asm ./src/memory/registers.h
	nasm -f elf -g ./src/memory/registers.asm -o ./build/memory/registers.asm.o

./build/idt/idt.asm.o: ./src/idt/idt.asm ./src/idt/idt.h
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c ./src/idt/idt.h
	i686-elf-gcc $(INCLUDES) -I./src/memory/idt ${FLAGS} -c ./src/idt/idt.c -o ./build/idt/idt.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/keyboard/keyboard.o: ./src/keyboard/keyboard.c ./src/keyboard/keyboard.h
	i686-elf-gcc $(INCLUDES) -I./src/keyboard  ${FLAGS} -c ./src/keyboard/keyboard.c -o ./build/keyboard/keyboard.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/keyboard/listener.o: ./src/keyboard/listener.c ./src/keyboard/listener.h
	i686-elf-gcc $(INCLUDES) -I./src/keyboard  ${FLAGS} -c ./src/keyboard/listener.c -o ./build/keyboard/listener.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/keyboard/listeners/fkeylistener.o: ./src/keyboard/listeners/fkeylistener.c ./src/keyboard/listeners/fkeylistener.h
	i686-elf-gcc $(INCLUDES) -I./src/keyboard/listeners ${FLAGS} -c ./src/keyboard/listeners/fkeylistener.c  -o ./build/keyboard/listeners/fkeylistener.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/keyboard/listeners/scrollkeylistener.o: ./src/keyboard/listeners/scrollkeylistener.c ./src/keyboard/listeners/scrollkeylistener.h
	i686-elf-gcc $(INCLUDES) -I ./src/keyboard -I./src/keyboard/listeners ${FLAGS} -c ./src/keyboard/listeners/scrollkeylistener.c  -o ./build/keyboard/listeners/scrollkeylistener.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/keyboard/classic.o: ./src/keyboard/classic.c ./src/keyboard/classic.h
	i686-elf-gcc $(INCLUDES) -I./src/keyboard  ${FLAGS} -c ./src/keyboard/classic.c -o ./build/keyboard/classic.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm ./src/memory/paging/paging.h
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o


./build/memory/paging/paging.o: ./src/memory/paging/paging.c ./src/memory/paging/paging.h
	i686-elf-gcc $(INCLUDES) -I./src/memory/paging  ${FLAGS} -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/memory/array.o: ./src/memory/array.c ./src/memory/array.h
	i686-elf-gcc $(INCLUDES) -I./src/memory/  ${FLAGS} -c ./src/memory/array.c -o ./build/memory/array.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


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

./build/fs/pparser.o: ./src/fs/pparser.c ./src/fs/pparser.h
	i686-elf-gcc $(INCLUDES) -I./src/fs ${FLAGS} -c ./src/fs/pparser.c -o ./build/fs/pparser.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/fs/file.o: ./src/fs/file.c ./src/fs/file.h
	i686-elf-gcc $(INCLUDES) -I./src/fs ${FLAGS} -c ./src/fs/file.c -o ./build/fs/file.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c ./src/fs/fat/fat16.h
	i686-elf-gcc $(INCLUDES) -I./src/fs/fat ${FLAGS} -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/loader/library.o: ./src/loader/library.c ./src/loader/library.h
	i686-elf-gcc $(INCLUDES) -I./src/loader/library ${FLAGS} -c ./src/loader/library.c -o ./build/loader/library.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g



./build/loader/formats/elf/elf.o: ./src/loader/formats/elf/elf.c ./src/loader/formats/elf/elf.h
	i686-elf-gcc $(INCLUDES) -I./src/loader/formats/elf ${FLAGS} -c ./src/loader/formats/elf/elf.c -o ./build/loader/formats/elf/elf.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/loader/formats/elf/elfloader.o: ./src/loader/formats/elf/elfloader.c ./src/loader/formats/elf/elfloader.h
	i686-elf-gcc $(INCLUDES) -I./src/loader/formats/elf ${FLAGS} -c ./src/loader/formats/elf/elfloader.c -o ./build/loader/formats/elf/elfloader.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/kernel.o: ./src/kernel.c ./src/kernel.h
	i686-elf-gcc $(INCLUDES) ${FLAGS} -c ./src/kernel.c -o ./build/kernel.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/disk/streamer.o: ./src/disk/streamer.c ./src/disk/streamer.h
	i686-elf-gcc  $(INCLUDES) -I./src/disk ${FLAGS} -c ./src/disk/streamer.c -o ./build/disk/streamer.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


./build/isr80h/isr80h.o: ./src/isr80h/isr80h.c ./src/isr80h/isr80h.h
	i686-elf-gcc  $(INCLUDES) -I./src/isr80h ${FLAGS} -c ./src/isr80h/isr80h.c -o ./build/isr80h/isr80h.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/isr80h/io.o: ./src/isr80h/io.c ./src/isr80h/io.h
	i686-elf-gcc  $(INCLUDES) -I./src/isr80h ${FLAGS} -c ./src/isr80h/io.c -o ./build/isr80h/io.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/isr80h/process.o: ./src/isr80h/process.c ./src/isr80h/process.h
	i686-elf-gcc  $(INCLUDES) -I./src/isr80h ${FLAGS} -c ./src/isr80h/process.c -o ./build/isr80h/process.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/isr80h/isrkernel.o: ./src/isr80h/isrkernel.c ./src/isr80h/isrkernel.h
	i686-elf-gcc  $(INCLUDES) -I./src/isr80h ${FLAGS} -c ./src/isr80h/isrkernel.c -o ./build/isr80h/isrkernel.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/isr80h/video.o: ./src/isr80h/video.c ./src/isr80h/video.h
	i686-elf-gcc  $(INCLUDES) -I./src/isr80h ${FLAGS} -c ./src/isr80h/video.c -o ./build/isr80h/video.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g

./build/isr80h/font.o: ./src/isr80h/font.c ./src/isr80h/font.h
	i686-elf-gcc  $(INCLUDES) -I./src/isr80h ${FLAGS} -c ./src/isr80h/font.c -o ./build/isr80h/font.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra -c -g


programs:
	cd ./src/programs/initlib && $(MAKE) all
	cd ./src/programs/stdlib && $(MAKE) all
	cd ./src/programs/helloworld && $(MAKE) all
	cd ./src/programs/killed && $(MAKE) all
	cd ./src/programs/crash && $(MAKE) all
	cd ./src/programs/shell && $(MAKE) all
	cd ./src/programs/taskbar && $(MAKE) all
	cd ./src/programs/testlib && $(MAKE) all
	cd ./src/programs/testlibprogram && $(MAKE) all


programs_clean:
	cd ./src/programs/initlib && $(MAKE) clean
	cd ./src/programs/stdlib && $(MAKE) clean
	cd ./src/programs/helloworld && $(MAKE) clean
	cd ./src/programs/crash && $(MAKE) clean
	cd ./src/programs/killed && $(MAKE) clean
	cd ./src/programs/shell && $(MAKE) clean
	cd ./src/programs/taskbar && $(MAKE) clean
	cd ./src/programs/testlib && $(MAKE) clean
	cd ./src/programs/testlibprogram && $(MAKE) clean


clean: programs_clean
	rm -rf ${FILES}
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./build/kernelfull.o


# Below is used to create an automated build number so that everytime we rebuild a new build number is generated
# Create an auto-incrementing build number.


BUILD_NUMBER_LDFLAGS  = --defsym=__BUILD_DATE=$$(date +'%Y%m%d')
BUILD_NUMBER_LDFLAGS += --defsym=__BUILD_NUMBER=$$(cat $(BUILD_NUMBER_FILE))

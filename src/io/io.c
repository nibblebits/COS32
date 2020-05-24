

unsigned short insw (unsigned short _port){
    unsigned short rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

unsigned char insb (unsigned short _port){
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outw (unsigned short _port, unsigned short _data){
    __asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}


void outb (unsigned short _port, unsigned char _data){
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
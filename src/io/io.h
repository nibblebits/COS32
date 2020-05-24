#ifndef IO_H
#define IO_H

int insb(int port);
int insw(int port);
void outb(int port, int value);
void outw(int port, int value);
#endif
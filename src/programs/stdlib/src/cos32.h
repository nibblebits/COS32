#ifndef COS32_H
#define COS32_H

struct kernel_info
{
    unsigned int date;
    unsigned int build_no;
};

void print(const char* message);
void kernel_information(struct kernel_info* info);

#endif
#include "cos32.h"
#include "stdio.h"
int main(int argc, char** argv)
{
    struct kernel_info info;
    kernel_information(&info);
    printf("COS32 Shell\n");
    printf("Kernel version: D%i-B%i\n", info.date, info.build_no);
    
    while(1) {}
}
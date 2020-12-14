#include "cos32.h"
int start_get_arguments(struct process_arguments* arguments);

int main(int argc, char** argv);

/**
 * This C function is responsible for calling the main function and passing the arguments
 * of this process to it.
 * 
 * We are called by start.asm
 */
int start_c_func()
{
    struct process_arguments arguments;
    start_get_arguments(&arguments);
    return main(arguments.argc, arguments.argv);
}
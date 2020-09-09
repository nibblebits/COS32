#include "cos32.h"


void cos32_run_command(const char* command, int max)
{
    // Do nothing for now...
}

void cos32_terminal_readline(char *out, int max, bool output_while_typing)
{
    int i = 0;
    for (i = 0; i < max - 1; i++)
    {
        char key = cos32_getkeyblock();
        // Carriage return.
        if (key == 13)
        {
            break;
        }

        if (output_while_typing)
        {
            cos32_putchar(key);
        }
        out[i] = key;
    }

    // Add the null terminator
    out[i] = 0x00;
}
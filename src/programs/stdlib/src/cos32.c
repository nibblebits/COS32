#include "cos32.h"
#include "string.h"
#include "stdio.h"

struct command_argument *cos32_parse_command(const char *command, int max)
{
    struct command_argument *root_command = 0;
    char scommand[1024];
    if (max >= (int)sizeof(scommand))
    {
        return 0;
    }

    strncpy(scommand, command, sizeof(scommand));
    char *token = strtok(scommand, " ");
    if (!token)
    {
        goto out;
    }

    root_command = cos32_malloc(sizeof(struct command_argument));
    if (!root_command)
    {
        printf("Out of memory!\n");
        goto out;
    }

    strncpy(root_command->argument, token, sizeof(root_command->argument));
    root_command->next = 0;

    struct command_argument *current = root_command;
    token = strtok(NULL, " ");
    while (token != 0)
    {
        struct command_argument *new_command = cos32_malloc(sizeof(struct command_argument));
        if (!new_command)
        {
            printf("Out of memory!\n");
            goto out;
        }
        strncpy(new_command->argument, token, sizeof(new_command->argument));
        new_command->next = 0;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    };

out:
    return root_command;
}
void cos32_free_commands(struct command_argument *commands)
{
    struct command_argument *current = commands;
    while (current)
    {
        struct command_argument *next = current->next;
        // cos32_free(current);
        current = next;
    }
}

int cos32_run_command(const char *command, int max)
{
    int res = 0;
    struct command_argument *commands = cos32_parse_command(command, max);
    if (!commands)
    {
        res = -1;
        goto out;
    }

    res = cos32_invoke_command(commands);
    if (res < 0)
    {
        goto out;
    }
out:
    cos32_free_commands(commands);

    // res should equal to zero if all is fine
    return res;
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

        // Backspace
        if (key == 0x08 && i >= 1)
        {
            out[i - 1] = 0x00;
            // -2 because we will +1 when we go onto next continue
            i -= 2;
            continue;
        }
        out[i] = key;
    }

    // Add the null terminator
    out[i] = 0x00;
}
#include "args.h"
#include <SDL3/SDL_log.h>

#define MAX_ARGC        50
#define MAX_ARGV_LENGTH 256

static int   s_argc;
static char* s_argv[MAX_ARGC + 1];
static char  s_cmdline[MAX_ARGV_LENGTH];

void args_init(const int argc, char** argv)
{
    int n = 0;

    for (int j = 0; j < MAX_ARGC && j < argc; j++)
    {
        int i = 0;

        while (n < MAX_ARGV_LENGTH - 1 && argv[j][i])
        {
            s_cmdline[n++] = argv[j][i++];
        }

        if (n < MAX_ARGV_LENGTH - 1)
            s_cmdline[n++] = ' ';
        else
            break;
    }

    if (n > 0 && s_cmdline[n - 1] == ' ')
        s_cmdline[n - 1] = 0; // kill the trailing space

    SDL_Log("Command line: %s\n", s_cmdline);

    for (s_argc = 0; s_argc < MAX_ARGC && s_argc < argc; s_argc++)
    {
        s_argv[s_argc] = argv[s_argc];
    }

    s_argv[s_argc] = " ";
}

int args_check_next(const int last, const char* arg)
{
    for (int i = last + 1; i < s_argc; i++)
    {
        if (!s_argv[i])
            continue;
        if (!strcmp(arg, s_argv[i]))
            return i;
    }

    return 0;
}

int args_check(const char* arg) { return args_check_next(0, arg); }
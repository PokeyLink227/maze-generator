#ifndef __POKEY_TERMINAL_INTERFACE__
#define __POKEY_TERMINAL_INTERFACE__

#include <stdio.h>
#include <stdlib.h>

enum terminal_command_type {
    TYPE_FLAG,
    TYPE_OPTION,
    TYPE_REQUIRED
};

enum terminal_parse_return {
    PARSE_SUCCESS,
    PARSE_ERROR,
    PARSE_HELP
};

typedef struct TerminalCommand {
    int type;
    const char * const name, * const format;
    void *data;
} TerminalCommand;

void init_color();
int terminal_parse(TerminalCommand *, int, char **, int);
const char *get_extension(const char *);


#endif /* __POKEY_TERMINAL_INTERFACE__ */

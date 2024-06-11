#ifndef __POKEY_TERMINAL_INTERFACE__
#define __POKEY_TERMINAL_INTERFACE__

#include <stdio.h>
#include <stdlib.h>

enum terminal_command_type {
    TYPE_FLAG,
    TYPE_OPTION
};

enum terminal_parse_return {
    PARSE_SUCCESS,
    PARSE_ERROR,
    PARSE_HELP
};

struct terminal_command {
    int type;
    const char * const name, * const format;
    void *data;
};

void init_color();
int terminal_parse(struct terminal_command *, int, char **, int);

#endif /* __POKEY_TERMINAL_INTERFACE__ */

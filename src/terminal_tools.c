#include "terminal_tools.h"

struct {
    char color_enabled;
} pokey_terminal_options;

#if defined(_WIN32)
    #include <windows.h>

    void init_color() {
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hStdout, &dwMode);
        SetConsoleMode(hStdout, dwMode|ENABLE_VIRTUAL_TERMINAL_PROCESSING|ENABLE_PROCESSED_OUTPUT);
        pokey_terminal_options.color_enabled = 1;
    }

#else
    void init_color() {
    }
#endif

const char *get_extension(const char *filename) {
    const char *start = filename;

    while (*filename) {
        if (*filename == '.') start = filename;
        filename++;
    }

    return start;
}

/*
does not allow program to accept non option arguments but this can be changed
*/
int terminal_parse(TerminalCommand *cmds, int num_commands, char **args, int num_args) {
    int arg_index, cmd_index, i, j, matched, format_index;
    char options_found[num_commands];

    for (int i = 0; i < num_commands; i++) options_found[i] = 0;

    /* starts at 1 to ignore program name */
    for (arg_index = 1; arg_index < num_args; arg_index++) {
        if (args[arg_index][0] != '-') {
            /* handle non option argument in future */
            if (pokey_terminal_options.color_enabled) printf("\x1b[95mWarning\x1b[0m"); else printf("Warning");
            printf(": excess arguments supplied \"%s\"\n", args[arg_index]);
            continue;
        }

        if (args[arg_index][1] == '-') {
            /* Double dash args '--' */
            if (args[arg_index][2] == '\0') {
                if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
                printf(": Missing option name\n");
                return PARSE_ERROR;
            }

            /* make special check for help command */
            for (i = 2; i < 7; i++) if ("--help"[i] != args[arg_index][i]) {
                matched = 0;
                break;
            }
            if (matched) {
                /*
                printf("Usage: %s {options}\nOptions: [] - Required, {} - Optional\n", args[0]);
                for (i = 0; i < num_commands; i++) if (cmds[i].type == TYPE_FLAG) {
                    printf("  -");
                    printf("%s\t\t%s\n", cmds[i].name, cmds[i].help_text);
                }
                printf("\n");
                for (i = 0; i < num_commands; i++) if (cmds[i].type == TYPE_OPTION) {
                    printf(" --");
                    printf("%s\t\t%s\n", cmds[i].name, cmds[i].help_text);
                }
                */

                return PARSE_HELP;
            }


            matched = 0;
            for (cmd_index = 0; cmd_index < num_commands; cmd_index++) {
                i = 2;
                j = 0;
                int equals = 1;

                while (cmds[cmd_index].name[j] || args[arg_index][i]) {
                    if (cmds[cmd_index].name[j++] != args[arg_index][i++]) {
                        equals = 0;
                        break;
                    }
                }
                if (!equals) continue;

                options_found[cmd_index]++;

                matched = 1;
                format_index = 0;
                while (cmds[cmd_index].format[format_index]) {
                    if (arg_index + format_index + 1 >= num_args || (arg_index + format_index + 1 < num_args && args[arg_index + format_index + 1][0] == '-')) {
                        /* need to make check if arg is optional or not */
                        if (cmds[cmd_index].format[format_index] >= 'a' && cmds[cmd_index].format[format_index] <= 'z') {
                            break;
                        }

                        if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
                        printf(": Not enough arguments supplied to option %s. Use --help for more information\n", args[arg_index]);
                        return PARSE_ERROR;
                    }

                    switch (cmds[cmd_index].format[format_index]) {
                        case 'u': /* unsigned int */
                        case 'U': {
                            *((unsigned int *)cmds[cmd_index].data + format_index) = atoi(args[arg_index + format_index + 1]);
                            break;
                        }
                        case 's': /* string */
                        case 'S': {
                            *((char **)cmds[cmd_index].data) = args[arg_index + format_index + 1];
                            break;
                        }
                        case 'b': /* unsigned byte */
                        case 'B': {
                            *((unsigned char *)cmds[cmd_index].data + format_index) = atoi(args[arg_index + format_index + 1]);
                            break;
                        }
                        case 'h': /* hexadecimal number, 0x prefix optional */
                        case 'H':
                        default:
                            if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
                            printf(": Unsupported format identifier %c\n", cmds[cmd_index].format[format_index]);
                    }

                    format_index++;
                }
                arg_index += format_index;
                break;
            }

            if (!matched) {
                if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
                printf(": Unknown option %s\n", args[arg_index]);
                return PARSE_ERROR;
            }

        } else {
            /* Single dash args '-' */
            if (args[arg_index][1] == '\0') {
                if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
                printf(": Missing flag idenifier\n");
                return PARSE_ERROR;
            }

            i = 1;
            /* match multiple flags per '-' */
            while (args[arg_index][i]) {
                matched = 0;

                for (cmd_index = 0; cmd_index < num_commands; cmd_index++) {
                    if (cmds[cmd_index].type != TYPE_FLAG) continue;
                    if (cmds[cmd_index].name[0] != args[arg_index][i]) continue;
                    *((unsigned char *)(cmds[cmd_index].data)) = 1;
                    matched = 1;
                }

                if (!matched) {
                    if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
                    printf(": Unknown flag -%c\n", args[arg_index][i]);
                    return PARSE_ERROR;
                }

                i++;
            }
        }
    }

    int is_error = 0;
    for (int i = 0; i < num_commands; i++) {
        if (cmds[i].type == TYPE_REQUIRED && options_found[i] == 0) {
            is_error = 1;
            if (pokey_terminal_options.color_enabled) printf("\x1b[91mError\x1b[0m"); else printf("Error");
            printf(": Missing required option --%s\n", cmds[i].name);
        }
    }
    if (is_error) return PARSE_ERROR;

    return PARSE_SUCCESS;
}

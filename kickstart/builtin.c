#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "builtin.h"
#include "command.h"

bool builtin_is_internal(scommand cmd) {
    assert(cmd != NULL);

    const char *internals[] = {"cd", "exit", "help"};
    int n_internals = sizeof(internals) / sizeof(internals[0]);

    char *first = scommand_front(cmd);
    if (first == NULL)
        return false;

    for (int i = 0; i < n_internals; i++) {
        if (strcmp(first, internals[i]) == 0)
            return true;
    }

    return false;
}

bool builtin_alone(pipeline p) {
    assert(p != NULL);
    return (pipeline_length(p) == 1 &&
            builtin_is_internal(pipeline_front(p)));
}

void builtin_run(scommand cmd) {
    assert(cmd != NULL);

    char *first = scommand_front(cmd);
    if (first == NULL)
        return;

    if (strcmp(first, "cd") == 0) {
        char *arg = NULL;
        char *full = scommand_to_string(cmd);
        if (full != NULL) {
            char *space = strchr(full, ' ');
            if (space != NULL) {
                arg = space + 1;
                char *end = strchr(arg, ' ');
                if (end != NULL)
                    *end = '\0';
            }
        }

        char *dir = (arg && *arg != '\0') ? arg : getenv("HOME");
        if (dir == NULL || chdir(dir) != 0) {
            perror("cd");
        }

        free(full);
    } 
    else if (strcmp(first, "exit") == 0) {
        exit(0);
    } 
    else if (strcmp(first, "help") == 0) {
        printf("Comandos internos disponibles:\n");
        printf("  cd [dir]     Cambia el directorio actual\n");
        printf("  exit         Cierra la shell\n");
        printf("  help         Muestra esta ayuda\n");
    } 
    else {
        fprintf(stderr, "Error: comando interno desconocido\n");
    }
}


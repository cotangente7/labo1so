#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

static void show_prompt(void) {
    printf("mybash> ");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    Parser input;
    pipeline p = NULL;
    bool quit = false;

    input = parser_new(stdin);
    if (!input) {
        fprintf(stderr, "Error: no se pudo inicializar el parser.\n");
        return EXIT_FAILURE;
    }

    while (!quit) {
        show_prompt();

        // parse_pipeline puede retornar NULL si no hay comando o error
        p = parse_pipeline(input);
        if (!p) {
            if (parser_at_eof(input)) {
                break; // fin de archivo
            } else {
                // Si hubo un error de parsing, limpiar basura y continuar
                bool garbage_found = false;
                parser_garbage(input, &garbage_found);
                if (garbage_found) {
                    fprintf(stderr, "Entrada no reconocida, se ignora.\n");
                }
                continue;
            }
        }

        quit = parser_at_eof(input);

        if (builtin_alone(p)) {
            // Ejecutar comando interno solo si p está bien construido
            if (!pipeline_is_empty(p)) {
                builtin_run(pipeline_front(p));
            }
        } else {
            execute_pipeline(p);
        }

        pipeline_destroy(p);
        p = NULL;
    }

    parser_destroy(input);
    input = NULL;

    return EXIT_SUCCESS;
}


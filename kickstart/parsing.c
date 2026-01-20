#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"

static scommand parse_scommand(Parser p) {
    scommand sc = scommand_new();
    arg_kind_t kind;
    char *arg = parser_next_argument(p, &kind);

    while (arg != NULL) {
        if (kind == ARG_NORMAL) {
            scommand_push_back(sc, strdup(arg));
        } else if (kind == ARG_INPUT) {
            scommand_set_redir_in(sc, strdup(arg));
        } else if (kind == ARG_OUTPUT) {
            scommand_set_redir_out(sc, strdup(arg));
        } else {
            
            scommand_destroy(sc);
            return NULL;
        }

        arg = parser_next_argument(p, &kind);
    }

    if (scommand_is_empty(sc)) {
        scommand_destroy(sc);
        return NULL;
    }

    return sc;
}


pipeline parse_pipeline(Parser p) {
    assert(p != NULL && !parser_at_eof(p));

    pipeline result = pipeline_new();
    bool error = false;
    scommand cmd = parse_scommand(p);
    error = (cmd == NULL);

    while (!error && cmd != NULL) {
        pipeline_push_back(result, cmd);

        bool was_pipe = false;
        parser_op_pipe(p, &was_pipe);

        if (was_pipe) {
            cmd = parse_scommand(p);
            error = (cmd == NULL);
        } else {
            break; 
        }
    }

    /* Revisar si hay operador de background al final */
    bool was_background = false;
    parser_op_background(p, &was_background);
    pipeline_set_wait(result, !was_background);

    /* Consumir espacios y salto de línea final */
    parser_skip_blanks(p);

    bool garbage_found = false;
    parser_garbage(p, &garbage_found);

    if (error) {
        pipeline_destroy(result);
        return NULL;
    }

    return result;
}

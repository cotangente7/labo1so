#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "command.h"

/* =========================
 *   scommand (simple cmd)
 * ========================= */

typedef struct scommand_s * scommand;
struct scommand_s {
    char **cmd;        // array de cadenas (comando + argumentos)
    int size;          // cantidad de elementos en cmd
    int capacity;      // capacidad actual del array
    char *redir_in;    // redirección de entrada
    char *redir_out;   // redirección de salida
};

scommand scommand_new(void){
    scommand f = malloc(sizeof(struct scommand_s));
    assert(f != NULL);
    f->capacity = 0;
    f->cmd = NULL;
    f->size = 0;
    f->redir_in = NULL;
    f->redir_out = NULL;
    return f;
}

scommand scommand_destroy(scommand self){
    if (!self) return NULL;  // permite destruir NULL

    for (int i = 0; i < self->size; i++) {
        free(self->cmd[i]);
    }
    free(self->cmd);

    free(self->redir_in);
    free(self->redir_out);

    free(self);
    return NULL;
}

void scommand_push_back(scommand self, char *argument) {
    assert(self != NULL && argument != NULL);

    if (self->size == self->capacity) {
        self->capacity = self->capacity == 0 ? 4 : self->capacity * 2;
        self->cmd = realloc(self->cmd, self->capacity * sizeof(char *));
        assert(self->cmd != NULL);
    }

    self->cmd[self->size] = argument;  // no duplicamos el string
    self->size++;
}

bool scommand_is_empty(const scommand self){
    if (!self) return true;
    return (self->size == 0);
}

unsigned int scommand_length(const scommand self){
    if (!self) return 0;
    return self->size;
}

void scommand_set_redir_in(scommand self, char *filename) {
    if (!self) return;

    free(self->redir_in);
    self->redir_in = NULL;

    if (filename != NULL) {
        self->redir_in = filename;  // no duplicamos, los tests esperan el puntero
    }
}

void scommand_set_redir_out(scommand self, char *filename) {
    if (!self) return;

    free(self->redir_out);
    self->redir_out = NULL;

    if (filename != NULL) {
        self->redir_out = filename;  // no duplicamos
    }
}

void scommand_pop_front(scommand self) {
    assert(self != NULL && !scommand_is_empty(self));

    free(self->cmd[0]);
    for (int i = 1; i < self->size; i++) {
        self->cmd[i-1] = self->cmd[i];
    }

    self->size--;
    self->cmd[self->size] = NULL;
}

char * scommand_get_redir_in(const scommand self){
    if (!self) return NULL;
    return self->redir_in;
}

char * scommand_get_redir_out(const scommand self){
    if (!self) return NULL;
    return self->redir_out;
}

char * scommand_front(const scommand self) {
    assert(self != NULL && !scommand_is_empty(self));
    return self->cmd[0];
}

char * scommand_to_string(const scommand self) {
    if (!self) return strdup("");

    int len = 0;
    for (int i = 0; i < self->size; i++) {
        len += strlen(self->cmd[i]) + 1;
    }
    if (self->redir_in)  len += 3 + strlen(self->redir_in);   // " < "
    if (self->redir_out) len += 3 + strlen(self->redir_out);  // " > "

    char *result = malloc(len + 1);
    if (!result) return NULL;
    result[0] = '\0';

    for (int i = 0; i < self->size; i++) {
        strcat(result, self->cmd[i]);
        if (i < self->size - 1) strcat(result, " ");
    }

    if (self->redir_in) {
        strcat(result, " < ");
        strcat(result, self->redir_in);
    }

    if (self->redir_out) {
        strcat(result, " > ");
        strcat(result, self->redir_out);
    }

    return result;
}

/* =========================
 *   pipeline
 * ========================= */

struct pipeline_s {
    scommand *commands;   // array dinámico de comandos simples
    int size;
    int capacity;
    bool wait;
};

pipeline pipeline_new(void) {
    pipeline p = malloc(sizeof(struct pipeline_s));
    assert(p != NULL);
    p->size = 0;
    p->capacity = 4;
    p->commands = calloc(p->capacity, sizeof(scommand));
    p->wait = true;
    return p;
}

pipeline pipeline_destroy(pipeline self) {
    if (!self) return NULL;

    for (int i = 0; i < self->size; i++) {
        if (self->commands[i] != NULL) {
            scommand_destroy(self->commands[i]);
        }
    }
    free(self->commands);
    free(self);
    return NULL;
}

void pipeline_push_back(pipeline self, scommand sc){
    assert(self != NULL && sc != NULL);

    if (self->size == self->capacity) {
        self->capacity = self->capacity == 0 ? 4 : self->capacity * 2;
        self->commands = realloc(self->commands, self->capacity * sizeof(scommand));
        assert(self->commands != NULL);
    }

    self->commands[self->size] = sc;
    self->size++;
}

void pipeline_pop_front(pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));

    scommand_destroy(self->commands[0]);
    for (int i = 1; i < self->size; i++) {
        self->commands[i-1] = self->commands[i];
    }
    self->size--;
    self->commands[self->size] = NULL;
}

void pipeline_set_wait(pipeline self, const bool w){
    if (!self) return;
    self->wait = w;
}

bool pipeline_is_empty(const pipeline self){
    if (!self) return true;
    return (self->size == 0);
}

unsigned int pipeline_length(const pipeline self){
    if (!self) return 0;
    return self->size;
}

scommand pipeline_front(const pipeline self) {
    assert(self != NULL && !pipeline_is_empty(self));
    return self->commands[0];
}

bool pipeline_get_wait(const pipeline self) {
    if (!self) return true;
    return self->wait;
}

char * pipeline_to_string(const pipeline self) {
    if (!self) return strdup("");
    if (pipeline_is_empty(self)) return strdup("");

    size_t bufsize = 1;
    char *result = calloc(bufsize, sizeof(char));
    result[0] = '\0';

    for (int i = 0; i < self->size; i++) {
        char *scmd_str = scommand_to_string(self->commands[i]);
        size_t needed = strlen(result) + strlen(scmd_str) + 4;

        result = realloc(result, needed);
        strcat(result, scmd_str);

        if (i < self->size - 1) {
            strcat(result, " | ");
        }

        free(scmd_str);
    }

    if (!self->wait) {
        size_t needed = strlen(result) + 3;
        result = realloc(result, needed);
        strcat(result, " &");
    }

    return result;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "execute.h"
#include "command.h"
#include <sys/wait.h>


static void run_scommand(scommand cmd) {
    int argc = scommand_length(cmd);
    if (argc == 0) return;

    char **argv = calloc(argc + 1, sizeof(char *));
    if (!argv) {
        perror("calloc");
        exit(1);
    }

    char *cmd_str = scommand_to_string(cmd);
    char *token = strtok(cmd_str, " ");
    int i = 0;
    while (token && i < argc) {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;

    char *infile = scommand_get_redir_in(cmd);
    char *outfile = scommand_get_redir_out(cmd);

    if (infile != NULL) {
        FILE *f = fopen(infile, "r");
        if (!f) { perror("fopen infile"); exit(1); }
        dup2(fileno(f), STDIN_FILENO);
        fclose(f);
    }

    if (outfile != NULL) {
        FILE *f = fopen(outfile, "w");
        if (!f) { perror("fopen outfile"); exit(1); }
        dup2(fileno(f), STDOUT_FILENO);
        fclose(f);
    }

    execvp(argv[0], argv);
    perror("execvp");
    free(argv);
    free(cmd_str);
    exit(1);
}

void execute_pipeline(pipeline apipe) {
    int n = pipeline_length(apipe);
    if (n == 0) return;

    int pipefd[2];
    int prev_fd = -1;
    pid_t *pids = calloc(n, sizeof(pid_t));
    if (!pids) { perror("calloc pids"); exit(1); }

    for (int i = 0; i < n; i++) {
        scommand cmd = pipeline_front(apipe);
        pipeline_pop_front(apipe);

        if (i < n - 1) {
            if (pipe(pipefd) < 0) { perror("pipe"); exit(1); }
        }

        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        else if (pid == 0) {
            if (prev_fd != -1) { dup2(prev_fd, STDIN_FILENO); close(prev_fd); }
            if (i < n - 1) { close(pipefd[0]); dup2(pipefd[1], STDOUT_FILENO); close(pipefd[1]); }
            run_scommand(cmd);
        } else {
            pids[i] = pid;
            if (prev_fd != -1) close(prev_fd);
            if (i < n - 1) { close(pipefd[1]); prev_fd = pipefd[0]; }
        }
    }

    if (pipeline_get_wait(apipe)) {
        for (int i = 0; i < n; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }

    free(pids);
}


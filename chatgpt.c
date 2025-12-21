#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    rl_catch_signals = 0;      // disable default readline SIGINT
    rl_set_signals();

    char* line;
    while (1) {
        line = readline("prompt> ");
        if (!line) break; // EOF
        if (*line) add_history(line);

        printf("You typed: %s\n", line);
        free(line);
    }
    return 0;
}

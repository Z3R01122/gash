#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_ALIASES 32

typedef struct {
    char name[64];
    char value[256];
} alias_t;

alias_t aliases[MAX_ALIASES];
int alias_count = 0;
int is_windows = 0;

void sigint_handler(int signo) {
    write(1, "\n", 1);
}

char *trim(char *str) {
    while (*str == ' ' || *str == '\t') str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = 0;
        end--;
    }
    return str;
}

void add_alias(const char *name, const char *value) {
    if (alias_count < MAX_ALIASES) {
        strncpy(aliases[alias_count].name, name, 63);
        strncpy(aliases[alias_count].value, value, 255);
        aliases[alias_count].name[63] = 0;
        aliases[alias_count].value[255] = 0;
        alias_count++;
    }
}

const char* find_alias(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}

void load_gashrc() {
    char path[512];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, 511, "%s/.gashrc", home);
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        char *trimmed = trim(line);
        if (strncmp(trimmed, "alias ", 6) == 0) {
            char *eq = strchr(trimmed + 6, '=');
            if (eq) {
                *eq = 0;
                char *name = trimmed + 6;
                char *val = eq + 1;
                if (*val == '\'' || *val == '"') val++;
                size_t len = strlen(val);
                if (len > 0 && (val[len - 1] == '\'' || val[len - 1] == '"')) val[len - 1] = 0;
                add_alias(name, val);
            }
        } else if (strncmp(trimmed, "export ", 7) == 0) {
            char *eq = strchr(trimmed + 7, '=');
            if (eq) {
                *eq = 0;
                setenv(trimmed + 7, eq + 1, 1);
            }
        }
    }
    fclose(f);
}

typedef void (*prompt_func_t)(void);

void default_prompt() {
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
    char *base = strrchr(cwd, '/');
    printf("gash:%s$ ", base ? base + 1 : cwd);
}

void windows_prompt() {
    printf("gash> ");
}

prompt_func_t prompt = NULL;

int detect_kernel() {
#ifdef _WIN32
    return 1;
#else
    return 0;
#endif
}

int parse_input(char *input, char **argv) {
    int argc = 0;
    char *token = strtok(input, " ");
    while (token && argc < MAX_ARGS - 1) {
        if (argc == 0) {
            const char *alias_val = find_alias(token);
            if (alias_val) {
                char *alias_copy = strdup(alias_val);
                char *alias_token = strtok(alias_copy, " ");
                while (alias_token && argc < MAX_ARGS - 1) {
                    argv[argc++] = strdup(alias_token);
                    alias_token = strtok(NULL, " ");
                }
                free(alias_copy);
                token = strtok(NULL, " ");
                continue;
            }
        }
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
}

int main() {
    signal(SIGINT, sigint_handler);

    is_windows = detect_kernel();
    prompt = is_windows ? windows_prompt : default_prompt;

    load_gashrc();

    while (1) {
        char *line = readline("");
        if (!line) break;

        char *input = trim(line);
        if (*input == 0) {
            free(line);
            continue;
        }

        add_history(input);

        if (strcmp(input, "exit") == 0) {
            free(line);
            break;
        }

        if (strncmp(input, "cd ", 3) == 0) {
            if (chdir(input + 3) != 0) perror("cd");
            free(line);
            continue;
        }

        if (strncmp(input, "alias", 5) == 0) {
            if (alias_count == 0) {
                printf("no aliases set\n");
            } else {
                for (int i = 0; i < alias_count; i++) {
                    printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
                }
            }
            free(line);
            continue;
        }

        if (strncmp(input, "export ", 7) == 0) {
            char *eq = strchr(input + 7, '=');
            if (eq) {
                *eq = 0;
                if (setenv(input + 7, eq + 1, 1) != 0) {
                    perror("export");
                }
            } else {
                fprintf(stderr, "export: usage: export VAR=VALUE\n");
            }
            free(line);
            continue;
        }

        char *argv[MAX_ARGS];
        char *input_copy = strdup(input);
        parse_input(input_copy, argv);

        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            fprintf(stderr, "gash: command not found: %s\n", argv[0]);
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
        }

        free(input_copy);
        free(line);
    }
    return 0;
}

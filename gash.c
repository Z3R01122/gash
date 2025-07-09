#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_ALIASES 32
#define MAX_HISTORY 1000
#define HISTORY_FILE ".gash_history"
#define MAX_JOBS 32

typedef struct {
    char name[64];
    char value[256];
} alias_t;

typedef struct {
    pid_t pid;
    char cmd[MAX_LINE];
    int stopped;
} job_t;

alias_t aliases[MAX_ALIASES];
job_t jobs[MAX_JOBS];
int alias_count = 0;
int job_count = 0;
int is_windows = 0;
typedef void (*prompt_func_t)(void);
prompt_func_t prompt = NULL;

void sigint_handler(int signo) {
    write(STDOUT_FILENO, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void save_history() {
    HIST_ENTRY **hist_list = history_list();
    if (!hist_list) return;
    char path[512];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; hist_list[i]; i++) {
        fprintf(f, "%s\n", hist_list[i]->line);
    }
    fclose(f);
}

void load_history() {
    char path[512];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == 0) continue;
        char *end = trimmed + strlen(trimmed) - 1;
        while (end > trimmed && (*end == ' ' || *end == '\t' || *end == '\n')) {
            *end-- = '\0';
        }
        if (*trimmed) add_history(trimmed);
    }
    fclose(f);
}

void load_gashrc() {
    char path[512];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/.gashrc", home);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == 0) continue;
        char *end = trimmed + strlen(trimmed) - 1;
        while (end > trimmed && (*end == ' ' || *end == '\t' || *end == '\n')) {
            *end-- = '\0';
        }
        if (strncmp(trimmed, "alias ", 6) == 0) {
            char *eq = strchr(trimmed + 6, '=');
            if (eq) {
                *eq = '\0';
                char *name = trimmed + 6;
                char *val = eq + 1;
                if (*val == '\'' || *val == '"') val++;
                size_t len = strlen(val);
                if (len > 0 && (val[len - 1] == '\'' || val[len - 1] == '"')) val[len - 1] = '\0';
                if (alias_count < MAX_ALIASES) {
                    strncpy(aliases[alias_count].name, name, sizeof(aliases[alias_count].name) - 1);
                    strncpy(aliases[alias_count].value, val, sizeof(aliases[alias_count].value) - 1);
                    aliases[alias_count].name[sizeof(aliases[alias_count].name) - 1] = '\0';
                    aliases[alias_count].value[sizeof(aliases[alias_count].value) - 1] = '\0';
                    alias_count++;
                }
            }
        } else if (strncmp(trimmed, "export ", 7) == 0) {
            char *eq = strchr(trimmed + 7, '=');
            if (eq) {
                *eq = '\0';
                setenv(trimmed + 7, eq + 1, 1);
            }
        } else if (strncmp(trimmed, "gash_prompt=", 12) == 0) {
            char *val = trimmed + 12;
            if (*val == '\'' || *val == '"') val++;
            size_t len = strlen(val);
            if (len > 0 && (val[len - 1] == '\'' || val[len - 1] == '"')) val[len - 1] = '\0';
            setenv("GASH_PROMPT", val, 1);
        }
    }
    fclose(f);
}

void colored_prompt() {
    const char *custom = getenv("GASH_PROMPT");
    if (custom && strlen(custom) > 0) {
        printf("\033[1;32m%s\033[0m", custom);
    } else {
        char cwd[512];
        if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
        char *base = strrchr(cwd, '/');
        printf("\033[1;34mgash:\033[1;33m%s\033[1;34m$\033[0m ", base ? base + 1 : cwd);
    }
    fflush(stdout);
}

void windows_prompt() {
    printf("gash> ");
    fflush(stdout);
}

void init_readline() {
    using_history();
    load_history();
    rl_bind_key('\t', rl_complete);
    rl_read_init_file(NULL);
    stifle_history(MAX_HISTORY);
}

const char* find_alias(const char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}

int parse_input(char *input, char **argv) {
    int argc = 0;
    char *token = strtok(input, " ");
    while (token && argc < MAX_ARGS - 1) {
        if (token[0] == '~') {
            const char *home = getenv("HOME");
            if (home) {
                static char path[512];
                snprintf(path, sizeof(path), "%s%s", home, token + 1);
                token = path;
            }
        }
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
        argv[argc++] = strdup(token);  // strdup all tokens here, no exceptions
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
}

bool handle_builtin(char *input) {
    if (strcmp(input, "exit") == 0) exit(0);
    if (strcmp(input, "pwd") == 0) {
        char cwd[512];
        puts(getcwd(cwd, sizeof(cwd)) ? cwd : "pwd: error");
        return true;
    }
    if (strcmp(input, "clear") == 0) {
        printf("\033[H\033[J");
        return true;
    }
    if (strcmp(input, "help") == 0) {
        puts("gash builtins: cd alias export eval exit help jobs fg bg time");
        return true;
    }
    if (strncmp(input, "cd ", 3) == 0) {
        if (chdir(input + 3) != 0) perror("cd");
        return true;
    }
    if (strcmp(input, "alias") == 0) {
        for (int i = 0; i < alias_count; i++) {
            printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
        }
        return true;
    }
    if (strncmp(input, "export ", 7) == 0) {
        char *eq = strchr(input + 7, '=');
        if (eq) {
            *eq = '\0';
            setenv(input + 7, eq + 1, 1);
        }
        return true;
    }
    if (strncmp(input, "eval ", 5) == 0) {
        system(input + 5);
        return true;
    }
    if (strcmp(input, "jobs") == 0) {
        for (int i = 0; i < job_count; i++) {
            printf("[%d] %s %s\n", i, jobs[i].stopped ? "Stopped" : "Running", jobs[i].cmd);
        }
        return true;
    }
    if (strncmp(input, "fg %", 4) == 0) {
        int job_num = atoi(input + 4);
        if (job_num >= 0 && job_num < job_count) {
            tcsetpgrp(STDIN_FILENO, jobs[job_num].pid);
            kill(jobs[job_num].pid, SIGCONT);
            waitpid(jobs[job_num].pid, NULL, WUNTRACED);
            jobs[job_num].stopped = 0;
        }
        return true;
    }
    if (strncmp(input, "bg %", 4) == 0) {
        int job_num = atoi(input + 4);
        if (job_num >= 0 && job_num < job_count) {
            kill(jobs[job_num].pid, SIGCONT);
            jobs[job_num].stopped = 0;
        }
        return true;
    }
    if (strncmp(input, "time ", 5) == 0) {
        clock_t start = clock();
        system(input + 5);
        printf("Execution time: %.2fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);
        return true;
    }
    return false;
}

void execute_command(char *input, int bg) {
    char *argv[MAX_ARGS];
    char *input_copy = strdup(input);
    int argc = parse_input(input_copy, argv);

    if (bg && job_count < MAX_JOBS) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgid(0, 0);
            execvp(argv[0], argv);
            exit(127);
        } else if (pid > 0) {
            strncpy(jobs[job_count].cmd, input, sizeof(jobs[job_count].cmd) - 1);
            jobs[job_count].pid = pid;
            jobs[job_count].stopped = 0;
            job_count++;
            printf("[%d] %d\n", job_count - 1, pid);
        }
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            fprintf(stderr, "gash: %s: command not found\n", argv[0]);
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                if (job_count < MAX_JOBS) {
                    strncpy(jobs[job_count].cmd, input, sizeof(jobs[job_count].cmd) - 1);
                    jobs[job_count].pid = pid;
                    jobs[job_count].stopped = 1;
                    job_count++;
                    printf("\n[%d] %d\n", job_count - 1, pid);
                }
            }
        }
    }
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(input_copy);
}

int detect_kernel() {
#ifdef _WIN32
    return 1;
#else
    return 0;
#endif
}

void cleanup() {
    save_history();
}

int main() {
    atexit(cleanup);
    signal(SIGINT, sigint_handler);
    is_windows = detect_kernel();
    prompt = is_windows ? windows_prompt : colored_prompt;
    init_readline();
    load_gashrc();

    while (1) {
        prompt();
        char *line = readline("");
        if (!line) break;
        char *input = line;
        while (*input == ' ' || *input == '\t') input++;
        if (*input == 0) {
            free(line);
            continue;
        }
        char *end = input + strlen(input) - 1;
        while (end > input && (*end == ' ' || *end == '\t' || *end == '\n')) {
            *end-- = '\0';
        }
        add_history(input);
        int bg = 0;
        if (end > input && *end == '&') {
            *end = '\0';
            bg = 1;
            while (end > input && (*end == ' ' || *end == '\t')) {
                *end-- = '\0';
            }
        }
        if (handle_builtin(input)) {
            free(line);
            continue;
        }
        execute_command(input, bg);
        free(line);
    }
    return 0;
}

# gash – the guess again shell

gash is a minimal unix-style shell built to be simple, clean, and easy to tweak.  
it runs commands, supports aliases, lets you set environment variables, customize your prompt, and eval raw strings. no bloat, no fuss.

---

## features

- command execution (ls, cd, echo, etc)  
- alias system (alias ll='ls -l')  
- export environment variables (export path=/my/bin:$path)  
- custom prompt support via `.gashrc` or `export gash_prompt`  
- eval support (eval "echo hi && ls")  
- built-in commands: cd, pwd, clear, exit, help

---

## installation

run this in your terminal:

```bash
curl -sSL https://raw.githubusercontent.com/z3r0265return/gash/main/install.sh | bash
```

by default it installs to `/usr/local/bin/gash`

---

## configuration

create a file called `~/.gashrc` to tweak your setup:

```bash
# set your prompt  
gash_prompt="[gash] "

# environment variables  
export myvar="hello"

# aliases  
alias ll='ls -l'  
alias gs='git status'
```

---

## built-in commands

| command  | description                |  
|----------|----------------------------|  
| cd [dir] | change directory           |  
| pwd      | print working directory    |  
| clear    | clear terminal screen      |  
| alias    | list all aliases           |  
| export   | set environment variables  |  
| eval     | run raw commands           |  
| help     | show help info             |  
| exit     | quit the shell             |

---

## goals

- keep it minimal  
- stay posix-like (but not strict)  
- avoid bash-style bloat

---

## project structure

- gash.c – main shell code  
- install.sh – install script  
- readme.md – this file

---

## license

mit license, free and open, no strings attached

---

## credits

built by [z3r0265return](https://github.com/z3r0265return)  
not affiliated with gnu or posix, just doing our own thing

---

if you want me to tweak it more or add anything else, just say.

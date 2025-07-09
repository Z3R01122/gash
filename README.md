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

by default it installs to `/usr/bin/gash`

keep in mind i tested this on arch linux

the dependencies are:
gcc
readline
git
bash
the linux kernel
a distro
a pc

---

## configuration

create a file called `~/.gashrc` to tweak your setup:
example:

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

--- 
## contributing

you can contribute by making a [pull request](https://github.com/z3r0265return/gash/pulls) or forking it.


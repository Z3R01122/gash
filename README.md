# gash – the Guess Again Shell

`gash` is a minimal Unix-style shell designed to be simple, clean, and extendable.  
It supports basic command execution, environment variables, aliases, custom prompts, and `eval`.

---

## 🔧 Features

- ✅ Command execution (`ls`, `cd`, `echo`, etc.)
- ✅ Alias system (`alias ll='ls -l'`)
- ✅ Environment variable export (`export PATH=/my/bin:$PATH`)
- ✅ Custom prompt via `.gashrc` or `export gash_prompt`
- ✅ Eval command (`eval "echo hi && ls"`)
- ✅ Built-in `cd`, `pwd`, `clear`, `exit`, and `help`

---

## 📦 Installation

```bash
curl -sSL https://raw.githubusercontent.com/z3r0265return/gash/main/install.sh | bash
```

> `gash` will be installed to `$HOME/.local/bin/gash` by default.  
> make sure to add `$HOME/.local/bin` to your `$PATH`.

---

## ⚙️ Configuration

Customize behavior with a `~/.gashrc` file:

```bash
# set a custom prompt
gash_prompt="[gash] "

# set environment variables
export MYVAR="hello"

# set aliases
alias ll='ls -l'
alias gs='git status'
```

---

## 💻 Built-in Commands

| Command      | Description                                |
|--------------|--------------------------------------------|
| `cd [dir]`   | Change directory                           |
| `pwd`        | Print current working directory            |
| `clear`      | Clear the terminal screen                  |
| `alias`      | View all aliases                           |
| `export`     | Set environment variables (`VAR=VAL`)      |
| `eval`       | Evaluate a raw command string              |
| `help`       | Show help info                             |
| `exit`       | Quit the shell                             |

---

## 🧠 Goals

- Stay minimal
- Stay POSIX-like (but not POSIX-compliant)
- Avoid bash-style feature bloat

---

## 📁 Project Structure

- `gash.c` – main shell implementation (C)
- `install.sh` – install script
- `README.md` – this file

---

## 📜 License

This project is licensed under the **MIT License**.  
Free software, no nonsense, no restrictions.

---

## 🙏 Credits

Made with care by [z3r0265return](https://github.com/z3r0265return).  
Not affiliated with GNU or POSIX — just doing our own thing.

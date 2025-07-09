#!/usr/bin/env bash

if ! command -v gcc &> /dev/null; then
  echo "[-] gcc not found, install it first"
  exit 1
fi

set -e

echo "[+] Cloning gash..."
if ! git clone https://github.com/z3r0265return/gash.git; then
  echo "[-] failed to clone repo"
  exit 1
fi

cd gash || { echo "[-] failed to enter gash directory"; exit 1; }

echo "[+] Initializing gash..."
if ! gcc -o gash gash.c; then
  echo "[-] compilation failed"
  exit 1
fi

chmod +x ./gash || { echo "[-] failed to set executable permission"; exit 1; }

if ! sudo mv ./gash /usr/local/bin/; then
  echo "[-] failed to move gash binary to /usr/local/bin/"
  exit 1
fi

if ! grep -q "/usr/local/bin/gash" /etc/shells; then
  if ! echo "/usr/local/bin/gash" | sudo tee -a /etc/shells > /dev/null; then
    echo "[-] failed to add /usr/local/bin/gash to /etc/shells"
    exit 1
  fi
fi

echo "[+] done. you can run it with:"
echo "    gash"
echo "or set it as your shell with:"
echo "    chsh -s /usr/local/bin/gash"

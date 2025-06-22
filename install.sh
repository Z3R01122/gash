#!/usr/bin/env bash

if ! command -v gcc &> /dev/null; then
  echo "[-] gcc not found, install it first"
  exit 1
fi

set -e

echo "[+] Cloning gash..."
git clone https://github.com/YOUR_USERNAME/gash.git
cd gash
echo "[+] Initializing gash..."
gcc -o gash gash.c
chmod +x ./gash
sudo mv ./gash /usr/local/bin/
if ! grep -q "/usr/local/bin/gash" /etc/shells; then
    echo "/usr/local/bin/gash" | sudo tee -a /etc/shells > /dev/null
fi

echo "[+] Done. You can run it with:"
echo "    gash"
echo "or set it as your shell with:"
echo "    chsh -s /usr/local/bin/gash"

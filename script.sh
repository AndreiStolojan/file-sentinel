#!/bin/bash

# Verificăm dacă există suficiente argumente
if [ $# -lt 1 ]; then
    echo "Usage: $0 <path>"
    exit 1
fi

path="$1"

# Verificăm dacă fișierul există
if [ ! -f "$path" ]; then
    echo "Error: File '$path' does not exist."
    exit 1
fi

# Acordăm drepturi de scriere pentru a modifica fișierul
chmod 777 "$path"

# Verificăm fiecare caracter din fișier
while read -n1 char; do
    # Verificăm dacă caracterul depășește codul ASCII de 127 (adica valori non-ASCII)
    if [ "$(printf '%d' "'$char")" -gt 127 ]; then
        echo "$path"
        exit 0
    fi
done < "$path"

# Verificăm dacă există cuvinte cheie în fișier
if grep -q -E '\b(malicious|risk|attack)\b' "$path"; then
    echo "$path"
    exit 0
fi

# Restrictionăm complet fișierul
chmod 000 "$path"

# Dacă nu s-au găsit probleme, considerăm fișierul sigur
echo "SAFE"
exit 0

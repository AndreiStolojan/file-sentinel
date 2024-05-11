#!/bin/bash

# Verificăm dacă există suficiente argumente
if [ $# -lt 1 ]; then
    echo "Usage: $0 <path>"
    exit 1
fi

path="$1"

# Verificăm dacă fișierul există
if [ ! -f "$path" ]; then
    echo "Error: Fisierul '$path' nu exista."
    exit 1
fi

chmod 777 "$path"

lines=$(wc -l < "$path")
words=$(wc -w < "$path")
total_chars=$(wc -c < "$path")


# Acordăm drepturi de scriere pentru a modifica fișierul


# Verificăm fiecare caracter din fișier

if [ "$words" -gt 1000 ] && [ "$total_chars" -gt 2000 ] && [ "$lines" -lt 3 ] ; then
    # Verificăm dacă fișierul conține caractere non-ASCII
    if grep -q -P '[^\x00-\x7F]' "$path"; then
        chmod 000 "$path"
        echo "$path"
        
        exit 1
    fi
    
    if grep -q -E '\b(corrupted|dangerous|risk|attack|malware|malicious)\b' "$path"; then
        chmod 000 "$path"
        echo "$path"
        exit 1
    fi
fi




# Restrictionăm complet fișierul
chmod 000 "$path"

# Dacă nu s-au găsit probleme, considerăm fișierul sigur
echo "SAFE"
exit 0

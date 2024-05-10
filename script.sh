#!/bin/bash

if [ "$#" -ne 1 ]; then 
    echo "Numarul de argumente este total incorect!"
    exit 1
fi 

nume_fisier="$1"

if [ ! -f "$nume_fisier" ]; then
    echo "Fisierul $nume_fisier nu exista sau nu este un fisier!"
    exit 1
fi

# Verificăm dacă utilizatorul are permisiuni de citire pentru fișier
if [ ! -r "$nume_fisier" ]; then
    echo "Nu ai permisiunea de a citi fisierul $nume_fisier"
    exit 1
fi

# Verificăm conținutul fișierului
if grep -q -P '[^\x00-\x7F]' "$nume_fisier"; then    
    echo "Fisierul $nume_fisier contine caractere non-ASCII!"
    exit 1
fi 

if grep -q -E 'malefic|periculos|parola' "$nume_fisier"; then
    exit 1
fi

echo "SAFE"
exit 0

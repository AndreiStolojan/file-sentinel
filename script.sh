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

# Salvăm drepturile inițiale ale fișierului
drepturi_initiale=$(stat -c %a "$nume_fisier")

# Verificăm dacă fișierul nu are niciun drept și acordăm drepturile 777
if [ "$drepturi_initiale" == "000" ]; then
    echo "Fisierul nu are niciun drept. Acordam drepturile 777."
    chmod 777 "$nume_fisier"
fi

gasit=0

# Verificăm conținutul fișierului
if grep -q -P '[^\x00-\x7F]' "$nume_fisier"; then    
    echo "Fisierul $nume_fisier contine caractere non-ASCII!"
    gasit=1
fi 

if grep -q -E 'malefic|periculos|parola' "$nume_fisier"; then
    echo "Fisierul $nume_fisier contine cuvinte periculoase"
    gasit=1
fi

# Restaurăm drepturile inițiale ale fișierului
if [ "$drepturi_initiale" == "000" ]; then
    echo "Restauram drepturile initiale."
    chmod "$drepturi_initiale" "$nume_fisier"
fi

exit $gasit

#!/bin/bash

if [ "$#" -ne 1 ] ;then 
    echo "Numarul de argumente este total incorect!"
    exit 1
fi 

nume_fisier="$1"

if [ ! -f "$nume_fisier" ] ; then
    echo "fisierul $nume_fisier nu este un fisier!"
    exit 1
fi

gasit=0

if grep -q -P '[^\x00-\x7F]' "$nume_fisier";
then    
    echo "Fisierul contine caractere non-ASCII!"
    gasit=1
fi 

if grep -q -E 'malefic|periculos|parola' "$nume_fisier";then
    echo "Fisierul contine cuvinte care ma fac sa cred ca este extrem de periculos pentru bunastarea calculatorului meu!"
    gasit=1
fi

if [ "$gasit" -ne 1 ] ; then
    echo "Acest fisier nu este malitios!"
fi

exit $gasit
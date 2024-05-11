README

    Introducere

Acesta este un program simplu scris în limbajul C care efectuează analiza și manipularea fișierelor din directoarele specificate. O nouă caracteristică a fost adăugată pentru a crea un snapshot al fiecărui director specificat. Snapshot-ul conține informații despre toate elementele din directorul respectiv.

Cum să folosești acest program
Precondiții
Asigură-te că ai instalat un compilator C (de exemplu, GCC) și o versiune de shell compatibilă cu scriptul Bash.

Instrucțiuni de utilizare
Compilează programul folosind comanda "gcc -o exec final_program.c" .
După compilare, poți rula programul folosind următorul format:
bash
Copy code
./exec [directoare] -s [izolated_space_dir] -o [director_iesire]
Unde:
[directoare] reprezintă o listă de directoare pe care dorești să le analizezi. Poți specifica unul sau mai multe directoare separate prin spațiu.
-s [izolated_space_dir] specifică directorul unde se mută fișierele infectate.
-o [director_iesire] specifică directorul în care se vor plasa fișierele analizate.
Notă: Programul funcționează numai cu directoare din directorul în care se află codul sursă și executabil. Asigură-te că rulezi programul din directorul corect pentru a evita erori si ca directoarele date ca argument nu se repeta.
Un snapshot al fiecărui director specificat va fi creat automat în directorul de output.

Funcționarea programului

Programul analizează fiecare fișier din directoarele specificate. Dacă un fișier îndeplinește anumite criterii de virus, acesta este mutat în directorul malitios. La sfârșitul analizei, fișierele analizate sunt plasate în directorul de output.

Exemple de utilizare
Exemplu 1: Analiză a unui singur director
bash
Copy code
./exec director1 -s izolated_space_dir -o output_dir
Această comandă va analiza fișierele din director1. Fișierele care îndeplinesc criteriile de virus vor fi mutate în izolated_space_dir, iar elementele analizate vor fi plasate în snapshotul directorului dat ca argument.

Un snapshot al director1 va fi creat în directorul de output.

Exemplu 2: Analiză a mai multor directoare
bash
Copy code
./exec director1 director2 director3 -s izolated_space_dir -o output_dir
Această comandă va analiza fișierele din director1, director2 și director3. Fișierele infectate vor fi mutate în izolated_space_dir, iar elementele analizate vor fi plasate în snapshotul directoarelor aferente lor.

Un snapshot al fiecărui director (director1, director2 și director3) va fi creat în directorul de output.

Contact
Pentru întrebări sau probleme legate de utilizarea acestui program, poți să mă contactezi la adresa email [andreistolojan@gmail.com], sau daca doriti creatia unei aplicatii mobile :) !
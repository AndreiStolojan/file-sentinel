#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <libgen.h>

#define NR_MAX_ARG 10

void move_malicious_file(char *filename, char *malicious_dir) {
    // Obține drepturile de acces ale fișierului
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        fprintf(stderr, "Eroare la obținerea drepturilor de acces pentru %s: %s\n", filename, strerror(errno));
        return;
    }

    // Verifică dacă fișierul nu are niciun drept
    if ((file_stat.st_mode & S_IRWXU) == 0 && (file_stat.st_mode & S_IRWXG) == 0 && (file_stat.st_mode & S_IRWXO) == 0) {
        // Creează calea pentru noul fișier în directorul de fișiere malițioase
        char new_path[1000];
        snprintf(new_path, sizeof(new_path), "%s/%s", malicious_dir, basename(filename));

        // Mută fișierul în directorul de fișiere malițioase
        if (rename(filename, new_path) == -1) {
            fprintf(stderr, "Eroare la mutarea fișierului %s în directorul de fișiere malițioase: %s\n", filename, strerror(errno));
            return;
        }

        printf("Fișierul %s a fost mutat în %s.\n", filename, malicious_dir);
    }
}

void createSnapshot(char *path, char *output_dir, char *malicious_dir)
{
    // Obține drepturile de acces ale elementului
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
        fprintf(stderr, "Eroare la obținerea drepturilor de acces pentru %s: %s\n", path, strerror(errno));
        return;
    }

    // Verifică dacă elementul nu are niciun drept
    if ((file_stat.st_mode & S_IRWXU) == 0 && (file_stat.st_mode & S_IRWXG) == 0 && (file_stat.st_mode & S_IRWXO) == 0) {
        // Mută elementul în directorul de fișiere malițioase
        if (rename(path, malicious_dir) == -1) {
            fprintf(stderr, "Eroare la mutarea elementului %s în directorul de fișiere malițioase: %s\n", path, strerror(errno));
            return;
        }

        printf("Elementul %s a fost mutat în %s.\n", path, malicious_dir);
        return; // Nu facem snapshot pentru elementul malițios
    }

    // Construiește calea completă pentru fișierul snapshot
    char snapshot_path[1000];
    snprintf(snapshot_path, sizeof(snapshot_path), "%s/snapshot_%lu", output_dir, (unsigned long)file_stat.st_ino);

    // Creează fișierul snapshot
    int snapshot_fd = open(snapshot_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (snapshot_fd == -1) {
        fprintf(stderr, "Eroare la crearea fișierului snapshot %s: %s\n", snapshot_path, strerror(errno));
        return;
    }

    // Scrie informații despre element în fișierul snapshot
    char buffer[1000];
    int len = snprintf(buffer, sizeof(buffer), "Nume: %s\n", basename(path));
    write(snapshot_fd, buffer, len);
    len = snprintf(buffer, sizeof(buffer), "Tip: %s\n", S_ISDIR(file_stat.st_mode) ? "Director" : "Fișier");
    write(snapshot_fd, buffer, len);
    
    close(snapshot_fd);
}

void process_directory(char *dir_path, char *output_dir, char *malicious_dir) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Eroare la deschiderea directorului %s: %s\n", dir_path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Ignoră intrările . și ..
        }

        // Construiește calea completă pentru fiecare element din director
        char element_path[1000];
        snprintf(element_path, sizeof(element_path), "%s/%s", dir_path, entry->d_name);


        // Aplică scriptul script.sh pentru fiecare fișier din director
        char command[2000];
        snprintf(command, sizeof(command), "sudo ./script.sh %s", element_path);
        system(command);

        // Verifică rezultatul întoarcerii și drepturile fișierului
        struct stat file_stat;
        if (stat(element_path, &file_stat) == -1) {
            fprintf(stderr, "Eroare la obținerea drepturilor de acces pentru %s: %s\n", element_path, strerror(errno));
            continue;
        }

        // Verifică dacă scriptul a returnat 1 și fișierul nu are drepturi
        if (WEXITSTATUS(system(command)) == 1 && (file_stat.st_mode & S_IRWXU) == 0 && (file_stat.st_mode & S_IRWXG) == 0 && (file_stat.st_mode & S_IRWXO) == 0) {
            move_malicious_file(element_path, malicious_dir);
        }

        // Dacă elementul este un director, procesează-l recursiv
        if (entry->d_type == DT_DIR) {
            process_directory(element_path, output_dir, malicious_dir);
        }
    }

    closedir(dir);
}


int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "Nu au fost specificate suficiente argumente\n");
        exit(EXIT_FAILURE);
    }

    char *malicious_dir = NULL; // Directorul special pentru fișiere malițioase
    char *output_dir = NULL;    // Directorul pentru snapshot-uri

    // Parcurge argumentele pentru a găsi directoarele pentru fișierele malițioase și snapshot-uri
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            // Argumentul de dinainte de "-o" este directorul pentru fișierele malițioase
            malicious_dir = argv[i - 1];
            // Argumentul de după "-o" este directorul de output
            output_dir = argv[i + 1];
        }
    }

    // Verifică dacă ambele directoare au fost specificate
    if (!malicious_dir || !output_dir)
    {
        fprintf(stderr, "Nu au fost specificate directoarele pentru fișierele malițioase și snapshot-uri (-o)\n");
        exit(EXIT_FAILURE);
    }

    // Procesează fiecare director dat ca argument în linia de comandă
    for (int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i+1], "-o") == 0)
        {
            i=i+3;
            break;
        }
        // Ignoră argumentele "-o" și directoarele asociate acestora
        if (strcmp(argv[i], "-o") == 0)
        {
            i += 2; // Treci peste "-o" și următorul argument care este directorul de output
            continue;
        }

        process_directory(argv[i], output_dir, malicious_dir);
    }

    return 0;
}


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

        printf("Calea nouă pentru mutarea fișierului %s este: %s\n", filename, new_path);

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
    // Obține informații despre elementul actual
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
        fprintf(stderr, "Eroare la obținerea informațiilor despre %s: %s\n", path, strerror(errno));
        return;
    }

    // Construiește calea completă pentru fișierul de snapshot
    char snapshot_path[1000];
    snprintf(snapshot_path, sizeof(snapshot_path), "%s/snapshot_%lu", output_dir, (unsigned long)file_stat.st_ino);

    // Deschide fișierul de snapshot pentru scriere
    int snapshot_fd = open(snapshot_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (snapshot_fd == -1) {
        fprintf(stderr, "Eroare la crearea fișierului de snapshot %s: %s\n", snapshot_path, strerror(errno));
        return;
    }

    // Scrie informații despre element în fișierul de snapshot
    char buffer[1000];
    snprintf(buffer, sizeof(buffer), "Nume: %s\n", basename(path));
    write(snapshot_fd, buffer, strlen(buffer));
    snprintf(buffer, sizeof(buffer), "Dimensiune: %lu bytes\n", (unsigned long)file_stat.st_size);
    write(snapshot_fd, buffer, strlen(buffer));
    snprintf(buffer, sizeof(buffer), "Tip: %s\n", S_ISDIR(file_stat.st_mode) ? "Director" : "Fișier");
    write(snapshot_fd, buffer, strlen(buffer));
    
    close(snapshot_fd);
}

void compareAndUpdateSnapshot(char *path, char *output_dir, char *malicious_dir, char *snapshot_path)
{
    // Obține informații despre elementul actual
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
        fprintf(stderr, "Eroare la obținerea informațiilor despre %s: %s\n", path, strerror(errno));
        return;
    }

    // Construiește calea completă pentru fișierul de snapshot
    snprintf(snapshot_path, 1000, "%s/snapshot_%lu", output_dir, (unsigned long)file_stat.st_ino);

    // Deschide fișierul de snapshot pentru citire
    int snapshot_fd = open(snapshot_path, O_RDONLY);
    if (snapshot_fd == -1) {
        fprintf(stderr, "Eroare la deschiderea fișierului de snapshot %s: %s\n", snapshot_path, strerror(errno));
        return;
    }

    // Citeste informațiile din fișierul de snapshot
    char buffer[1000];
    ssize_t bytes_read = read(snapshot_fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        fprintf(stderr, "Eroare la citirea din fișierul de snapshot %s: %s\n", snapshot_path, strerror(errno));
        close(snapshot_fd);
        return;
    }
    close(snapshot_fd);

    // Separă informațiile din fișierul de snapshot în linii
    char *line = strtok(buffer, "\n");
    char name[256], type[256];
    unsigned long size;
    while (line != NULL) {
        if (sscanf(line, "Nume: %s", name) == 1) {
            // Obține numele fișierului din snapshot
        } else if (sscanf(line, "Dimensiune: %lu bytes", &size) == 1) {
            // Obține dimensiunea fișierului din snapshot
        } else if (sscanf(line, "Tip: %s", type) == 1) {
            // Obține tipul fișierului din snapshot
        }
        line = strtok(NULL, "\n");
    }

    // Verifică dacă fișierul a fost modificat
    if (file_stat.st_size != size) {
        // Actualizează snapshot-ul cu noile informații
        createSnapshot(path, output_dir, malicious_dir);
        printf("Snapshot-ul pentru %s a fost actualizat.\n", path);
    } 
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
        char element_path[PATH_MAX];
        snprintf(element_path, sizeof(element_path), "%s/%s", dir_path, entry->d_name);

        // Verifică dacă elementul este un director sau un fișier
        struct stat st;
        if (stat(element_path, &st) == -1) {
            fprintf(stderr, "Eroare la obținerea informațiilor despre %s: %s\n", element_path, strerror(errno));
            continue;
        }

        // Verifică dacă elementul este un director
        if (S_ISDIR(st.st_mode)) {
            // Procesează recursiv directorul
            process_directory(element_path, output_dir, malicious_dir);
        } else {
            // Compară și actualizează snapshot-ul pentru fișierul curent
            char snapshot_path[1000];
            compareAndUpdateSnapshot(element_path, output_dir, malicious_dir, snapshot_path);
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


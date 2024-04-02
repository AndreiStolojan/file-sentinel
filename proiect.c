#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h> 

void fileTree(char *path, int fd)
{
    DIR *director;
    director = opendir(path);
    if (director == NULL)
    {
        fprintf(stderr, "Eroare la deschiderea directorului %s: %s\n", path, strerror(errno));
        return;
    }
    struct dirent *entry;
    char path2[1000];
    struct stat buf;
    struct tm *mtime_info; // Structura pentru a stoca informațiile despre timp
    char mtime_str[20];    // Buffer pentru șirul de caractere al timpului modificării

    while ((entry = readdir(director)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(path2, sizeof(path2), "%s/%s", path, entry->d_name);
            printf("%s\n", path2);
            if (lstat(path2, &buf) == -1)
            {
                fprintf(stderr, "Eroare la citirea metadatelor pentru %s: %s\n", path2, strerror(errno));
                continue;
            }

            // Obțineți informațiile despre timpul modificării și formatați-l ca șir de caractere
            mtime_info = localtime(&buf.st_mtime);
            strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", mtime_info);

            // Scrierea numelui fișierului, metadatelor și orei modificării în format CSV
            printf(fd, "%s,%o,%lu,%s\n", entry->d_name, (unsigned int)buf.st_mode, (unsigned long)buf.st_ino, mtime_str);
            fileTree(path2, fd);
        }
    }
    closedir(director);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Număr de argumente insuficient. Utilizare: %s director\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int idFis;
    idFis = open("snapshot.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (idFis == -1)
    {
        fprintf(stderr, "Eroare la deschiderea fișierului snapshot.txt: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fileTree(argv[1], idFis);
    close(idFis);
    return 0;
}

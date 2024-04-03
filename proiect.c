// sapt 7 enunt: sa primeasca oricate aargumente nu mai mult de 10, niciun argument nu se repeta, programul proceseaza doar directoare, orice altceva va fi ignorat
// captura se va aplica pe toate argumentele care sunt valide
// utilizatorul va compara snapshotul trecut cu cel curent iar daca sunt diferente, cel vechi va fi actualizat, noi alegem cum
// avem un argument suplimentar care va fi directorul de iesire care va contine toate snapshoturile intrarilor specificate 
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// #include <errno.h>
// #include <time.h> 

#define NR_MAX_ARG 10

void fileTree(char *path, int fd)
{
    DIR *director;
    director = opendir(path);
    struct dirent *entry;
    char path2[1000];
    struct stat buf;
    if (director == NULL )
    {
        if(strcmp(path,"-o") == 0)
        {
            return;
        }

        // if(S_ISDIR)

        printf("Eroare la deschiderea directorului %s\n", path);
        return;
    }
    
    // struct tm *mtime_info; // Structura pentru a stoca informațiile despre timp
    // char mtime_str[20];    // Buffer pentru șirul de caractere al timpului modificării

    while ((entry = readdir(director)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "-o") != 0)
        {
            sprintf(path2, "%s/%s", path, entry->d_name);

            if (lstat(path2, &buf) == -1)
            {
                printf( "Eroare la citirea metadatelor pentru %s\n", path2);
                continue;
            }

           
            // mtime_info = localtime(&buf.st_mtime);
            // strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", mtime_info);

            printf("%s,%o,%lu\n", entry->d_name, buf.st_mode, buf.st_ino);
            write(fd,entry->d_name,strlen(entry->d_name));
            write(fd,&buf.st_mode,sizeof(buf.st_mode));
            write(fd,&buf.st_ino,sizeof(buf.st_ino));
    
            fileTree(path2, fd);
            
            // dprintf(fd, "%s,%o,%lu,%s\n", entry->d_name, (unsigned int)buf.st_mode, (unsigned long)buf.st_ino, mtime_str);
            
        }
    }
    closedir(director);
}

int main(int argc, char **argv)
{
    char *director_output = NULL;

    if(strcmp(argv[argc -2], "-o") == 0)
    {
        director_output = argv[argc - 1];
    }

    if (argc >= NR_MAX_ARG + 1)
    {
        printf("Prea multe argumente! numar maxim: %d", NR_MAX_ARG - 1);
        exit(EXIT_FAILURE);
    }

    int snapshotFile;
    snapshotFile = open("snapshot.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (snapshotFile == -1)
    {
        printf( "Eroare la deschiderea fișierului snapshot.txt\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 1; i <= NR_MAX_ARG + 1; i ++)
    {
        fileTree(argv[i], snapshotFile);

        if(argv[i + 1] == NULL)
        {
            break;
        }
    }
    close(snapshotFile);
    return 0;
}

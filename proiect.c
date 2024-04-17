#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include<libgen.h>

#define NR_MAX_ARG 10

void mutare_fisier_malitios(char *nume_fisier, char *directoare_malitioase)
{
    struct stat file_stat;
    if(stat(nume_fisier, &file_stat) == -1)
    {
        fprintf(stderr, "Eroare la obtinerea drepturilor pt fisierul:%s\n",nume_fisier);
        return;
    }

    if((file_stat.st_mode & S_IRWXU) == 0 && (file_stat.st_mode & S_IRWXG) == 0 && (file_stat.st_mode & S_IRWXO) == 0)
    {
        //Creare path pentru directorul cu fisiere malitioase
        char path_nou[1000];
        sprintf(path_nou,"%s/%s",directoare_malitioase,basename(nume_fisier));

        //Mutare fisier malitios in directorul cu fisiere malitioase
        if(rename(nume_fisier, path_nou) == -1)
        {
            fprintf(stderr, "Fisierul %s nu s-a transferat cu succes in directorul pentru fisiere malitioase!\n",nume_fisier);
            return;
        }

        printf("Fisierul %s a fost mutat cu succes!\n",nume_fisier);
    }
}

void createSnapshot(char *path, char *output_dir)
{
    DIR *dir;
    struct dirent *entry;
    struct stat buf;
    char path2[1000];

    if ((dir = opendir(path)) == NULL)
    {
        fprintf(stderr, "Eroare la deschiderea directorului %s\n", path);
        return;
    }

    if (stat(path, &buf) == -1)
    {
        fprintf(stderr, "Eroare la citirea metadatelor pentru %s\n", path);
        closedir(dir);
        return;
    }

    char snapshot_file[100];
    sprintf(snapshot_file, "%s/snapshot_%lu.txt", output_dir, buf.st_ino);

    int snapshot_fd;
    if ((snapshot_fd = open(snapshot_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1)
    {
        fprintf(stderr, "Eroare la deschiderea fișierului %s\n", snapshot_file);
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) //scot fisierul parinte si cele ascunse
        {
            sprintf(path2, "%s/%s", path, entry->d_name);

            if (lstat(path2, &buf) == -1)
            {
                fprintf(stderr, "Eroare la citirea metadatelor pentru %s\n", path2);
                continue;
            }

            if (S_ISDIR(buf.st_mode))
                createSnapshot(path2, output_dir);

            // Scrie informațiile despre fișier în fișierul snapshot
            if (write(snapshot_fd, &buf, sizeof(struct stat)) != sizeof(struct stat))
            {
                fprintf(stderr, "Eroare la scrierea în fișierul %s\n", snapshot_file);
                closedir(dir);
                close(snapshot_fd);
                return;
            }

            // Scrie numele fișierului în fișierul snapshot
            if (write(snapshot_fd, entry->d_name, strlen(entry->d_name)) != strlen(entry->d_name))
            {
                fprintf(stderr, "Eroare la scrierea în fișierul %s\n", snapshot_file);
                closedir(dir);
                close(snapshot_fd);
                return;
            }

            // Scrie inode-ul fișierului în fișierul snapshot
            if (write(snapshot_fd, &(buf.st_ino), sizeof(buf.st_ino)) != sizeof(buf.st_ino))
            {
                fprintf(stderr, "Eroare la scrierea în fișierul %s\n", snapshot_file);
                closedir(dir);
                close(snapshot_fd);
                return;
            }
        }
    }

    closedir(dir);
    close(snapshot_fd);
}


int main(int argc, char **argv)
{
    if (argc < 2) //verificare nr de argumente
    {
        fprintf(stderr, "Nu au fost specificate suficiente argumente\n");
        exit(EXIT_FAILURE);
    }

    if (argc > NR_MAX_ARG + 2) //nr maxim de 10 argumente
    {
        fprintf(stderr, "Prea multe argumente! Numar maxim: %d\n", NR_MAX_ARG);
        exit(EXIT_FAILURE);
    }

    char *output_dir = NULL;
    char *directoare_malitioase = NULL;
    if (strcmp(argv[argc - 2], "-o") == 0) //atribui directorul de output unde are inainte -o
    {

        output_dir = argv[argc - 1];
    }

    for (int i = 1; i < argc - 2; i++)
    {
        pid_t pid = fork(); // Creăm un proces copil

        if (pid < 0)
        {
            // Eroare la fork
            fprintf(stderr, "Eroare la crearea procesului pentru directorul %s\n", argv[i]);
            continue;
        }
        else if (pid == 0)
        {
            // Suntem în procesul copil
            createSnapshot(argv[i], output_dir);
            exit(EXIT_SUCCESS); // Terminăm procesul copil după ce am terminat crearea snapshot-ului
        }
        else
        {
            // Suntem în procesul părinte
            int status;
        }
    }

    waitpid(pid, &status, 0); // Așteptăm terminarea procesului copil
    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        fprintf(stderr, "Eroare la crearea snapshot-ului pentru directorul %s\n", argv[i]);
    }

    return 0;
}

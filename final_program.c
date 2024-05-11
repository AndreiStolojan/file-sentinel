#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <libgen.h>

#define MAX_PATH 300
#define SNAPSHOT_MAX_LEN 100000

struct stat return_lstat(char *name)
{
    struct stat buf;

    if (lstat(name, &buf) < 0)
    {
        fprintf(stderr, "Eroare la lstat pentru %s!\n", name);
        exit(1);
    }

    return buf;
}

int openFile(char *file_name)
{
    int pd_file = open(file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (pd_file < 0)
    {
        fprintf(stderr, "Eroare la deschiderea fisierului %s!\n", file_name);
        exit(3);
    }

    return pd_file;
}

int openFileRead(char *file_name)
{
    int pd_file = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (pd_file < 0)
    {
        fprintf(stderr, "Eroare la deschiderea fisierului %s!\n", file_name);
        exit(3);
    }

    return pd_file;
}

int dirVerify(char *name)
{
    struct stat buf = return_lstat(name);
    return S_ISDIR(buf.st_mode);
}

int hasNoWritePermissions(struct stat fileStat)
{
    // Check if owner and group have write permissions
    if ((fileStat.st_mode & S_IWUSR) && (fileStat.st_mode & S_IWGRP))
    {
        return 0; // File has write permissions for owner and group
    }
    return 1; // File does not have write permissions for owner and group
}

void load_snapshot(char **loaded_text, char *snapshot_directory)
{
    char *text = (char *)malloc((sizeof(char) + 1) * SNAPSHOT_MAX_LEN);
    text[0] = '\0';

    int fd_file = openFileRead(snapshot_directory);
    int read_status = read(fd_file, text, SNAPSHOT_MAX_LEN);
    if (read_status < 0 || read_status == SNAPSHOT_MAX_LEN)
    {
        perror("Not all snapshot was loaded");
    }
    close(fd_file);
    *loaded_text = text;
}

void generateSnapshot(char *snapshot_directory, char *directory_name, int snapshot_fd, char *malicious_directory, int *nr_virus)
{
    struct stat dir_inode = return_lstat(directory_name);

    // Deschidem directorul dat ca argument
    DIR *dir = opendir(directory_name);
    if (dir == NULL)
    {
        fprintf(stderr, "Eroare la deschiderea directorului %s!\n", directory_name);
        exit(EXIT_FAILURE);
    }

    char type_dir[20];
    strcpy(type_dir, "Director");
    char last_modification_dir[50];
    struct tm *tm_info_dir;
    tm_info_dir = localtime(&dir_inode.st_mtime);
    strftime(last_modification_dir, sizeof(last_modification_dir), "%Y-%m-%d %H:%M:%S", tm_info_dir);
    char entry_data_dir[1024];
    snprintf(entry_data_dir, sizeof(entry_data_dir), "Nume: %s, Tip: %s, Size: %ld bytes, Ultima modificare: %s\n",
             directory_name, type_dir, dir_inode.st_size, last_modification_dir);
    if (write(snapshot_fd, entry_data_dir, strlen(entry_data_dir)) != strlen(entry_data_dir))
    {
        perror("Eroare la scrierea în fișierul de snapshot");
        exit(1);
    }

    // Parcurgem directorul și scriem informațiile în fișierul de snapshot
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        int current_is_dir = 0;
        int current_is_file = 0;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char entry_path[MAX_PATH];
        snprintf(entry_path, sizeof(entry_path), "%s/%s", directory_name, entry->d_name);

        struct stat buf = return_lstat(entry_path);

        char type[20];
        if (S_ISDIR(buf.st_mode))
        {
            strcpy(type, "Director");
            current_is_dir = 1;
        }
        else
        {
            strcpy(type, "Fișier");
            current_is_file = 1;
        }

        char last_modification[50];
        struct tm *tm_info;
        tm_info = localtime(&buf.st_mtime);
        strftime(last_modification, sizeof(last_modification), "%Y-%m-%d %H:%M:%S", tm_info);

        char entry_data[1024];
        snprintf(entry_data, sizeof(entry_data), "Nume: %s, Tip: %s, Size: %ld bytes, Ultima modificare: %s\n",
                 entry->d_name, type, buf.st_size, last_modification);

        if (current_is_file)
        {
            if ((buf.st_mode & S_IRWXU) == 0 && (buf.st_mode & S_IRWXG) == 0 && (buf.st_mode & S_IRWXO) == 0)
            {
                int pfd[2];
                if (pipe(pfd) < 0)
                    perror("Pipeul nu merge.");

                pid_t pid = fork();
                if (pid < 0)
                    perror("Eroare la creerea proceselor.\n");

                if (pid == 0)
                {
                    close(pfd[0]); /* inchide capatul de citire; */
                    dup2(pfd[1], 1);

                    execl("script.sh", "script", entry_path, (char *)NULL);

                    close(pfd[1]); /* inchide capatul de scriere */
                    perror("NU O SUPRASCRIS EXECU.\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    close(pfd[1]);
                    char mesaj[1000];
                    int lungime = read(pfd[0], mesaj, 1000);
                    mesaj[lungime - 1] = '\0';

                    if (strcmp("SAFE", mesaj) != 0)
                    {
                        (*nr_virus)++; // Incrementăm numărul de viruși
                        char dest[1000];
                        strcpy(dest, malicious_directory);
                        char *last_slash = strrchr(mesaj, '/');
                        strcat(dest, last_slash);

                        if (rename(mesaj, dest) < 0)
                        {
                            perror("Nu am putut copia\n");
                            exit(EXIT_FAILURE);
                        }
                    }

                    close(pfd[0]);

                    // int return_code = -1;
                    // pid_t finished_pid = 0;
                    // finished_pid = wait(&return_code);
                    // if(WIFEXITED(return_code))
                    //     if(WEXITSTATUS(return_code) != EXIT_SUCCESS)
                    //         printf("Nepotul are o eroare pid=%d: code=%d\n", finished_pid, WEXITSTATUS(return_code));
                }
            }
            else if (write(snapshot_fd, entry_data, strlen(entry_data)) != strlen(entry_data))
            {
                perror("Eroare la scrierea în fișierul de snapshot");
                exit(1);
            }
        }

        if (current_is_dir)
        {
            generateSnapshot(snapshot_directory, entry_path, snapshot_fd, malicious_directory, nr_virus);
        }
    }

    closedir(dir);
}

int treat_dir(char *directory_name, char *snapshot_path, char *snapshot_directory, char *malicious_directory)
{
    char *text_loaded1 = NULL;
    load_snapshot(&text_loaded1, snapshot_path);

    int nr_virus = 0; // Initializăm numărul de viruși cu 0

    int snapshot_fd = openFile(snapshot_path);
    generateSnapshot(snapshot_directory, directory_name, snapshot_fd, malicious_directory, &nr_virus);
    close(snapshot_fd);

    char *text_loaded2 = NULL;
    load_snapshot(&text_loaded2, snapshot_path);

    if (strcmp(text_loaded1, text_loaded2) != 0)
        printf("S-a produs o schimbare in directorul '%s'\n", directory_name);
    else
        printf("Nu sunt schimbari in directorul '%s' !\n", directory_name);

    printf("Numar de virusi in directorul '%s' : %d\n\n", directory_name, nr_virus); // Afisam numarul de viruși detectați

    return nr_virus;
}

int main(int argc, char **argv)
{
    char *snapshot_directory, *malicious_directory;
    int nr_virusi = 0;

    if (argc < 4)
    {
        perror("Numar insuficient de argumente!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            snapshot_directory = argv[i + 1];
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            malicious_directory = argv[i + 1];
        }
    }

    if (!malicious_directory)
    {
        perror("Directorul pentru fisierele periculoase lipseste!\n");
        exit(1);
    }

    if (!snapshot_directory)
    {
        perror("Directorul pentru stocarea snapshot-urilor lipseste!\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "-s") == 0)
        {
            i++;
            continue;
        }

        if (dirVerify(argv[i]))
        {
            char *directory_name = basename(argv[i]);
            char snapshot_path[MAX_PATH];
            struct stat dir_inode = return_lstat(argv[i]);
            snprintf(snapshot_path, sizeof(snapshot_path), "%s/snapshot_%ld", snapshot_directory, dir_inode.st_ino);

            pid_t pid = fork();
            if (pid < 0)
                perror("Eroare la creerea proceselor.\n");

            if (pid == 0)
            {
                int nr_virusi_partial = treat_dir(directory_name, snapshot_path, snapshot_directory, malicious_directory);
                exit(nr_virusi_partial);
            }
            else
            {
                printf("\nProcesul cu PID-ul %d a fost creat.\n", pid);
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status))
                {
                    nr_virusi += WEXITSTATUS(status);
                }
            }
        }
        else
        {
            fprintf(stderr, "Argumentul %s nu este director!\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    int return_code = -1;
    pid_t finished_pid = 0;
    for (int i = 1; i < argc; i++)
    {
        finished_pid = wait(&return_code);
        if (WIFEXITED(return_code))
            if (WEXITSTATUS(return_code) != EXIT_SUCCESS)
                printf("Procesul are o eroare pid=%d: code=%d\n", finished_pid, WEXITSTATUS(return_code));
    }

    printf("\n\nNumarul total de virusi gasiti: %d !\n", nr_virusi);

    return 0;
}

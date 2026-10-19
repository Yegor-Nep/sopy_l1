#include "sys/types.h"
#define _XOPEN_SOURCE 700
#define MAXBUFF 4096
#define CUTBUFF 64

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/uio.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

// join 2 path. returned pointer is for newly allocated memory and must be freed
char* join_paths(const char* path1, const char* path2)
{
    char* res;
    const int l1 = strlen(path1);
    if (path1[l1 - 1] == '/')
    {
        res = malloc(strlen(path1) + strlen(path2) + 1);
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
    }
    else
    {
        res = malloc(strlen(path1) + strlen(path2) + 2);  // additional space for "/"
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
        res[l1] = '/';
        res[l1 + 1] = 0;
    }
    return strcat(res, path2);
}

void usage(int argc, char** argv)
{
    (void)argc;
    fprintf(stderr, "USAGE: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
}

void findData(const char *path, const char *linkpath, const char *linkpath2) {
    FILE *file;
    if((file = fopen(path, "r")) == NULL) {
        ERR("fopen");
    }
    char buffer[MAXBUFF];
    char *pos, *leftValue, *rightValue;
    char Author[MAXBUFF] = "missing!";
    char Genre[CUTBUFF+1] = "missing!";
    char Title[CUTBUFF+1] = "missing!";
    while(fgets(buffer, MAXBUFF, file)) {
        pos = strchr(buffer, ':');
        if(pos == NULL) {
            continue;
        }
        *pos = '\0';
        leftValue = buffer;
        rightValue = pos+1;
        // printf("%s %s\n", leftValue, rightValue);
        if(strcmp(leftValue, "author") == 0) {
            strcpy(Author, rightValue);
        }
        else if(strcmp(leftValue, "title") == 0) {
            // printf("author: %s\n", rightValue);
            strncpy(Title, rightValue, CUTBUFF);
            Title[CUTBUFF] = '\0';
        }
        else if(strcmp(leftValue, "genre") == 0) {
            strncpy(Genre, rightValue, CUTBUFF);
            Genre[CUTBUFF] = '\0';
        }

    }

    // printf("author: %s\ntitle: %s\ngenre: %s\n", Author, Title, Genre);
    if(strcmp(Title, "missing!") != 0) {
        Title[strcspn(Title, "\r\n")] = '\0';
        // printf("%s\n", Title);
        char *tempPath = join_paths(linkpath, "by-title/");
        char *fullpath1 = join_paths(tempPath, Title);
        char *fullpath2 = join_paths(linkpath2, path);
        // printf("%s\n%s\n", fullpath2, fullpath1);
        if(symlink(fullpath2, fullpath1) != 0) {
            // printf("%s\n%s\n", fullpath1, fullpath2);
            ERR("symlink");
        }
        free(fullpath1); free(tempPath); free(fullpath2);
    }

    if(strcmp(Genre, "missing!") != 0) {
        Genre[strcspn(Genre, "\r\n")] = '\0';
        char *tempPath = join_paths(linkpath, "by-genre/");
        char *fullpath1 = join_paths(tempPath, Genre);
        if(mkdir(fullpath1, 0777)) {
            // fprintf(stderr, "dir exists\n");
        }
        free(tempPath);
        char *fullpath3 = join_paths(fullpath1, Title);
        tempPath = join_paths(linkpath2, path);
        char *fullpath2 = join_paths("../", tempPath);
        printf("%s\n%s\n", fullpath3, fullpath2);
        if(symlink(fullpath2, fullpath3) != 0) {
            // printf("%s\n%s\n", fullpath3, fullpath3);
            ERR("symlink");
        }
        free(fullpath1); free(fullpath3); free(fullpath2); free(tempPath);
    }

    fclose(file);
}

void Search(const char *path, const char* linkpath, const char *dir) {
    DIR *dirp;
    struct dirent *dp;
    struct stat buff;
    char cwd[MAXBUFF];
    if(getcwd(cwd, MAXBUFF) == NULL)
        ERR("getcwd");
    if((dirp = opendir(dir)) == NULL) {
        ERR("opendir");
    }
    chdir(dir);

    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if(lstat(dp->d_name, &buff)) {
                ERR("lstat");
            }

            if(S_ISDIR(buff.st_mode) && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                // printf("%s\n", dp->d_name);
                char *tempPath = join_paths("../", linkpath);
                char *toFiletemp = join_paths(path, dp->d_name);
                char *correctToFileTemp = join_paths(toFiletemp, "/");
                Search(toFiletemp, tempPath, dp->d_name);
                free(tempPath); free(toFiletemp); free(correctToFileTemp);
            }

            if(S_ISREG(buff.st_mode)) {
                // printf("%s\n", dp->d_name);
                char *tempPath = join_paths(linkpath, "by-visible-title/");
                char *fullpath1 = join_paths(tempPath, dp->d_name);
                char *fullpath2 = join_paths(path, dp->d_name);
                // printf("%s\n", fullpath1);
                if(symlink(fullpath2, fullpath1) != 0) {
                    ERR("symlink");
                }
                free(fullpath1); free(tempPath); free(fullpath2);
                findData(dp->d_name, linkpath, path);
            }
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    chdir(cwd);
    closedir(dirp);

}

unsigned int contains(const char* name) {
    DIR *dirp;
    struct dirent *dp;
    struct stat buff;
    if((dirp = opendir("index/by-title")) == NULL) {
        ERR("opendir");
    }
    chdir("index/by-title");
    int isThere = 0;
    int correctSize = 0;
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if(lstat(dp->d_name, &buff)) {
                ERR("lstat");
            }

            if(strcmp(name, dp->d_name) == 0) {
                isThere = 1;
                if(stat(dp->d_name, &buff)) {
                    ERR("stat");
                }
                correctSize = buff.st_size;
                break;
            }
        }

    } while(dp != NULL);

    if (errno != 0)
        ERR("readdir");
    closedir(dirp);
    chdir("../..");
    if(isThere) {
        return correctSize;
    }
    return 0;
}
void rData(const char *path) {
    char title[CUTBUFF+1];
    struct iovec iov[2];
    unsigned int size;
    int iovcnt;
    iov[0].iov_base = &size;
    iov[0].iov_len = 4;
    iov[1].iov_base = title;
    iov[1].iov_len = CUTBUFF;
    int fd = open(path, O_RDONLY);
    if(fd == -1) ERR("open");
    ssize_t bytes_read;
    iovcnt = sizeof(iov) / sizeof(struct iovec);
    unsigned int toPrint;
    while((bytes_read = readv(fd, iov, iovcnt)) > 0) {
        title[CUTBUFF] = '\0';
        // printf("%d %s\n",size, title);
        toPrint = contains(title);
        if(toPrint == 0) {
            printf("Book \"%s\" is missing\n", title);
        }
        else if(size != toPrint) {
            printf("Book \"%s\" size mismatch (%d vs %d)\n", title, size, toPrint);
        }

    }
    if(bytes_read == -1) {
        ERR("readv");
    }
    close(fd);

}



int main(int argc, char **argv) {
    if(argc != 2)
        usage(argc, argv);
    char *path = argv[1];

    rData(path);

    return EXIT_SUCCESS;
}

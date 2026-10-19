```c
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

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -n Name -p OCTAL -s SIZE\n", pname);
    exit(EXIT_FAILURE);
}
```


# remembet to free();
```c
char* join_paths(const char* path1, const char* path2) {
    char* res;
    const int l1 = strlen(path1);
    if (path1[l1 - 1] == '/') {
        res = malloc(strlen(path1) + strlen(path2) + 1);
        if (!res) ERR("malloc");
        strcpy(res, path1);
    } else {
        res = malloc(strlen(path1) + strlen(path2) + 2);  
        if (!res) ERR("malloc");
        strcpy(res, path1);
        res[l1] = '/';
        res[l1 + 1] = '\0';
    }
    return strcat(res, path2);
}
```

## 3. Parsowanie plików tekstowych (fgets + ucinanie znaków)
Wzorzec do czytania linijka po linijce, szukania separatora (np. `:`) i wyciągania wartości pozbawionej znaków nowej linii (`\n`, `\r`).

```c
void parse_text_file(const char *path) {
    FILE *file;
    if ((file = fopen(path, "r")) == NULL) ERR("fopen");
    
    char buffer[MAXBUFF];
    char *pos, *leftValue, *rightValue;

    while (fgets(buffer, MAXBUFF, file)) {
        pos = strchr(buffer, ':'); // strstr jak chcemy szukać dłużyszhc sekwencji znaków np strstr(buffer, "==");
        if (pos == NULL) continue; 
        
        *pos = '\0'; 
        leftValue = buffer;
        rightValue = pos + 1;

        rightValue[strcspn(rightValue, "\r\n")] = '\0'; // usuwanie znanepotrzebnyhc na końcu \n;

        if (strcmp(leftValue, "title") == 0) {
            char Title[CUTBUFF + 1];
            strncpy(Title, rightValue, CUTBUFF);
            Title[CUTBUFF] = '\0'; 
            
        }
    }
    fclose(file);
}
```
```c
char *separation = strstr(package, "==");
if(separation == NULL) {
    fprintf(stderr, "package name should be in format <PACKAGE_NAME>==<VERSION>\n");
    exit(EXIT_FAILURE);
}

*separation = '\0'; 

char *pName = package;        
char *pVersion = separation+2; 
```

## 4. Rekurencyjne przeglądanie katalogów 

```c
void traverse_dir(const char *dir_path) {
    DIR *dirp;
    struct dirent *dp;
    struct stat buff;
    
    char cwd[MAXBUFF];
    if (getcwd(cwd, MAXBUFF) == NULL) ERR("getcwd");
    
    if ((dirp = opendir(dir_path)) == NULL) ERR("opendir");
    chdir(dir_path); 

    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (lstat(dp->d_name, &buff)) ERR("lstat");

            if (S_ISDIR(buff.st_mode) && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                traverse_dir(dp->d_name); 
            }

            if (S_ISREG(buff.st_mode)) {
            }
        }
    } while (dp != NULL);

    if (errno != 0) ERR("readdir");
    
    chdir(cwd); 
    closedir(dirp);
}
```

## 5. czytanie za pomoca readv i potem sprawdzanie z stat

```c
#define CUTBUFF 64

void read_database(const char *path, const char *index_dir) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) ERR("open");

    unsigned int size;
    char title[CUTBUFF + 1];
    
    struct iovec iov[2];
    iov[0].iov_base = &size;
    iov[0].iov_len = sizeof(unsigned int); // lub po prostu 4 jak chcemy 4 bajyy 
    iov[1].iov_base = title;
    iov[1].iov_len = CUTBUFF;              

    int iovcnt = sizeof(iov) / sizeof(struct iovec);
    ssize_t bytes_read;

    while ((bytes_read = readv(fd, iov, iovcnt)) > 0) {
        title[CUTBUFF] = '\0'; 
        
        char *full_path_to_check = join_paths(index_dir, title);
        struct stat buff;
        
        if (stat(full_path_to_check, &buff) != 0) {
            printf("Book \"%s\" is missing\n", title);
        } else {
            if (size != (unsigned int)buff.st_size) {
                printf("Book \"%s\" size mismatch (%u vs %lu)\n", title, size, buff.st_size);
            }
        }
        free(full_path_to_check);
    }
    
    if (bytes_read == -1) ERR("readv");
    close(fd);
}
```
## 6. usuniecei jakiejś linii z pliku która spełnia pewien warunek
```c
char buffer[100];
char temp[100];
FILE *file, *newFile;

if((file = fopen("requirements", "r")) == NULL) {
    ERR("fopen");
}
if((newFile = fopen("requirements.temp", "w")) == NULL) {
    ERR("fopen");
}

while(fgets(buffer, 100, file)) {
    sscanf(buffer, "%s", temp);
    if(strcmp(temp, name) == 0) continue; 
    fputs(buffer, newFile);
}

fclose(file);
fclose(newFile);

unlink("requirements"); 
rename("requirements.temp", "requirements"); 
```

## 6. Szablon do getopt
```c
int c;
char *name[argc];   
int itr = 0;
char *pName = NULL;
char *pDelete = NULL;

while ((c = getopt(argc, argv, "cv:i:r:")) != -1) {
    switch (c) {
        case 'c':
            break;
        case 'v':
            v_arg = true;
            name[itr] = optarg; // Przypisanie samego wskaźnika, bez strcpy
            itr++;
            break;
        case 'i':
            break;
        case 'r':
            pDelete = optarg;
            break;
        case '?':
        default:
            usage(argv[0]);
    }
}
```
## 7. rozszerzenie pliku
```c
bool check_extension(const char* name, const char* extension) {
    const char* ldot = strrchr(name, '.');
    if (ldot != NULL) {
        return strncmp(ldot + 1, extension, strlen(extension)) == 0;
    }
    return false;
}
```

## 8. czytanie tego co zostało po flagach getopt
```c
// jakieś tam getopt

// Czytanie tego, co zostało po flagach np. ./program -v -c plik1 plik2
for (int i = optind; i < argc; i++) {
    printf("%s\n", argv[i]);
}
```

## 9. zmienne srodowiskowe
```c
man 3p environ
getenv()
setenv()
putenv()
```
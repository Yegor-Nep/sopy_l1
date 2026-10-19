#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>


#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -p path -e extension -d depth -o\n", pname);
    exit(EXIT_FAILURE);
}

#define MAX_PATH 4096
#define MAX_CATALOG_INPUT 100

bool check_extension(const char* name, const char* extension, size_t length) {
    const char* ldot = strrchr(name, '.');
      if (ldot != NULL)
      {
        if (length == 0)
          length = strlen(extension);
        return strncmp(ldot + 1, extension, length) == 0;
      }
      return 0;
}
void scan_dir(const char *path, const char *extension, int depth, FILE *file) {
    DIR *dirp;
    struct dirent* dp;
    struct stat buff;
    printf("path: %s\n", path);
    if(file) fprintf(file, "path: %s\n", path);
    if((dirp = opendir(path)) == NULL) {
        ERR("opendir");
    }
    char cwd[MAX_PATH];
    getcwd(cwd, MAX_PATH);
    chdir(path);

    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if(lstat(dp->d_name, &buff)) {
                ERR("lstat");
            }

            if(S_ISREG(buff.st_mode)) {
                if(extension == NULL) {
                    if(file) fprintf(file, "%s %ld\n", dp->d_name, buff.st_size);
                    printf("%s %ld\n", dp->d_name, buff.st_size);
                }
                else{
                    if(check_extension(dp->d_name, extension, strlen(extension))) {
                        if(file) fprintf(file,"%s %ld\n", dp->d_name, buff.st_size);
                        printf("%s %ld\n", dp->d_name, buff.st_size);
                    }
                }
            }
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");

    if(depth > 1) {
        rewinddir(dirp);

        do {
            errno = 0;
            if ((dp = readdir(dirp)) != NULL) {
                if(lstat(dp->d_name, &buff)) {
                    ERR("lstat");
                }

                if(S_ISDIR(buff.st_mode) && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0 ) {
                    scan_dir(dp->d_name, extension, depth-1, file);
                }
            }
        } while (dp != NULL);
    }


    closedir(dirp);
    chdir(cwd);

}


int main(int argc, char **argv) {

    int c;
        ssize_t size = -1;
        char* extension = NULL;
        char paths[MAX_CATALOG_INPUT][MAX_PATH];
        int itr = 0;
        int depth = 0;
        char *endptr;
        bool saveToFile = false;
        char *outputfile = NULL;
        const char *env = "L1_OUTPUTFILE";

        while ((c = getopt(argc, argv, "p:e:d:o")) != -1)
            switch (c)
            {
                case 'p':
                    // printf("Sciezka: %s\n", path);
                    strcpy(paths[itr], optarg);
                    itr++;
                    break;
                case 'e':
                    if(extension != NULL) {
                        usage(argv[0]);
                    }
                    extension = optarg;
                    break;
                case 'd':
                    depth = strtol(optarg, &endptr, 10);

                    if(optarg == endptr) {
                        ERR("strol");
                    }
                    break;
                case 'o':
                    saveToFile = true;
                    break;
                case '?':
                default:
                    usage(argv[0]);
            }


        FILE *file = NULL;
        if(saveToFile) {
            if((outputfile = getenv(env)) == NULL) {
                fprintf(stderr, "env var not found\n");
                exit(EXIT_FAILURE);
            }
            if(outputfile != NULL) {
                if((file = fopen(outputfile, "w")) == NULL) {
                    ERR("fopen");
                }
            }
        }
        for(int i = 0; i < itr; i++) {
            // printf("%s\n", paths[i]);
            scan_dir(paths[i], extension, depth, file);
        }

        if(file) {
            fclose(file);
        }
        return EXIT_SUCCESS;
}

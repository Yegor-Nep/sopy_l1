#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define PATH_MAX 4096

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -n Name -p OCTAL -s SIZE\n", pname);
    exit(EXIT_FAILURE);
}


void scanDir(const char* path, const char* filePath) {

    DIR *dir;
    struct dirent *dp;
    struct stat filestat;
    FILE *fptr;
    if((fptr = fopen(filePath, "a")) == NULL) {
        ERR("fopen");
    }
    if((dir = opendir(path)) == NULL) {
        ERR("opendir");
        }
        fprintf(fptr,"Sciezka: %s\n", path);

        do
    {
        errno = 0;
        if ((dp = readdir(dir)) != NULL)
        {
            chdir(path);
            if(lstat(dp->d_name, &filestat)) {
                ERR("fstat");
            }
            
            fprintf(fptr,"%s %ld\n", dp->d_name, filestat.st_size);
            
        }
    } while (dp != NULL);

        if (errno != 0)
        ERR("readdir");
    if (closedir(dir))
        ERR("closedir");

    fclose(fptr);

}

int main(int argc, char** argv) {
    char* path = ".";
    char cwd[PATH_MAX];
    char* filePath = NULL;
    getcwd(cwd, PATH_MAX);
    char cwdTemp[PATH_MAX];
    getcwd(cwdTemp, PATH_MAX);
    char *tempFile = cwdTemp;
    strcat(strcat(tempFile, "/"), "temp.txt");

    int c;
    int difultPath = 1;
     while ((c = getopt(argc, argv, "p:o:")) != -1)
        switch (c)
        {
            case 'p':
                path = optarg;
                // printf("halo%s\n", path);
                scanDir(path, tempFile);
                chdir(cwd);
                difultPath = 0;
                break;
            case 'o':
                if(filePath != NULL)
                    usage(argv[0]);
                filePath = optarg;
                break;
            case '?':
                break;
            default:
                usage(argv[0]);
        }

    if(difultPath)
        scanDir(path, tempFile);

    chdir(cwd);
    
    FILE *fptr;
    if((fptr = fopen(tempFile, "r")) == NULL) {
        ERR("fopen");
    }
    if(filePath != NULL) {
        FILE *file;
        if((file = fopen(filePath, "w")) == NULL) {
            ERR("fopen");
        }
        int d;
        while((d = fgetc(fptr)) != EOF) {
            fprintf(file, "%c", d);
        }


        fclose(file);
    }
    else {

        int d;
        while((d = fgetc(fptr)) != EOF) {
            printf("%c", d);
        }
    }
    fclose(fptr);
    unlink(tempFile);

    return EXIT_SUCCESS;
}
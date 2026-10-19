#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <threads.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -c -v <ENV_NAME> -i <PACKAGE_NAME>==<VERSION> -r <PACKAGE NAME>\n", pname);
    exit(EXIT_FAILURE);
}

#define MAX_LEN 4096

void create_env(const char *name) {

    if(mkdir(name, 0777) != 0) {
        ERR("mkdir");
    }
    chdir(name);
    FILE *file;
    if((file = fopen("requirements", "w")) == NULL) {
        ERR("fopen");
    }
    fclose(file);
    chdir("..");
}

void add_package(const char *env, char *package) {
    DIR *dir;
    if((dir = opendir(env)) == NULL) {
        ERR("opendir");
    }
    closedir(dir);
    chdir(env);
    char *separation;
    separation = strstr(package, "==");
    if(separation == NULL) {
        fprintf(stderr, "package name should be in format <PACKAGE_NAME>==<VERSION>\n");
        exit(EXIT_FAILURE);
    }
    *separation = '\0';
    char *pName = package;
    char *pVersion = separation+2;

    FILE *check;
    if( (check = fopen(pName, "r")) ) {
        fclose(check);
        fprintf(stderr, "Package already exists");
        exit(EXIT_FAILURE);
    }

    FILE *file;
    if((file = fopen("requirements", "a")) == NULL) {
        ERR("fopen");
    }
    fprintf(file, "%s %s\n", pName, pVersion);
    fclose(file);

    if((file = fopen(pName, "w")) == NULL) {
        ERR("fopen");
    }
    chmod(pName, 0444);
    fclose(file);


    chdir("..");
}

void deletePackage(const char *env, char *name) {
    DIR *dir;
    if((dir = opendir(env)) == NULL) {
        ERR("opendir");
    }
    closedir(dir);
    chdir(env);

    FILE *check;
    if((check = fopen(name, "r")) == NULL) {
        ERR("fopen");
    }

    unlink(name);
    // printf("%s\n", pName);
    char buffer[100];
    FILE *file, *newFile;
    if((file=fopen("requirements", "r")) == NULL) {
        ERR("fopen");
    }
    if((newFile=fopen("requirements.temp", "w")) == NULL) {
        ERR("fopen");
    }
    char temp[100];
    while(fgets(buffer, 100, file)) {
        sscanf(buffer, "%s", temp);
        if(strcmp(temp, name) == 0) continue;
        fputs(buffer, newFile);
    }

    fclose(file);
    unlink("requirements");
    fclose(newFile);
    rename("requirements.temp", "requirements");

    chdir("..");
}

int main(int argc, char** argv) {
    int c;
    bool create = false;
    bool v_arg = false;
    char *name[MAX_LEN];
    int itr = 0;
    char *pName = NULL;
    char *pDelete = NULL;
    while ((c = getopt(argc, argv, "cv:i:r:")) != -1)
        switch (c)
        {
            case 'c':
                // printf("Sciezka: %s\n", path);
                create = true;
                break;
            case 'v':
                v_arg = true;
                name[itr] = optarg;
                itr++;
                break;
            case 'i':
                pName = optarg;
                break;
            case 'r':
                pDelete = optarg;
                break;
            case '?':
            default:
                usage(argv[0]);
        }

    if(create == true && v_arg == false) {
        create_env(argv[argc-1]);
        return EXIT_SUCCESS;
    }

    for(int i = 0; i < itr; i++) {
        if(create) create_env(name[i]);
        if(pName) add_package(name[i], pName);
    }
    for(int i = 0; i < itr; i++) {
        if(pDelete) deletePackage(name[i], pDelete);
    }


    return EXIT_SUCCESS;
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    if (argc<2){
        printf("Not enough args\n");
        exit(0);
    }
    char buff;
    char* newArgv[MAXARG];
    int processArg = 0;
    for (int i=1;i<argc;i++){
        newArgv[processArg] = argv[i];
        processArg ++;
    }
    char arg[512];
    int id = 0;
    while (read(0, &buff, 1)==1){
        if (buff==' '||buff=='\n'){
            newArgv[processArg] = (char*)malloc(sizeof(char)*(id));
            for (int i=0;i<id;i++) newArgv[processArg][i] = arg[i];
            processArg ++;
            id = 0;
        }else{
            arg[id] = buff;
            id++;
        }
    }
    
    if (fork()==0){
        exec(argv[1], newArgv);
        // for(int i=0;i<processArg;i++){
        //     printf("%s\n", newArgv[i]);
        // }
        printf("Error occur");
        exit(0);
    }else{
        wait((int*)0);
        for (int i=argc-2;i<processArg;i++){
            //printf("%s ", newArgv[i]);
            free(newArgv[i]);
        }
        exit(0);
    }
}
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int main(int argc, char *argv[]){
    if(argc == 1){
        int pOrg[2], pNext[2];
        pipe(pOrg);
        char buff[4];
        if (fork()==0){
            int this = 0;
            close(pOrg[1]);
            while (read(pOrg[0], buff, 4)==4) {
                if (this == 0){
                    printf("prime %d\n", *(int*)buff);
                    pipe(pNext);
                    if (fork()==0){
                        close(pOrg[0]);
                        close(pNext[1]);
                        pOrg[0] = pNext[0];
                    }else{
                        close(pNext[0]);
                        this = *(int*)buff;
                    }
                }else{
                    if ((*(int*)buff%this)!=0){
                        write(pNext[1], buff, 4);
                    }
                }
            }
            close(pOrg[0]);
            if (this!=0){
                close(pNext[1]);
                wait((int*)0);
            }
            exit(0);
        }else{
            close(pOrg[0]);
            printf("%d\n", 2);
            for (int i=3;i<35;i++){
                if ((i%2)!=0){
                    *(int*)buff = i;
                    write(pOrg[1], buff, 4);
                }
            }
            close(pOrg[1]);
            wait((int*)0);
            exit(0);
        }
        exit(0);
    }else{
        write(1, "Args error!\n", strlen("Args error!\n"));
        exit(0);
    }
}
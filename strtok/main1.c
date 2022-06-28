#include<string.h>
#include<stdio.h>

int main(int argc, char** argv){
    char str[256];
    sprintf(str, "Effective programming mostly in C");

    char* token = strtok(str, " ");
    printf("%s\n", token);
    while(NULL != (token = strtok(NULL, " "))){
        printf("%s\n", token);
    }
    return 0;
}

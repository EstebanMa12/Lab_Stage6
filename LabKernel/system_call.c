#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

#define SYSCALL_NUM 451
#define BASE_LEN 30
#define BASE_NUM 5
#define BASE_NUM_CUBED 11


int main(int argc, char *argv[]){

    if (argc !=3){
        printf("Usage: %s <number> <username>\n", argv[0]);
        return -1;
    }

    int number = atoi(argv[1]);
    char *username = argv[2];

    int dest_len = BASE_LEN + BASE_NUM + BASE_NUM_CUBED + strlen(username);
    char *dest_buffer = malloc(dest_len);

    if (!dest_buffer){
        printf("Error: Unable to allocate buffer\n");
        return -1;
    }

    long result = syscall(SYSCALL_NUM, number, username, strlen(username), dest_buffer, dest_len);

    if (result < 0){
        printf("Error: System call returned %ld\n", result);
        free(dest_buffer);
        return -1;
    }
    printf("Result: %ld\n", result);
    free(dest_buffer);

    return 0;

}
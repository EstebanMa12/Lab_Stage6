#include <stdio.h>
#include <stdlib.h>

void validate_buffer_size(char* buffer, int size) {
    if (buffer == NULL) {
        printf("Error: Buffer is NULL\n");
        return;
    }

    int actual_size = sizeof(buffer) / sizeof(char);
    if (actual_size != size) {
        printf("Error: Buffer size is incorrect. Expected: %d, Actual: %d\n", size, actual_size);
    } else {
        printf("Buffer size is correct\n");
    }
}

int main() {
    char buffer1[10];
    validate_buffer_size(buffer1, 10);

    char buffer2[20];
    validate_buffer_size(buffer2, 20);

    char* buffer3 = malloc(30 * sizeof(char));
    validate_buffer_size(buffer3, 30);

    free(buffer3);

    return 0;
}
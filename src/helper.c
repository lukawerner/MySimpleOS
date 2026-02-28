#include "helper.h"
#include "shellmemory.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int is_alphanumeric(char *string) {
    if (!string) {
        return 0;
    }
    int i = 0;
    while (string[i] != '\0') {
        if (!isalnum(string[i]) && string[i] != '_') {
            return 0;
        }
        i++;
    }
    return 1;
}

void bubble_sort_alphabetical(char *array[], int array_length) {
    for (int i = 0; i < array_length - 1; i++) {
        for (int j = 1; j < array_length - i; j++) {
            char *curr = array[j - 1];
            char *next = array[j];
            if (strcmp(curr, next) > 0) {
                array[j] = curr;
                array[j - 1] = next;
            }
        }
    }
}

void free_array(char *array[], int array_length) {
    for (int i = 0; i < array_length; i++) {
        free(array[i]);
    }
    free(array);
}

char *parseToken(char *input) {
    char *output;

    if (input[0] == '$' && input[1] != '\0') {
        output = mem_get_value(input + 1);
    } else {
        output = input;
    }
    if (strcmp(output, "Variable does not exist") == 0) {
        output = "";
    }
    return output;
}

#include "headers/Helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to generate a unique ID of 8 characters and return a dynamically allocated string
char* generateUniqueID() {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int charsetSize = sizeof(charset) - 1; // Minus 1 to ignore the null terminator

    // Allocate memory for the ID (+1 for null terminator)
    char *id = (char *)malloc((ID_LENGTH + 1) * sizeof(char));

    if (id == NULL) {
        // Memory allocation failed
        printf("Memory allocation failed!\n");
        exit(1);
    }

    // Generate random ID
    for (int i = 0; i < ID_LENGTH; i++) {
        int key = rand() % charsetSize;  // Select random index from charset
        id[i] = charset[key];
    }

    id[ID_LENGTH] = '\0';  // Null-terminate the string

    return id;  // Return the dynamically allocated string
}
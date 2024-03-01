#include "vector.h"

// Function to initialize a vector
void vector_init(Vector *vector) {
    vector->data = (char *)malloc(sizeof(char) * INITIAL_CAPACITY);
    if (vector->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    vector->size = 0;
    vector->capacity = INITIAL_CAPACITY;
}

// Function to add an element to the vector
void vector_add(Vector *vector, char value) {
    if (vector->size >= vector->capacity) {
        // Double the vector if capacity is reached
        vector->capacity *= 2;
        vector->data = (char *)realloc(vector->data, sizeof(char) * vector->capacity);
        if (vector->data == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    vector->data[vector->size++] = value;
}

// Function to get an element from the vector
char vector_get(Vector *vector, size_t index) {
    if (index < 0 || index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    return vector->data[index];
}

// Function to free the memory used by the vector
void vector_free(Vector *vector) {
    free(vector->data);
    vector->size = 0;
    vector->capacity = 0;
}

static const char BASE64_TABLE[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#";

// Get char buffer.
char* vector_get_char_buffer(Vector* vec){
    char* hexBuffer = malloc(vec->size) + 1; // Space for null char.
    for(int i = 0; i < vec->size; i++){
        hexBuffer[i] = BASE64_TABLE[vec->data[i]];
    }
    hexBuffer[vec->size-1] = 0;
    return hexBuffer;
}
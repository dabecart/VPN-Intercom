/**
 * Simple implementation of a char (byte) vector like in C++.
*/

#ifndef VECTOR_DAN_h
#define VECTOR_DAN_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 64

typedef struct {
    char *data;
    size_t size;     // Number of elements currently stored
    size_t capacity; // Capacity of the vector
} Vector;

// Function to initialize a vector
void vector_init(Vector *vector);
// Function to initialize a vector with certain size
void vector_set(Vector *vector, char* content, size_t size);
// Function to add an element to the vector
void vector_add(Vector *vector, char value);
// Function to get an element from the vector
char vector_get(Vector *vector, size_t index);
// Function to free the memory used by the vector
void vector_free(Vector *vector); 
// Get buffer in special code.
char* vector_get_char_buffer(Vector* vec);

#endif
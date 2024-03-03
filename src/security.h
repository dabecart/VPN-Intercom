#ifndef SECURITY__h
#define SECURITY__h

#include "vector.h"

// Encode the data's vector into a buffer.
char* vector_encode(Vector* vec, size_t *outSize);
// Decode the data's buffer into a vector.
Vector vector_decode(char* input);



#endif
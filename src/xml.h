#ifndef XML_DANO_h
#define XML_DANO_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "vector.h"
#include "security.h"

// Maximum number of variables allowed by protocol.
#define XML_DATA_BLOCK_VARIABLE_COUNT 10

// Variable's types in DataFields in XML.
#define XML_TYPE_BOOL       0
#define XML_TYPE_INT        1
#define XML_TYPE_DECIMAL    2
#define XML_TYPE_STRING     3
#define XML_TYPE_BUFFER     4

static const char* XML_TYPE_NAMES[5] = {"bool", "int", "decimal", "string", "buffer"};

// Size of variable types.
#define XML_TYPE_BOOL_SIZE      sizeof(char)
#define XML_TYPE_INT_SIZE       sizeof(uint64_t)
#define XML_TYPE_DECIMAL_SIZE   sizeof(double)
#define XML_TYPE_STRING_SIZE    -1
#define XML_TYPE_BUFFER_SIZE    -1

// Contains information about the message and the messenger.
typedef struct{
    char transmitterAddress[16];
    char receiverAddress[16];
    char transmitterAcronym[30];
    uint64_t dataSize;
    char functionSemantic[30];
    uint64_t sentTime;
    uint64_t responseTime;

    // Flags
    uint8_t expectsAck;
    uint8_t isResponse;
    uint8_t isAck;
}XML_Header;

// A single field inside the data node in an XML file.
typedef struct{
    char name[30];
    uint8_t type;
    uint64_t size;  // Size may vary depending on the variable, except for -1 variables.
    Vector data;
}DataField;

// Array of DataFields.
typedef DataField XML_Array[XML_DATA_BLOCK_VARIABLE_COUNT];
// List of fields.
typedef struct{
    int count;
    XML_Array fields;
}XML_Data;

// Whole XML file.
typedef struct{
    XML_Header header;
    XML_Data data;
}XML_Packet;

// Creates an XMLField variable. Specify if the packet will require acknowledgement by the 
// receiver.
XML_Packet createXMLPacket();

// Function to convert from XML_Field to an XML document per se.
xmlDocPtr toXMLDocument(XML_Packet packet);

// Function to convert from xmlDocPtr to a XML packet.
int toXMLPacket(xmlDocPtr ptr, XML_Packet *pack);

#endif
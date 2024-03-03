#include "xml.h"

// Creates an XMLField variable.
XML_Packet createXMLPacket(){
    XML_Packet ret;
    memset(&ret, 0, sizeof(XML_Packet));
    return ret;
}

// Function to create a new XML node with a given name and content
xmlNodePtr createXmlNode(xmlDocPtr doc, const char *name, const char *content) {
    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST name);
    if (content != NULL) {
        xmlNodePtr contentNode = xmlNewText(BAD_CAST content);
        xmlAddChild(node, contentNode);
    }
    return node;
}

// Function to convert from XML_Field to an XML document per se.
xmlDocPtr toXMLDocument(XML_Packet packet) {
    // Create a new XML document
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    if (doc == NULL) {
        fprintf(stderr, "Failed to create document\n");
        return NULL;
    }

    xmlNodePtr parentNode = xmlNewNode(NULL, BAD_CAST "packet");
    xmlDocSetRootElement(doc, parentNode);

    xmlNodePtr headerNode = xmlNewNode(NULL, BAD_CAST "head");

    char* dumbCopy = malloc(30);    // Will surely be big enough for copying all fields.
    memcpy(dumbCopy, packet.header.receiverAddress, sizeof(packet.header.transmitterAddress));
    xmlNodePtr child = createXmlNode(doc, "tx", packet.header.transmitterAddress);
    xmlAddChild(headerNode, child);

    memcpy(dumbCopy, packet.header.receiverAddress, sizeof(packet.header.receiverAddress));
    child = createXmlNode(doc, "rx", packet.header.receiverAddress);
    xmlAddChild(headerNode, child);

    memcpy(dumbCopy, packet.header.transmitterAcronym, sizeof(packet.header.transmitterAcronym));
    child = createXmlNode(doc, "acron", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%ld", packet.header.dataSize);
    child = createXmlNode(doc, "size", dumbCopy);
    xmlAddChild(headerNode, child);

    memcpy(dumbCopy, packet.header.functionSemantic, sizeof(packet.header.functionSemantic));
    child = createXmlNode(doc, "func", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%ld", packet.header.sentTime);
    child = createXmlNode(doc, "txT", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%ld", packet.header.responseTime);
    child = createXmlNode(doc, "rxT", dumbCopy);
    xmlAddChild(headerNode, child);

    uint8_t flags = packet.header.expectsAck;
    flags |= packet.header.isResponse << 1;
    flags |= packet.header.isAck << 2;
    flags |= packet.header.busy << 3;
    // Neat trick to add leading zeros but with a fixed digit width.
    int decimalPlaces = 1 + (flags>=10) + (flags>=100);
    strcpy(dumbCopy, "000");
    sprintf(dumbCopy + 3 - decimalPlaces, "%d", flags);
    child = createXmlNode(doc, "flag", dumbCopy);
    xmlAddChild(headerNode, child);

    xmlAddChild(parentNode, headerNode);

    xmlNodePtr dataNode = xmlNewNode(NULL, BAD_CAST "data");

    for(int i = 0; i < packet.data.count; i++){
        DataField* currentField = &packet.data.fields[i];
        size_t nodeSize = 0;
        child = createXmlNode(doc, currentField->name, 
                              vector_encode(&currentField->data, &nodeSize));

        // printf("%s %ld\n", currentField->name, currentField->size);
        // xmlNewProp(child, BAD_CAST "type", BAD_CAST XML_TYPE_NAMES[currentField->type]);
        sprintf(dumbCopy, "%ld", nodeSize);
        xmlNewProp(child, BAD_CAST "size", BAD_CAST dumbCopy);

        xmlAddChild(dataNode, child);
    }

    free(dumbCopy);

    xmlAddChild(parentNode, dataNode);

    return doc;
}

int toXMLPacket(xmlDocPtr ptr, XML_Packet *outputPacket){
    XML_Packet pack = createXMLPacket();
    xmlNode *rootNode = xmlDocGetRootElement(ptr);
    if(rootNode!=NULL && xmlStrcmp(rootNode->name, (const xmlChar*)"packet") != 0){
        fprintf(stderr, "This XML is not a packet!\n");
        return -1;
    }
    
    // HEADER
    xmlNode *headerNode = rootNode->children;
    if(headerNode!=NULL && xmlStrcmp(headerNode->name, (const xmlChar*)"head") != 0){
        goto structure_error;
    }

    xmlNode *currNode = headerNode->children;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"tx") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        memcpy(pack.header.transmitterAddress, content, strlen(content));
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"rx") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        memcpy(pack.header.receiverAddress, content, strlen(content));
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"acron") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        memcpy(pack.header.transmitterAcronym, content, strlen(content));
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"size") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        pack.header.dataSize = strtoull(content, NULL, 10); // base 10
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"func") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        memcpy(pack.header.functionSemantic, content, strlen(content));
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"txT") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        pack.header.sentTime = strtoull(content, NULL, 10); // base 10
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"rxT") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        pack.header.responseTime = strtoull(content, NULL, 10); // base 10
    }else{
        goto structure_error;
    }

    currNode = currNode->next;
    if(xmlStrcmp(currNode->name, (const xmlChar*)"flag") == 0){
        char *content = (char*) xmlNodeGetContent(currNode);
        int flags = atoi(content);
        pack.header.expectsAck = flags & 0x1;
        pack.header.isResponse = (flags>>1) & 0x1;
        pack.header.isAck = (flags>>2) & 0x1;
    }else{
        goto structure_error;
    }

    // DATA
    xmlNode *dataNode = headerNode->next;
    for(currNode = dataNode->children; currNode && pack.data.count < XML_DATA_BLOCK_VARIABLE_COUNT; currNode = currNode->next){
        int nameLen = strlen(currNode->name);
        DataField *currField = &pack.data.fields[pack.data.count];
        if(nameLen > sizeof(currField->name)){
            goto structure_error;
        }

        memcpy(currField->name, currNode->name, nameLen);
        char *content = (char*) xmlNodeGetContent(currNode);
        currField->size = strlen(content);
        vector_set(&currField->data, content, currField->size);
        pack.data.count++;
    }

    if(pack.data.count >= XML_DATA_BLOCK_VARIABLE_COUNT){
        goto structure_error;
    }

    xmlFreeDoc(ptr);

    // Save to output.
    memcpy(outputPacket, &pack, sizeof(pack));

    return 0;

structure_error:
    fprintf(stderr, "\x1b[0mThis packet does not follow the defined structure!\n");
    return -1;
}
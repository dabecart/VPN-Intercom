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

    xmlNodePtr headerNode = xmlNewNode(NULL, BAD_CAST "header");

    char* dumbCopy = malloc(30);    // Will surely be big enough for copying all fields.
    memcpy(dumbCopy, packet.header.receiverAddress, sizeof(packet.header.receiverAddress));
    xmlNodePtr child = createXmlNode(doc, "receiverAddress", packet.header.receiverAddress);
    xmlAddChild(headerNode, child);

    memcpy(dumbCopy, packet.header.deviceAcronym, sizeof(packet.header.deviceAcronym));
    child = createXmlNode(doc, "deviceAcronym", dumbCopy);
    xmlAddChild(headerNode, child);
    
    sprintf(dumbCopy, "%d", packet.header.packetSize);
    child = createXmlNode(doc, "packetSize", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%d", packet.header.packetNumber);
    child = createXmlNode(doc, "packetNumber", dumbCopy);
    xmlAddChild(headerNode, child);

    memcpy(dumbCopy, packet.header.functionSemantic, sizeof(packet.header.functionSemantic));
    child = createXmlNode(doc, "functionSemantic", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%d", packet.header.sentTime);
    child = createXmlNode(doc, "sentTime", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%d", packet.header.responseTime);
    child = createXmlNode(doc, "responseTime", dumbCopy);
    xmlAddChild(headerNode, child);

    sprintf(dumbCopy, "%d", packet.header.expectsAck);
    child = createXmlNode(doc, "ack", dumbCopy);
    xmlAddChild(headerNode, child);

    xmlAddChild(parentNode, headerNode);

    xmlNodePtr dataNode = xmlNewNode(NULL, BAD_CAST "data");

    for(int i = 0; i < packet.data.count; i++){
        DataField* currentField = &packet.data.fields[i];
        child = createXmlNode(doc, &currentField->name[0], vector_get_char_buffer(&currentField->data));
        
        xmlNewProp(child, BAD_CAST "type", BAD_CAST XML_TYPE_NAMES[currentField->type]);
        
        snprintf(dumbCopy, sizeof(dumbCopy), "%d", currentField->size);
        xmlNewProp(child, BAD_CAST "size", BAD_CAST dumbCopy);

        xmlAddChild(dataNode, child);
    }

    free(dumbCopy);

    xmlAddChild(parentNode, dataNode);

    return doc;
}

XML_Packet toXMLPacket(xmlDocPtr ptr){
    XML_Packet pack = createXMLPacket(0);
    // TODO: DO THIS SHIAT
    return pack;
}
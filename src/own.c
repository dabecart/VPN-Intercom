#include "own.h"

OwnVariables vbes;
pthread_t runtime_handler;

#ifdef __RPI4__
// #pragma message("Building for the rpi4") 
void* runtime_routine(void* args){
  getVPN_IP(vbes.myIP);

  // Create the XML fields
  XML_Packet packet = createXMLPacket();
  // Prepare the client socket to send the file.
  char *ip_address = "10.215.133.1"; // Destination IP address
  packet.header.functionSemantic = "photo";
  sendXMLPacketTo(packet, ip_address, XML_ACK_NEEDED, NULL, NULL);

  while(1){
    // Do nothing.
  }

  return NULL;
}

int processIncomingPackages(XML_Packet* packet){
  return 0;
}
#endif

#ifdef __RPI2__
// #pragma message("Building for the rpi2")

void* runtime_routine(void* args){
  getVPN_IP(vbes.myIP);

  while(1){
    // Do nothing.
  }

  return NULL;
}

int processIncomingPackages(XML_Packet* packet){
  // Get image.
  int cmp = strcmp(packet->header.functionSemantic, "photo");
  if(cmp == 0){
    char* photoBuffer = NULL;
    size_t photoFileSize;
    int result = takePicture(1280, 720, photoBuffer, &photoFileSize);
    if(!result){ // If no error happens
      Vector v = {photoBuffer, photoFileSize, photoFileSize};
      packet->data.fields[packet->data.count++] = (DataField) {
        "photo",
        XML_TYPE_BUFFER,
        photoFileSize,
        v 
      };
      
      sendResponseTo(packet);
    }
  }
}

#endif

pthread_t* launchRuntimeRoutine(){
  // Create runtime thread.
  if (pthread_create(&runtime_handler, NULL, runtime_routine, NULL) != 0) {
      perror("pthread_create Runtime");
      exit(EXIT_FAILURE);
  }
  return &runtime_handler;
}
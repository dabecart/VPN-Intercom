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
  const char *ip_address = "10.215.133.1"; // Destination IP address
  sendXMLPacketTo(packet, ip_address, XML_ACK_NEEDED);

  while(1){
    // Do nothing.
  }

  return NULL;
}
#endif

#ifdef __RPI2__
// #pragma message("Building for the rpi4") 
void* runtime_routine(void* args){
  while(1){
    // Do nothing.
  }

  return NULL;
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
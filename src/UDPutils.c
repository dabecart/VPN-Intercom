#include "UDPutils.h"

int server_socket;
struct sockaddr_in server_address;

AckWaitlist acklist;

XML_Packet inputPacket;
pthread_mutex_t mutexPacket;
pthread_t rx_handle, tx_handle;
pthread_cond_t responseReceived_wakeUp;

char deviceIP[16];

void* UDPListener(void *args) {
  struct sockaddr_in client_address;
  char inputBuffer[BUFFER_SIZE];
  uint8_t multiPacketInProgress = 0;

  char* multipackBuffer = NULL;
  size_t multipackSize = 0;

  socklen_t client_address_len = sizeof(client_address);

  while(1) {
    memset(inputBuffer, 0, BUFFER_SIZE);

    // Receive message from client
    ssize_t received_bytes = recvfrom(server_socket, inputBuffer, BUFFER_SIZE, 0,
                                      (struct sockaddr *)&client_address, &client_address_len);

    if (received_bytes == -1) {
      perror("Error receiving message");
      continue;
    }

    // Print received message
    printf("Received message from %s:%d: \x1b[35m %s \x1b[0m", inet_ntoa(client_address.sin_addr),
      ntohs(client_address.sin_port), inputBuffer);

    int deviceID = getIPDevice(inet_ntoa(client_address.sin_addr));
    // If the IP is wrong, skip the packet.
    if(deviceID < 0){
      continue;
    }

    xmlDocPtr doc;
    // If the device is in multipacket, save it directly to the multipackBuffer.
    if(acklist[deviceID].multipackMode){
      memcpy(multipackBuffer+multipackSize, inputBuffer, received_bytes);      
      multipackSize += received_bytes;

      // Detected end of multipacket mode!
      if(strcmp(multipackBuffer+multipackSize-4, "end%%") == 0){
        doc = xmlReadMemory(multipackBuffer, multipackSize, "noname.xml", NULL, XML_PARSE_NOBLANKS);
      
        // Disable multipack.
        acklist[deviceID].multipackMode = 0;
        free(multipackBuffer);     
        multipackBuffer = NULL;
      }
    }else{
      // Check if received message is XML or not.
      doc = xmlReadMemory(inputBuffer, received_bytes, "noname.xml", NULL, XML_PARSE_NOBLANKS);
      if (doc == NULL) {
        fprintf(stderr, "Received buffer may not be an XML");
        continue;
      }
    }


    // If the mutex is locked that means that another thread is using the input buffer
    // and this device should send a BUSY signal.
    if(pthread_mutex_trylock(&mutexPacket) != 0){
      // Fetch the flag number without parsing the file.
      char *flag_ptr = strstr(inputBuffer, "<flag>");
      int flagNumber;
      if(sscanf(flag_ptr, "<flag>%d", &flagNumber) == 1){
        flagNumber |= 1<<4; // Add busy flag.
        
        // Leading zeros stuff.
        char newFlag[] = "000";
        int decimalPlaces = 1 + (flagNumber>=10) + (flagNumber>=100);
        sprintf(newFlag + 3 - decimalPlaces, "%d", flagNumber);
        strcpy(flag_ptr + 6, newFlag);  // <flag> has 6 chars.
      }else{
        fprintf(stderr, "Error processing incoming message for busy flag");
        continue;
      }

      // Send to the sender (it's a ping pong but with this flag's payload!).
      if (sendto(server_socket, inputBuffer, received_bytes, 0,
          (struct sockaddr *)&client_address, sizeof(client_address)) == -1) {
        perror("Error sending busy flag");
      }

      continue;
    } // If the mutex wasn't locked, it will lock it now.

    // If it is XML, parse the document and process it.
    // pthread_mutex_lock(&mutexPacket); // Already blocked with trylock
    int parseResult = toXMLPacket(doc, &inputPacket);
    pthread_mutex_unlock(&mutexPacket);

    if(parseResult){
      //XML file could not be parsed.
      continue;
    }

    // If it's a RESPONSE of any kind, check it out. 
    if(inputPacket.header.responseTime>0 && (inputPacket.header.isAck || inputPacket.header.isResponse)){
      printf("Waking up response received\n");
      pthread_cond_signal(&responseReceived_wakeUp);
      continue;
    }

    // If it's a REQUEST...
    // Device asking for multipack!
    if((strcmp(inputPacket.header.functionSemantic, "multipack") == 0)){
      // Device was already in multipack, release data.
      if(acklist[deviceID].multipackMode){
        free(multipackBuffer);
      }
      acklist[deviceID].multipackMode = 1;
      multipackBuffer = (char*) malloc(inputPacket.header.dataSize);
      printf("Multipacket enabled with device %d\n", deviceID);
    }
    
    if(inputPacket.header.expectsAck){
      sendAckPacketTo(&inputPacket);
    }
    // Each device will answer to packets differently.
    processIncomingPackages(&inputPacket);
  }

  return NULL;
}

void* waitingAckThread(void* args){
  static int retVal = 0;
  struct timespec wakeUpTime;
  struct timeval now;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  int repetitions;
  for(repetitions = 0; repetitions < UDP_SENDING_REPETITIONS; repetitions++){
    gettimeofday(&now, NULL);

    wakeUpTime.tv_sec = now.tv_sec + UDP_SENDING_INTERVAL;
    wakeUpTime.tv_nsec = now.tv_usec*1000;  // To be exact to the nanosecond!

    // Either be woken up by an acknowledge function or wait for the timeout.
    pthread_mutex_lock(&mutex);
    int returnCause = pthread_cond_timedwait(&responseReceived_wakeUp, &mutex, &wakeUpTime);
    pthread_mutex_unlock(&mutex);

    // Woken up by something!
    if(returnCause == 0){
      // Lock the message so new incoming messages don't erase it.
      pthread_mutex_lock(&mutexPacket);

      if(inputPacket.header.isAck){
        int IP_port = getIPDevice(inputPacket.header.receiverAddress);
        if(IP_port < 0){
          // Disregard this packet, the IP is not valid!
          pthread_mutex_unlock(&mutexPacket);
          continue;
        }
        printf("Device with ending IP %d woke me up\n", IP_port);

        int deviceIndex;
        for(deviceIndex = 0; deviceIndex < MAX_SUPPORTED_DEVICES; deviceIndex++){
          // Check if this response/ack is for the already sent packet.
          if(deviceIndex==IP_port && acklist[deviceIndex].lastTimeSent==inputPacket.header.sentTime){
            int breakCondition = 0;

            if(acklist[deviceIndex].expectingAck){
              if(acklist[deviceIndex].ackCondition != NULL){
                pthread_cond_signal(acklist[deviceIndex].ackCondition);
              }
              acklist[deviceIndex].expectingAck = 0;
              breakCondition = 1; // found the correspondant ack.
              printf("Ack found!\n");
            }

            if(acklist[deviceIndex].expectingResponse){
              if(acklist[deviceIndex].responseCondition != NULL){
                pthread_cond_signal(acklist[deviceIndex].responseCondition);
              }
              acklist[deviceIndex].expectingResponse = 0;
              breakCondition = 1; // found the correspondant response.
              printf("Response found!\n");
            }

            if(!acklist[deviceIndex].expectingAck && !acklist[deviceIndex].expectingResponse){
              // Everything has been managed, erase it all!
              memset(&acklist[deviceIndex], 0, sizeof(acklist[deviceIndex]));
            }

            if(breakCondition) break; // End the search for correspondant acklist.
          }
        }
        pthread_mutex_unlock(&mutexPacket);

        // If the device index is the same as the MAX SUPPORTED DEVICES that means that
        // this ack signal did not reach its intended target, and so, wait again.
        if(deviceIndex < MAX_SUPPORTED_DEVICES) break; // out of the repetitions loop.
      }
    }else{
      printf("Missing ack %d/%d. Sending again...\n", repetitions+1, UDP_SENDING_REPETITIONS);
    }
  }

  if(repetitions >= UDP_SENDING_REPETITIONS){
    fprintf(stderr, "Exhausted ack repetitions!");
    retVal = -1;
    pthread_exit(&retVal);
  }else{
    printf("Ack received!");
  }

  pthread_mutex_destroy(&mutex);
  pthread_exit(&retVal);
}

// Send a buffer of certain size to an IP address.
int sendBufferTo(char* buffer, int buffer_size, const char* ip_address){
  if(buffer == NULL){
    fprintf(stderr, "No buffer given");
    return -1;
  }
  
  struct sockaddr_in client_address;
  memset(&client_address, 0, sizeof(client_address));
  client_address.sin_family = AF_INET;
  client_address.sin_port = htons(PORT);
  if (inet_aton(ip_address, &client_address.sin_addr) == 0) {
    perror("Invalid address");
    return -1;
  }
  
  if (sendto(server_socket, buffer, buffer_size, 0,
      (struct sockaddr *)&client_address, sizeof(client_address)) == -1) {
    perror("Error sending response");
    printf("Output message: \x1b[31m%s\x1b[0m\n", buffer);
    return -1;
  }else{
      printf("Message sent\n");
  }
  return 0;
}

int sendXMLPacketTo(XML_Packet xml, char* ip_address, uint8_t flags, 
                    pthread_cond_t* ackCondition,  pthread_cond_t* responseCondition){
  if(ip_address == NULL){
    fprintf(stderr, "No IP given");
    return -1;
  }

  // Calculate the size of all individual data fields and the whole packet.
  xml.header.dataSize = 0;
  for(int i = 0; i < xml.data.count; i++){
    DataField *field = &xml.data.fields[i];
    // This size will not be the real size of the outgoing file as it will 
    // be encoded with whatever protocol is implemented in <security.h>
    field->size = field->data.size + strlen(field->name) + 5; // 5: Counting <,>,</,<
    xml.header.dataSize += field->size;
  }

  int estimatedSize = encode_output_size(xml.header.dataSize) + HEADER_OVERHEAD;
  int multipackConnection = 0;
  if(estimatedSize > MAX_DATA_PAYLOAD_SIZE){
    // Enable a multi-packet connection!
    XML_Packet multiPackRequest = createXMLPacket();
    strcpy(multiPackRequest.header.functionSemantic, "multipack");
    multiPackRequest.header.dataSize = estimatedSize;
    int returnCause = sendXMLPacketTo(multiPackRequest, ip_address, XML_ACK_NEEDED | XML_BLOCK, NULL, NULL);
    
    if(returnCause == 0){
      printf("Commencing multi-packet\n");
      multipackConnection = 1;
    }else{ // Timed-out
      fprintf(stderr, "Could not establish multi-packet connection.");
      return -1;
    }
  }  

  // Update sending time of the packet.
  uint64_t startTime = (uint64_t) time(NULL);
  if(xml.header.isResponse || xml.header.isAck){
    // This packet is a response.
    xml.header.responseTime = startTime;
  }else{
    // This packet is a query.
    xml.header.sentTime = startTime;
    // Se the transmitter/receiver IPs.
    memcpy(xml.header.transmitterAddress, deviceIP, strlen(deviceIP));
    memcpy(xml.header.receiverAddress, ip_address, strlen(ip_address));
  }

  if(flags&XML_ACK_NEEDED){
    xml.header.expectsAck = 1;
    // Add to the acknowledge watch list.
    int deviceIPIndex = getIPDevice(ip_address);
    if(deviceIPIndex < 0){
      return -1;
    }

    acklist[deviceIPIndex].expectingAck = 1;
    acklist[deviceIPIndex].lastTimeSent = xml.header.sentTime;
    acklist[deviceIPIndex].ackCondition = ackCondition;
  }

  // Convert to XML file
  xmlDocPtr xmlDoc = toXMLDocument(xml);
  xmlChar *buffer = NULL;
  int buffer_size = 0;
  xmlDocDumpMemory(xmlDoc, &buffer, &buffer_size);

  // Save the XML document to a file
  // const char *filename = "output.xml";
  // int result = xmlSaveFormatFile(filename, xmlDoc, 1); // 1 for formatting
  
  // Send the buffer to the IP.
  int allOk = 0;
  xmlChar *outBuffer = buffer;
  int remainingBytes = buffer_size;
  while(remainingBytes > 0){
    int outSz = MAX_DATA_PAYLOAD_SIZE;
    if(remainingBytes < MAX_DATA_PAYLOAD_SIZE){
      outSz = remainingBytes;
    }
    
    int sendResult = sendBufferTo(outBuffer, outSz, ip_address);

    if((flags&XML_ACK_NEEDED) || multipackConnection){
      if (pthread_create(&tx_handle, NULL, waitingAckThread, NULL) != 0) {
        perror("pthread_create acknowledgement");
        allOk = -1;
        break;
      }
    }

    // If it needs to block, wait until the child thread ends.
    if((flags&XML_BLOCK) || multipackConnection){
      int *ackResult;
      if (pthread_join(tx_handle, (void**)&ackResult) != 0) {
          perror("pthread_join TX_handle");
          allOk = -1;
          break;
      }

      if(sendResult || *ackResult){
        fprintf(stderr, "Error sending packet");
        allOk = -1;
        break;
      }
    }

    outBuffer += outSz;
    remainingBytes -= outSz;
  }

  // Release resources.
  xmlFree(buffer);
  xmlFreeDoc(xmlDoc);

  return allOk;
}

int sendAckPacketTo(XML_Packet *pack){
  // For the ack, as it can be later used for the response, it's better to create
  // a quick copy and just sent an empty ack.
  XML_Packet response = createXMLPacket();
  memcpy(&response.header, &pack->header, sizeof(response.header));
  response.header.isAck = 1;
  sendXMLPacketTo(response, response.header.transmitterAddress, 0, NULL, NULL);
  printf("Ack sent!\n");
  return 0;
}

int sendResponseTo(XML_Packet *pack){
  pack->header.isAck = 0;
  pack->header.isResponse = 1;
  pack->header.expectsAck = 0; // Maybe this will be temporary...

  if(sendXMLPacketTo(*pack, pack->header.transmitterAddress, 0, NULL, NULL)){
    printf("Response was not sent\n");
    return -1;
  }
  return 0;
}

void getVPN_IP(char returnIP[]){
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST]; 

  // Only fetch the IP of the device ONCE and store it to this variable.
  if(!deviceIP[0]){
    if (getifaddrs(&ifaddr) == -1) {
      perror("getifaddrs");
      exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == NULL)
        continue;

      family = ifa->ifa_addr->sa_family;

      if (family == AF_INET || family == AF_INET6) {
        s = getnameinfo(ifa->ifa_addr,
          (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
          host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s != 0) {
          printf("getnameinfo() failed: %s\n", gai_strerror(s));
          exit(EXIT_FAILURE);
        }

        if (family == AF_INET) {
          if (strncmp(host, VPN_PREFIX, strlen(VPN_PREFIX)) == 0) {
            printf("Interface: %s, Address: %s\n", ifa->ifa_name, host);
            break;
          }
        }
      }
    }

    memcpy(deviceIP, host, strlen(host));

    // Clean memory.
    freeifaddrs(ifaddr);
  }

  // Output to argument of function.
  memcpy(returnIP, deviceIP, strlen(deviceIP));
}

int getIPDevice(char* fullIP){
  if(strncmp(fullIP, VPN_PREFIX, strlen(VPN_PREFIX)) == 0){
    return atoi(strrchr(fullIP,'.') + 1);
  }else{
    fprintf(stderr, "Strange IP detected! %s", fullIP);
    return -1;
  }
}

void setupUDPServer(){
    printf("UDP Server start\n");

    // Create UDP socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    // Bind socket to address and port
    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
      perror("Error binding socket");
      close(server_socket);
      exit(EXIT_FAILURE);
    }

    memset(&acklist, 0, sizeof(acklist));

    if(pthread_mutex_init(&mutexPacket, NULL) != 0){
      perror("Mutex init has failed");
      exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&responseReceived_wakeUp, NULL) != 0) {
      // Error handling if initialization fails
      perror("pthread_cond_init");
      exit(EXIT_FAILURE);
    }

    // Store the device IP to later create messages.
    getVPN_IP(deviceIP);

    printf("UDP server listening on port %d...\n", PORT);
}

pthread_t* launchUDPServer(){
  // Create UDPListener thread. 
  if (pthread_create(&rx_handle, NULL, UDPListener, NULL) != 0) {
      perror("pthread_create UDPListener");
      exit(EXIT_FAILURE);
  }
  return &rx_handle;
}

void closeUDPServer(){
  close(server_socket);
}
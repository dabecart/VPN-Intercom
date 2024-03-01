#ifndef UDPutils_h
#define UDPutils_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "xml.h"

#define VPN_PREFIX "10.215.133."
#define MAX_SUPPORTED_DEVICES 10

#define PORT 8888
#define BUFFER_SIZE 1024

typedef struct{
  uint8_t expectingAck;
  uint8_t expectingResponse;
  uint64_t lastTimeSent;
  pthread_cond_t* ackCondition;
  pthread_cond_t* responseCondition;
} AckData;
typedef AckData AckWaitlist[MAX_SUPPORTED_DEVICES];

// Initializes the UDP server.
void setupUDPServer();
pthread_t* launchUDPServer();
void closeUDPServer();

int sendBufferTo(char* buffer, int buffer_size, const char* ip_address);

#define UDP_SENDING_INTERVAL 2 // secs
#define UDP_SENDING_REPETITIONS 3

#define XML_ACK_NEEDED 0x01
#define XML_BLOCK 0x02
#define XML_REPEAT_UNTIL_ACK 0x04
int sendXMLPacketTo(XML_Packet xml, const char* ip_address, uint8_t flags);
int sendAckPacketTo(XML_Packet *pack);

void getVPN_IP(char returnIP[]);


#endif
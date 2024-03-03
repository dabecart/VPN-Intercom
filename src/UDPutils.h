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
#include "own.h"

#define VPN_PREFIX "10.215.133."
#define MAX_SUPPORTED_DEVICES 10

#define PORT 8888
#define BUFFER_SIZE 2048

// Overhead size for the header.
#define HEADER_OVERHEAD 250
// Number of data bytes per payload.
// 1400 is an average payload size in UDP over IPv4.
#define MAX_DATA_PAYLOAD_SIZE 1400
#define WAIT_FOR_MULTIPACK 3 // sec

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
int sendXMLPacketTo(XML_Packet xml, char* ip_address, uint8_t flags, 
                    pthread_cond_t* ackCondition,  pthread_cond_t* responseCondition);
int sendAckPacketTo(XML_Packet *pack);
int sendResponseTo(XML_Packet *pack);

void getVPN_IP(char returnIP[]);

// Returns the last digit of the IP. If it does not start
// with the VPN_PREFIX it will return -1. 
int getIPDevice(char* fullIP);

#endif
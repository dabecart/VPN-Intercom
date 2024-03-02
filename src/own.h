/*
  Own stores the OWN FUNCTIONS: Functions that share the name between different devices but the implementation
  is different! Nevertheless, the purpose of these functions remains the same.
*/

#ifndef OWN_FUNCS_h
#define OWN_FUNCS_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xml.h"
#include "UDPutils.h"
#include "webcam.h"

typedef struct{
  char myIP[16];
}OwnVariables;

pthread_t* launchRuntimeRoutine();
int processIncomingPackages(XML_Packet* packet);

#endif
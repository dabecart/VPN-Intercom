#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "UDPutils.h"
#include "own.h"

int main(){
    setupUDPServer();

    pthread_t *udplistener_ID, *runtime_ID;
    udplistener_ID = launchUDPServer();
    runtime_ID = launchRuntimeRoutine();
    
    // Wait for end of previous thread (will never get here).
    if (pthread_join(*udplistener_ID, NULL) != 0 && pthread_join(*runtime_ID, NULL) != 0) {
        perror("pthread_join main");
        exit(EXIT_FAILURE);
    }

    closeUDPServer();

    return 0;
}


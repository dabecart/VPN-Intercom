#ifndef WEBCAM__h
#define WEBCAM__h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define VIDEO_DEVICE "/dev/video0"
#define NUM_BUFFERS 4

int takePicture(int width, int height, char* output, size_t* outSize);

#endif
#include "webcam.h"

struct buffer {
  void *start;
  size_t length;
};

int takePicture(int width, int height, char* output, size_t* outSize) {
  if(width > 1920 || height > 1920 || width <= 0 || height <= 0){
    fprintf(stderr, "Dimensions are not valid!");
    return -1;
  }

  int fd;
  struct v4l2_capability cap;
  struct v4l2_format fmt;
  struct v4l2_requestbuffers req;
  struct v4l2_buffer buf;
  struct buffer *buffers;
  unsigned int i;

  // Open the device
  fd = open(VIDEO_DEVICE, O_RDWR);
  if (fd == -1) {
    perror("Failed to open video device");
    return -1;
  }

  // Query device capabilities
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
    perror("Failed to query device capabilities");
    close(fd);
    return -1;
  }

  // Set image format
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = width;
  fmt.fmt.pix.height = height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; // or other formats like V4L2_PIX_FMT_YUYV
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
      perror("Failed to set image format");
      close(fd);
      return -1;
  }

  // Set JPEG compression quality
  struct v4l2_jpegcompression jpegcomp;
  memset(&jpegcomp, 0, sizeof(jpegcomp));
  jpegcomp.quality = 90; // Set the quality value as needed, typically between 0 and 100
  if (ioctl(fd, VIDIOC_S_JPEGCOMP, &jpegcomp) == -1) {
    perror("Failed to set JPEG compression quality");
  }

  // Request buffers
  req.count = NUM_BUFFERS;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
      perror("Failed to request buffers");
      close(fd);
      return -1;
  }

  // Allocate buffers
  buffers = calloc(req.count, sizeof(*buffers));
  if (!buffers) {
      perror("Failed to allocate buffers");
      close(fd);
      return -1;
  }

  // Map buffers
  for (i = 0; i < req.count; ++i) {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
      perror("Failed to query buffer");
      close(fd);
      return -1;
    }

    buffers[i].length = buf.length;
    buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffers[i].start == MAP_FAILED) {
      perror("Failed to map buffer");
      close(fd);
      return -1;
    }
  }

  // Queue buffers
  for (i = 0; i < req.count; ++i) {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
      perror("Failed to queue buffer");
      close(fd);
      return -1;
    }
  }

  // Start streaming
  if (ioctl(fd, VIDIOC_STREAMON, &buf.type) == -1) {
    perror("Failed to start streaming");
    close(fd);
    return -1;
  }

  // Capture image
  if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
    perror("Failed to dequeue buffer");
    close(fd);
    return -1;
  }

  // Save image in JPEG format to a buffer
  output = (char *)malloc(buf.bytesused * sizeof(char));
  if (!output) {
    perror("Failed to allocate memory for image buffer");
    close(fd);
    return -1;
  }
  memcpy(output, buffers[buf.index].start, buf.bytesused);
  *outSize = buf.bytesused;

  // To save the photo to a file.
  // FILE *fp = fopen("captured_photo.jpg", "wb");
  // if (!fp) {
  //   perror("Failed to open file for writing");
  //   close(fd);
  //   return -1;
  // }
  // fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
  // fclose(fp);

  // Stop streaming
  if (ioctl(fd, VIDIOC_STREAMOFF, &buf.type) == -1) {
    perror("Failed to stop streaming");
    close(fd);
    return -1;
  }

  // Unmap buffers
  for (i = 0; i < req.count; ++i) {
    munmap(buffers[i].start, buffers[i].length);
  }
  free(buffers);

  // Close the device
  close(fd);

  return 0;
}
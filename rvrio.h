#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "rvrdefs.h"

#define BYTESINFO    1
#define VERBOSE      2
#define VERYVERBOSE  3

void init_port();
void set_logging_level(int level);

void messageSend(u_int8_t cid, u_int8_t did, 
                 u_int8_t source, u_int8_t target, 
                 u_int8_t * payload, u_int8_t payload_length); 
u_int8_t * messageSendAndRecv(u_int8_t cid, u_int8_t did, 
                              u_int8_t source, u_int8_t target, 
                              u_int8_t * payload, u_int8_t payload_length, u_int8_t recv_length); 

#include "rvrio.h"
#include "rvrdefs.h"
#include "rvrcmd.h"

/* The python SDK padded this to 16 bytes...does it have to be? */
u_int8_t * echo(char * sequence, int length) {
    u_int8_t * buffer = (u_int8_t *)malloc(sizeof(u_int8_t)*16);
    u_int8_t * b = buffer;
    if (length > 16) length = 16;
    for (int i=0; i<length; i++) *(b++) = *(sequence++);
    /* TODO: For some reason, the reply comes back from the RVR with an extra 0x00 in the first byte */
    u_int8_t * ret = messageSendAndRecv(ECHO_COMMAND, API_AND_SHELL_DEVICE_ID, NO_SOURCE, TARGET_ID_NORDIC, buffer, 16, 17);
    return ++ret;
}


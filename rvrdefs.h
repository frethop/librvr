#ifndef __RVRDEFS_H
#define __RVRDEFS_H 1

#include <sys/types.h>

union flag {
    u_int8_t allbits;
    struct bits {
        unsigned int is_response:1;
        unsigned int response_requested:1;
        unsigned int response_requested_if_error:1;
        unsigned int is_activity:1;
        unsigned int has_target:1;
        unsigned int has_source:1;
        unsigned int UNUSED:1;
        unsigned int has_more_flags:1;
    } flag_bits;
};

union locationID {
    u_int8_t wholeID;
    struct {
        unsigned int port:4;
        unsigned int node:4;
    } parts;
};

struct header {
    union flag flags;
    union locationID targetID;     /* optional, depends on flags */
    union locationID sourceID;     /* optional, depends on flags */
    u_int8_t deviceID;     /* command group ... see defines below */
    u_int8_t commandID;
    u_int8_t sequence_num;
};

#define MESSAGE_START 0x8D
#define MESSAGE_END   0xD8

struct message {
    struct header * msghdr;
    u_int8_t * payload;
    int loadlength;
};

#define NO_SOURCE       0x00
#define NO_DESTINATION  0x00

#define ECHO_COMMAND                         0x00
#define SLEEP_COMMAND                        0x01 
#define RAW_MOTOR_COMMAND                    0x01
#define GET_BLUETOOTH_ADVERT                 0x05
#define RESET_YAW_COMMAND                    0x07
#define WAKE_COMMAND                         0x0D
#define GET_BATTERY_PERCENTAGE_COMMAND       0x10
#define GET_BATTERY_VOLTAGE_STATE_COMMAND    0x17
#define GET_BATTERY_VOLTAGE_IN_VOLTS_COMMAND 0x25
#define SET_ALL_LEDS_COMMAND                 0x1A

#define API_AND_SHELL_DEVICE_ID   0x10
#define SYSTEM_INFO_DEVICE_ID     0x11
#define POWER_DEVICE_ID           0x13
#define DRIVE_DEVICE_ID           0x16
#define SENSOR_DEVICE_ID          0x18
#define CONNECTION_DEVICE_ID      0x19
#define IO_DEVICE_ID              0x1A

#define TARGET_ID_NORDIC          0x01
#define TARGET_ID_ST              0x02

#endif
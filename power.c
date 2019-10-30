#include "rvrio.h"
#include "rvrdefs.h"
#include "rvrcmd.h"

void wake() {
    messageSend(WAKE_COMMAND, POWER_DEVICE_ID, NO_SOURCE, TARGET_ID_NORDIC, NULL, 0);
}

void sleepr() {
    messageSend(SLEEP_COMMAND, POWER_DEVICE_ID, NO_SOURCE, TARGET_ID_NORDIC, NULL, 0);
}

u_int8_t get_battery_percentage() { 
    u_int8_t * percentage = messageSendAndRecv(GET_BATTERY_PERCENTAGE_COMMAND, POWER_DEVICE_ID, NO_SOURCE, TARGET_ID_NORDIC, NULL, 0, 2);
    return percentage[0]*256 + percentage[1];
} 

u_int8_t get_battery_voltage_state() {
    u_int8_t * state = messageSendAndRecv(GET_BATTERY_VOLTAGE_STATE_COMMAND, POWER_DEVICE_ID, NO_SOURCE, TARGET_ID_NORDIC, NULL, 0, 1);
    return *state;
}


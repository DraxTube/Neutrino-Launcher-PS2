#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include "ds34usb_client.h"

static SifRpcClientData_t ds34usb;
static u8 rpcbuf[64] __attribute__((aligned(64)));

int ds34usb_init() {
    ds34usb.server = NULL;
    while (!ds34usb.server) {
        if (SifBindRpc(&ds34usb, 0x18E3878E, 0) < 0) return 0;
        nopdelay();
    }
    return 1;
}

int ds34usb_get_status(int port) {
    rpcbuf[0] = port;
    if (SifCallRpc(&ds34usb, 2, 0, rpcbuf, 1, rpcbuf, 1, NULL, NULL) == 0)
        return rpcbuf[0];
    return 0;
}

int ds34usb_get_data(int port, u8 *data) {
    rpcbuf[0] = port;
    if (SifCallRpc(&ds34usb, 7, 0, rpcbuf, 1, rpcbuf, 18, NULL, NULL) == 0) {
        memcpy(data, rpcbuf, 18);
        return 1;
    }
    return 0;
}

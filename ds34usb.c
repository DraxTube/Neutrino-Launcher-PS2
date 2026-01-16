#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include "ds34usb.h"

#define DS34USB_INIT           1
#define DS34USB_GET_STATUS     2
#define DS34USB_GET_BDADDR     3
#define DS34USB_SET_BDADDR     4
#define DS34USB_SET_RUMBLE     5
#define DS34USB_SET_LED        6
#define DS34USB_GET_DATA       7
#define DS34USB_RESET          8

#define DS34USB_BIND_RPC_ID 0x18E3878E

static SifRpcClientData_t ds34usb;
static u8 rpcbuf[64] __attribute__((aligned(64)));
static u8 ds34usb_inited = 0;

int ds34usb_init() {
    ds34usb.server = NULL;
    do {
        if (SifBindRpc(&ds34usb, DS34USB_BIND_RPC_ID, 0) < 0) return 0;
        nopdelay();
    } while (!ds34usb.server);
    ds34usb_inited = 1;
    return 1;
}

int ds34usb_deinit() { ds34usb_inited = 0; return 1; }

int ds34usb_get_status(int port) {
    if (!ds34usb_inited) return 0;
    rpcbuf[0] = port;
    if (SifCallRpc(&ds34usb, DS34USB_GET_STATUS, 0, rpcbuf, 1, rpcbuf, 1, NULL, NULL) == 0)
        return rpcbuf[0];
    return 0;
}

int ds34usb_get_data(int port, u8 *data) {
    if (!ds34usb_inited) return 0;
    rpcbuf[0] = port;
    int ret = (SifCallRpc(&ds34usb, DS34USB_GET_DATA, 0, rpcbuf, 1, rpcbuf, 18, NULL, NULL) == 0);
    memcpy(data, rpcbuf, 18);
    return ret;
}

// (Le altre funzioni come set_led o rumble possono essere aggiunte qui seguendo lo stesso schema)

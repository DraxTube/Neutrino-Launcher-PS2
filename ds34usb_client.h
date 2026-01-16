#ifndef DS34USB_CLIENT_H
#define DS34USB_CLIENT_H
#include <tamtypes.h>
#define DS34USB_STATE_RUNNING 0x08
int ds34usb_init();
int ds34usb_get_status(int port);
int ds34usb_get_data(int port, u8 *data);
#endif

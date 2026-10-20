#ifndef FISH_USB_HPM_H
#define FISH_USB_HPM_H
/* Run on the scene thread: MSC SD reads/writes never execute in an ISR. */
void fish_usb_poll(void);
#endif

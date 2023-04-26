#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <gd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <tenma.h>

#define CLR_DISPLAY_BG 0
#define CLR_BTN_LOW 1
#define CLR_BTN_HI 3
#define CLR_BTN_STAT 6
#define CLR_MAIN_BORDER 11
#define CLR_CH1 12
#define CLR_CH2 13
#define CLR_MATH 14
#define CLR_TEXT 15

const int width = 400;
const int height = 240;

libusb_context *ctx = NULL;
libusb_device_handle *dso_728705 = NULL;

int allb(uint32_t *buffer, int l) {
    int i;
    for (i = 0; i < l; i++) {
        if (buffer[i] != 0xbbbbbbbb) return 0;
    }
    return 1;
}

gdImagePtr dso_728705_getFrame() {
    if (dso_728705 == NULL) return NULL;

    uint32_t buffer[64/4];
    uint8_t data[1] = { 0xE2 };
    int done = 0;
    int all = 0;
    int x = 0;
    int y = 0;
    int r = 0;

    gdImagePtr img = gdImageCreate(width, height);

    if (libusb_bulk_transfer(dso_728705, 2, data, 1, &done, 1000) < 0) {
        gdImageDestroy(img);
        return NULL;
    }

    r = libusb_bulk_transfer(dso_728705, 0x82, (uint8_t *)buffer, 64, &done, 1000);
    if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) {
        gdImageDestroy(img);
        return NULL;
    }

    while (!allb(buffer, 64/4)) {
        r = libusb_bulk_transfer(dso_728705, 0x82, (uint8_t *)buffer, 64, &done, 1000);
        if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) {
            gdImageDestroy(img);
            return NULL;
        }
    }

    time_t ts = time(NULL);

    while (all < 48000) {
        int i;
        for (i = 0; i < done/4; i++) {
            int p1 = (buffer[i] >> 28) & 0x0F;
            int p2 = (buffer[i] >> 24) & 0x0F;
            int p3 = (buffer[i] >> 20) & 0x0F;
            int p4 = (buffer[i] >> 16) & 0x0F;
            int p5 = (buffer[i] >> 12) & 0x0F;
            int p6 = (buffer[i] >> 8) & 0x0F;
            int p7 = (buffer[i] >> 4) & 0x0F;
            int p8 = (buffer[i] >> 0) & 0x0F;

            gdImageSetPixel(img, x++, y, p5);
            gdImageSetPixel(img, x++, y, p6);
            gdImageSetPixel(img, x++, y, p7);
            gdImageSetPixel(img, x++, y, p8);

            gdImageSetPixel(img, x++, y, p1);
            gdImageSetPixel(img, x++, y, p2);
            gdImageSetPixel(img, x++, y, p3);
            gdImageSetPixel(img, x++, y, p4);

            if (x >= width) {
                x = 0;
                y++;
            }
        }
        all += done;
        r = libusb_bulk_transfer(dso_728705, 0x82, (uint8_t *)buffer, 64, &done, 10);
        if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) {
            gdImageDestroy(img);
            return NULL;
        }
        if (time(NULL) - ts >= 1) {
            gdImageDestroy(img);
            return NULL;
        }
    }
    return img;
}

int dso_728705_open() {
    if (ctx == NULL) {
        if (libusb_init(&ctx) < 0) return -1;
    }
    dso_728705 = libusb_open_device_with_vid_pid(ctx, 0x4348, 0x5537);
    if (!dso_728705) return -1;

    if (libusb_claim_interface(dso_728705, 0) < 0) return -1;

    return 0;
}

void dso_728705_close() {
    if (dso_728705) {
        libusb_close(dso_728705);
    }
}

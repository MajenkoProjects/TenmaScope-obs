#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libusb.h>
#include <unistd.h>
#include <gd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

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
int colors[16];

char *textFont = "/home/matt/.fonts/Hack-Bold.ttf";
int textFontSize = 20;
int text;
int black;

int stringWidth(gdImagePtr img, char *txt) {
    int brect[8];
    gdImageStringFT(img, brect, text, textFont, textFontSize, 0, 9999, 9999, txt);
    return brect[2] - brect[0];
}

int allb(uint32_t *buffer, int l) {
    int i;
    for (i = 0; i < l; i++) {
        if (buffer[i] != 0xbbbbbbbb) return 0;
    }
    return 1;
}

int getFrame(libusb_device_handle *dev, gdImagePtr img) {
    uint32_t buffer[64/4];
    uint8_t data[1] = { 0xE2 };
    int done = 0;
    int all = 0;
    int x = 0;
    int y = 0;
    int r = 0;

    r = libusb_bulk_transfer(dev, 2, data, 1, &done, 1000);
    if (r != 0) return r;

    r = libusb_bulk_transfer(dev, 0x82, (uint8_t *)buffer, 64, &done, 1000);
    if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) return r;

    while (!allb(buffer, 64/4)) {
        r = libusb_bulk_transfer(dev, 0x82, (uint8_t *)buffer, 64, &done, 1000);
        if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) return r;
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

            gdImageSetPixel(img, x++, y, colors[p5]);
            gdImageSetPixel(img, x++, y, colors[p6]);
            gdImageSetPixel(img, x++, y, colors[p7]);
            gdImageSetPixel(img, x++, y, colors[p8]);

            gdImageSetPixel(img, x++, y, colors[p1]);
            gdImageSetPixel(img, x++, y, colors[p2]);
            gdImageSetPixel(img, x++, y, colors[p3]);
            gdImageSetPixel(img, x++, y, colors[p4]);

            if (x >= 400) {
                x = 0;
                y++;
            }
        }
        all += done;
        r = libusb_bulk_transfer(dev, 0x82, (uint8_t *)buffer, 64, &done, 10);
        if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) return r;
        if (time(NULL) - ts >= 1) {
            return LIBUSB_ERROR_TIMEOUT;
        }
    }
}

void setOffline(gdImagePtr img) {
    gdImageFilledRectangle(img, 0, 0, width, height, black);
//    int w = stringWidth(img, "DSO Offline");
//    gdImageStringFT(img, NULL, text, textFont, textFontSize, 0, width/2 - w/2, 120, "DSO Offline");
}

int main(int argc, char **argv)
{

    gdImagePtr img;
	int r;
    uint32_t all = 0;
    libusb_context *ctx;
    int opt;
    int fdwr = 0;
    libusb_device_handle *dev = NULL;

	r = libusb_init(&ctx);
	if (r < 0)
		return r;

    img = gdImageCreateTrueColor(width, height);

    colors[CLR_DISPLAY_BG] = gdImageColorAllocate(img, 0, 0, 0);
    colors[CLR_BTN_LOW] = gdImageColorAllocate(img, 0x2b, 0x2b, 0x2b);
    colors[2] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[CLR_BTN_HI] = gdImageColorAllocate(img, 0x60, 0x60, 0x60);
    colors[4] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[5] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[CLR_BTN_STAT] = gdImageColorAllocate(img, 20, 130, 218);
    colors[7] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[8] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[9] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[10] = gdImageColorAllocate(img, 0, 0, 0); // Not used?
    colors[CLR_MAIN_BORDER] = gdImageColorAllocate(img, 0x40, 0x40, 0x40);
    colors[CLR_CH1] = gdImageColorAllocate(img, 0, 200, 80);
    colors[CLR_CH2] = gdImageColorAllocate(img, 200, 170, 0);
    colors[CLR_MATH] = gdImageColorAllocate(img, 200, 0, 100);
    colors[CLR_TEXT] = gdImageColorAllocate(img, 255, 255, 255);

    black = gdImageColorAllocate(img, 0, 0, 255);
    text = gdImageColorAllocate(img, 255, 0, 0);

    dev = libusb_open_device_with_vid_pid(ctx, 0x4348, 0x5537);
    if (!dev) {
        printf("Unable to open DSO - is it connected?\n");
        exit(10);
    }
    r = libusb_claim_interface(dev, 0);
    if (r != 0) {
        printf("Failed claimimg interface\n");
        exit(10);
    }
    r = getFrame(dev, img);
    if (r == LIBUSB_ERROR_NO_DEVICE) {
        printf("Error getting frame\n");
        libusb_close(dev);
        exit(10);
    }

    char filename[50];
    sprintf(filename, "dso-%lu.png", time(NULL));

    FILE *file = fopen(filename, "w");
    gdImagePng(img, file);
    fclose(file);
    printf("Saved to %s\n", filename);

	libusb_exit(NULL);
	return 0;
}

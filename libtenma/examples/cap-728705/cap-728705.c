#include <stdio.h>
#include <gd.h>
#include <tenma.h>

int main(int argc, char **argv) {

    if (dso_728705_open() < 0) {
        fprintf(stderr, "Unable to open DSO\n");
        return -1;
    }

    gdImagePtr img = dso_728705_getFrame();

    if (!img) {
        fprintf(stderr, "Unable to capture frame\n");
        dso_728705_close();
        return -1;
    }
    
    gdImageColorAllocate(img, 0, 0, 0);
    gdImageColorAllocate(img, 0x2b, 0x2b, 0x2b);
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 0x60, 0x60, 0x60);
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 20, 130, 218);
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 0, 0, 0); // Not used?
    gdImageColorAllocate(img, 0x40, 0x40, 0x40);
    gdImageColorAllocate(img, 0, 200, 80);
    gdImageColorAllocate(img, 200, 170, 0);
    gdImageColorAllocate(img, 200, 0, 100);
    gdImageColorAllocate(img, 255, 255, 255);

    char filename[50];
    sprintf(filename, "dso-%lu.png", time(NULL));

    FILE *file = fopen(filename, "w");
    gdImagePng(img, file);
    fclose(file);
    printf("Saved to %s\n", filename);

    dso_728705_close();
	return 0;
}

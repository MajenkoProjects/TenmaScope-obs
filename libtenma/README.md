# libtenma

This is a collection of routines for interacting with a number of TENMA devices.

This largely the result of reverse engineering of USB protocols. It is not a complete reference
for the interfacing protocols.

## 72-8705 Two-channel DSO

This is a USB connected oscilloscope which allows the capturing of images from the screen. 

* `int dso_728705_open()`

Attempts to open the device. Returns 0 if it succeeds or -1 if it fails.

* `gdImagePtr dso_728705_getFrame()`

Retrieves a frame from the DSO as a GD indexed image. No colours are allocated and must be allocated
by the user after capturingi with `gdImageColorAllocate()`. Returns NULL if the capturing fails.

The image should be destroyed with `gdImageDestroy()` after use.

* `void dso_728705_close()`

Close the device after use.


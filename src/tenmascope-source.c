#include <obs-module.h>
#include <util/threading.h>
#include <util/platform.h>
#include <obs.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <gd.h>
#include <tenma.h>

static int singleton = 0;

struct tenmascope_source {
	obs_source_t *source;
	os_event_t *stop_signal;
	pthread_t thread;
	bool initialized;
	bool online;

	int udpSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_in cameraAddr;

	uint8_t bufferA[1310920]; // Max possible size plus some
	uint8_t bufferB[1310920]; // Max possible size plus some
	uint8_t *buffer;
	uint32_t bufferSize;
};

static const char *wm_get_name(void *unused) {
	UNUSED_PARAMETER(unused);
	return "Tenma Oscilloscope";
}

static void wm_destroy(void *data) {
	struct tenmascope_source *wm = data;

	if (wm) {
		if (wm->initialized) {
			os_event_signal(wm->stop_signal);
		}

		if (wm->thread) {
			pthread_join(wm->thread, NULL);
		}

		if (wm->stop_signal) {
			os_event_destroy(wm->stop_signal);
		}

		if (wm) {
			bfree(wm);
		}
	}

	singleton = 0;
}

static uint32_t palette[] = {
	0x000000,
	0x2b2b2b,
	0x000000,
	0x606060,
	0x000000,
	0x000000,
	0x1482da,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x404040,
	0x00c850,
	0xc8aa50,
	0xc80064,
	0xffffff,
};

static void *video_thread(void *data) {
	struct tenmascope_source *wm = data;
	uint32_t pixels[400 * 240];
	uint64_t cur_time = os_gettime_ns();

	struct obs_source_frame frame = {
		.data = {[0] = (uint8_t *)pixels},
		.linesize = {[0] = 400 * 4},
		.width = 400,
		.height = 240,
		.format = VIDEO_FORMAT_BGRA,
	};

	int ok = dso_728705_open();

	int fail = 0;

	while (os_event_try(wm->stop_signal) == EAGAIN) {
		if (ok == 0) {
			gdImagePtr pic = dso_728705_getFrame();
			if (pic) {
				fail = 0;
				int op = 0;
				for (int y = 0; y < 240; y++) {
					for (int x = 0; x < 400; x++) {
						pixels[op++] = 0xFF000000 | palette[gdImageGetPixel(pic, x, y) & 0xF];
					}
				}
				gdImageDestroy(pic);
				frame.timestamp = cur_time;
				obs_source_output_video(wm->source, &frame);
			} else {
				fail++;
				if (fail > 10) {
					ok = -1;
					dso_728705_close();
				}
			}
		} else {
			bzero(pixels, 400 * 240 * 4);
			frame.timestamp = cur_time;
			obs_source_output_video(wm->source, &frame);
			ok = dso_728705_open();
			fail = 0;
		}
		os_sleepto_ns(cur_time += 30000000);
	}
	if (ok == 0) {
		dso_728705_close();
	}
	return NULL;
}

static void *wm_create(obs_data_t *settings, obs_source_t *source) {

	if (singleton != 0) {
		return NULL;
	}

	singleton = 1;

	struct tenmascope_source *wm = bzalloc(sizeof(struct tenmascope_source));

	wm->source = source;

	wm->online = 0;
	wm->buffer = wm->bufferA;

	if (os_event_init(&wm->stop_signal, OS_EVENT_TYPE_MANUAL) != 0) {
		wm_destroy(wm);
		return NULL;
	}

	if (pthread_create(&wm->thread, NULL, video_thread, wm) != 0) {
		wm_destroy(wm);
		return NULL;
	}

	wm->initialized = true;

	UNUSED_PARAMETER(settings);

	return wm;
}

static uint32_t wm_get_width(void *data) {
	UNUSED_PARAMETER(data);
	return 400;
}

static uint32_t wm_get_height(void *data) {
	UNUSED_PARAMETER(data);
	return 240;
}

struct obs_source_info tenmascope_source = {
	.id				= "tenmascope_source",
	.type			= OBS_SOURCE_TYPE_INPUT,
	.output_flags	= OBS_SOURCE_ASYNC_VIDEO,
	.get_name		= wm_get_name,
	.create			= wm_create,
	.destroy		= wm_destroy,
	.get_width		= wm_get_width,
	.get_height		= wm_get_height,
};

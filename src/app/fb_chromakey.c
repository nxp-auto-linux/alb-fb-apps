/*********************************************************************
* Copyright (C) 2014 by Freescale, Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
 *********************************************************************/

#include "../lib/fb_helper.h"

/*****************************************
 * USAGE:
 * 	./fb_transition
 *
 * ABOUT:
 * Draw a rectangle in /dev/fb0 and /dev/fb1 per pixel transparency activated
 * Perform layer transitions on /dev/fb0 and /dev/fb1
 ************************************************************/
int main(int argc, char** argv)
{
	int i, j, k;
	int ret;

	int dir;
	int dcu_fd;
	int x, y;
	struct IOCTL_LAYER_POS layer_cfg;
	struct IOCTL_LAYER_CHROMA layer_chroma_keys;
	struct IOCTL_LAYER_ALFA_KEY layer_alfa_key;
	struct IOCTL_LAYER_ALFA_VAL layer_alfa_value;
	long int location = 0;

	int fb_resx = 360;
	int fb_resy = 360;
	int fb_bpp = 32;

	/* setup all active FB layers */
	ret = setup_fb_layers(FB_NUM_LAYERS, fb_resx, fb_resy, fb_bpp, 0);
	DIE(ret != 0, "could not setup fb layers");

	/* Draw in /dev/fb0 and /dev/fb1 */
	for(x=40; x<360; x++)
		for(y=40; y<360; y++){
			 location = (x + y * fb_resx) * 4;

			if((y>80) && (y<320) && (x>80) && (x<320))
				*(fb_ptr[0] + location)	= 100;
			else
				*(fb_ptr[0] + location)	= 200;	/* BLUE */

			*(fb_ptr[0] + location + 1) = 200;	/* GREEN */
			*(fb_ptr[0] + location + 2) = 200;	/* RED */
			*(fb_ptr[0] + location + 3) = 255;	/* ALFA */

			*(fb_ptr[1] + location)		= x % 255;
			*(fb_ptr[1] + location + 1) = y % 255;
			*(fb_ptr[1] + location + 2) = (x + y) % 255;
			*(fb_ptr[1] + location + 3) = 255;
	}

	/* Perform layer transitions */
	dcu_fd = open("/dev/dcu0", O_RDONLY);
	DIE(dcu_fd < 0, "could not open /dev/dcu0");

	/* alfa mode set to chroma */
	layer_alfa_key.id = 0;
	layer_alfa_key.key = IOCTL_DCU_ALPHAKEY_WHOLEFRAME;

	if (ioctl(dcu_fd, IOCTL_SET_LAYER_ALPHA_MODE, &layer_alfa_key) == -1)
		printf("DCU0 IOCTL_SET_LAYER_ALPHA_MODE failed: %s\n", strerror(errno));

	/* chroma color key interval */
	layer_chroma_keys.id = 0;
	layer_chroma_keys.state = IOCTL_DCU_CHROMA_ON;
	layer_chroma_keys.min.red = 0;
	layer_chroma_keys.min.green = 0;
	layer_chroma_keys.min.blue = 0;
	layer_chroma_keys.max.red = 255;
	layer_chroma_keys.max.green = 255;
	layer_chroma_keys.max.blue = 100;

	if (ioctl(dcu_fd, IOCTL_SET_LAYER_CHROMA_KEY, &layer_chroma_keys) == -1)
		printf("DCU0 IOCTL_SET_LAYER_CHROMA_KEY failed: %s\n", strerror(errno));

	/* set layer transparency */
	layer_alfa_value.id = 0;
	layer_alfa_value.val = 128;

	if (ioctl(dcu_fd, IOCTL_SET_LAYER_ALPHA_VAL, &layer_alfa_value) == -1)
		printf("DCU0 IOCTL_SET_LAYER_ALPHA_VAL failed: %s\n", strerror(errno));

	dir = 0;
	while(1)
	{
		dir++;

		if((dir % 800) == 0)
		{
			/* alfa mode set to none - chroma on selected pixels */
			layer_alfa_key.id = 0;
			layer_alfa_key.key = IOCTL_DCU_ALPHAKEY_WHOLEFRAME;

			if (ioctl(dcu_fd, IOCTL_SET_LAYER_ALPHA_MODE, &layer_alfa_key) == -1)
				printf("DCU0 IOCTL_SET_LAYER_ALPHA_MODE failed: %s\n", strerror(errno));
		}
		else if((dir % 1601) == 0)
		{
			/* alfa mode set to whole frame - blend all pixels, chroma on selected */
			layer_alfa_key.id = 0;
			layer_alfa_key.key = IOCTL_DCU_ALPHAKEY_OFF;

			if (ioctl(dcu_fd, IOCTL_SET_LAYER_ALPHA_MODE, &layer_alfa_key) == -1)
				printf("DCU0 IOCTL_SET_LAYER_ALPHA_MODE failed: %s\n", strerror(errno));
		}

		/* rotate layer 0 clockwise */
		layer_cfg.id = 0;
		layer_cfg.pos.mX = 200 - ((dir / 50) % 200);
		layer_cfg.pos.mY = 200 - ((2 * dir / 50) % 200);

		if (ioctl(dcu_fd, IOCTL_SET_LAYER_POS, &layer_cfg) == -1)
			printf("DCU0 DCU_IOCTL_SET_LAYER_POS failed: %s\n", strerror(errno));

		usleep(30);
	}

	/* cleanup */
	clean_fb_layers(FB_NUM_LAYERS);

	return 0;
}


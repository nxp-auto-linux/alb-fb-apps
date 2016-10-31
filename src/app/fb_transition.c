/*********************************************************************
* Copyright (C) 2014, 2016 by Freescale, Inc.
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
	long int location = 0;

	int fb_resx = 480;
	int fb_resy = 270;
	int fb_bpp = 32;

	/* setup all active FB layers */
	ret = setup_fb_layers(FB_NUM_LAYERS, fb_resx, fb_resy, fb_bpp, 0);
	DIE(ret != 0, "could not setup fb layers");

	/* Draw in /dev/fb0 and /dev/fb1 */
	for(x=40; x<fb_resx; x++)
		for(y=40; y<fb_resy; y++){
			 location = (x + y * fb_resx) * 4;

			*(fb_ptr[0] + location)	= 200;
			*(fb_ptr[0] + location + 1) = 200;
			*(fb_ptr[0] + location + 2) = 200;
			*(fb_ptr[0] + location + 3) = 140;

			*(fb_ptr[1] + location)	= 200;
			*(fb_ptr[1] + location + 1) = 200;
			*(fb_ptr[1] + location + 2) = 200;
			*(fb_ptr[1] + location + 3) = 140;

			*(fb_ptr[2] + location)	= 200;
			*(fb_ptr[2] + location + 1) = 200;
			*(fb_ptr[2] + location + 2) = 200;
			*(fb_ptr[2] + location + 3) = 140;

			*(fb_ptr[3] + location)	= 200;
			*(fb_ptr[3] + location + 1) = 200;
			*(fb_ptr[3] + location + 2) = 200;
			*(fb_ptr[3] + location + 3) = 140;
	}

	/* Perform layer transitions */
	dcu_fd = open("/dev/dcu0", O_RDONLY);
	DIE(dcu_fd < 0, "could not open /dev/dcu0");

	dir = 0;
	while(1)
	{
		dir++;

		/* move layer 0 */
		layer_cfg.id = 0;
		layer_cfg.pos.mX = 300 - ((dir / 40) % 300);
		layer_cfg.pos.mY = 300 - ((dir / 40 * 2) % 300);

		if (ioctl(dcu_fd, IOCTL_SET_LAYER_POS, &layer_cfg) == -1)
			printf("DCU0 DCU_IOCTL_SET_LAYER_POS failed: %s\n", strerror(errno));

		/* move layer 1 */
		layer_cfg.id = 1;
		layer_cfg.pos.mX = 100 + ((dir / 40 * 2) % 100);
		layer_cfg.pos.mY = 100 + ((dir / 40) % 100);

		if (ioctl(dcu_fd, IOCTL_SET_LAYER_POS, &layer_cfg) == -1)
			printf("DCU0 DCU_IOCTL_SET_LAYER_POS failed: %s\n", strerror(errno));

		/* move layer 2 */
		layer_cfg.id = 2;
		layer_cfg.pos.mX = 200 - ((dir /100 * 3) % 200);
		layer_cfg.pos.mY = 200 + ((dir /100) % 200);

		if (ioctl(dcu_fd, IOCTL_SET_LAYER_POS, &layer_cfg) == -1)
			printf("DCU0 DCU_IOCTL_SET_LAYER_POS failed: %s\n", strerror(errno));

		/* move layer 3 */
		layer_cfg.id = 3;
		layer_cfg.pos.mX = 250 - ((dir /20 * 3) % 200);
		layer_cfg.pos.mY = 250 + ((dir /20) % 200);

		if (ioctl(dcu_fd, IOCTL_SET_LAYER_POS, &layer_cfg) == -1)
			printf("DCU0 DCU_IOCTL_SET_LAYER_POS failed: %s\n", strerror(errno));

		usleep(30);
	}

	/* cleanup */
	clean_fb_layers(FB_NUM_LAYERS);

	return 0;
}


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

#include "fb_helper.h"

const char *fb_name[FB_NUM_LAYERS] = {
	"/dev/fb0",
	"/dev/fb1",
	"/dev/fb2",
	"/dev/fb3",
	"/dev/fb4",
	"/dev/fb5",
	"/dev/fb6",
	"/dev/fb7"
};

const char *dcu_name[DCU_NUM] =
{
	"/dev/dcu0"
};

char *fb_ptr[FB_NUM_LAYERS];
int fb_fd[FB_NUM_LAYERS];
int dcu_fd[DCU_NUM];
int fb_screensize[FB_NUM_LAYERS];

/* crea single buffer per layer and clear all layers */
int setup_fb_layers(int num_fb_active,
		int fb_resx,
		int fb_resy,
		int fb_bpp)
{
	struct IOCTL_LAYER_POS layer_cfg;
	struct IOCTL_LAYER_CHROMA layer_chroma_keys;
	struct IOCTL_LAYER_ALFA_KEY layer_alfa_key;

	int i, j, ret;
	struct fb_var_screeninfo fb_var_info[FB_NUM_LAYERS];
	struct fb_fix_screeninfo fb_fix_info[FB_NUM_LAYERS];

	dcu_fd[0] = open("/dev/dcu0", O_RDWR);
	DIE(dcu_fd[0] < 0, "could not open /dev/dcu0");

	for(i=0; i<num_fb_active; i++){

		/*get file descriptor */
		fb_fd[i] = open(fb_name[i], O_RDWR);
		DIE(fb_fd[i] == (-1), "Could not open /dev/fb device");

		/* Get variable screen information */
		ret = ioctl(fb_fd[i], FBIOGET_VSCREENINFO, &fb_var_info[i]);
		DIE(ret == (-1), "Error reading variable information /dev/fb");

		/* set 24 BPP & set value*/
		fb_var_info[i].xres_virtual = fb_resx;
		fb_var_info[i].yres_virtual = fb_resy;
		fb_var_info[i].xres = fb_resx;
		fb_var_info[i].yres = fb_resy;
		fb_var_info[i].bits_per_pixel = fb_bpp;
		ret = ioctl(fb_fd[i], FBIOPUT_VSCREENINFO, &fb_var_info[i]);
		DIE(ret == (-1), "Error writing fixed information /dev/fb");

		/* Get fixed screen information */
		ret = ioctl(fb_fd[i], FBIOGET_FSCREENINFO, &fb_fix_info[i]);
		DIE(ret == (-1), "Error reading fixed information /dev/fb");

		if((fb_var_info[i].xres != fb_resx) ||
			(fb_var_info[i].yres != fb_resy) ||
			(fb_var_info[i].bits_per_pixel != fb_bpp))
				return 1;

		fb_screensize[i] = fb_var_info[i].xres * fb_var_info[i].yres *
					(fb_var_info[i].bits_per_pixel/8);

		/* map memory */
		fb_ptr[i] = (char *)mmap(0, fb_screensize[i],
			PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd[i], 0);
		DIE(fb_ptr[i] == NULL, "Failed to map /dev/fb to memory");

		/* clear layer contents */
		for(j=0; j<fb_fix_info[i].smem_len; j++)
			*(fb_ptr[i] + j) = 0;

		/* set DCU layer blending, whole frame */
		layer_alfa_key.id = i;
		layer_alfa_key.key = IOCTL_DCU_ALPHAKEY_WHOLEFRAME;
		if (ioctl(dcu_fd[0], IOCTL_SET_LAYER_ALPHA_MODE, &layer_alfa_key) == -1)
			printf("DCU0 IOCTL_SET_LAYER_ALPHA_MODE failed: %s\n", strerror(errno));

		/* disable chroma */
		layer_chroma_keys.id = i;
		layer_chroma_keys.state = IOCTL_DCU_CHROMA_OFF;
		if (ioctl(dcu_fd[0], IOCTL_SET_LAYER_CHROMA_KEY, &layer_chroma_keys) == -1)
			printf("DCU0 IOCTL_SET_LAYER_CHROMA_KEY failed: %s\n", strerror(errno));
	}
	return 0;
}

/* clean */
int clean_fb_layers(int num_fb_active)
{
	int i;

	for(i=0; i<num_fb_active; i++){
		munmap(fb_ptr[i], fb_screensize[i]);
		close(fb_fd[i]);
		close(dcu_fd[0]);
	}
}

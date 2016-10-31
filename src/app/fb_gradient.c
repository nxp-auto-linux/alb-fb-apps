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
 * 	./fb_gradient
 *
 * ABOUT:
 * Draw a rectangle in each layer with transparency activated
 * Demo of per-pixel transparency with 2 layers (/dev/fb0 and /dev/fb1)
 *****************************************/
int main(int argc, char** argv)
{
	int i, j, k;
	int ret;
	int x, y;
	int sq_vx, sq_vy;
	int sq_x, sq_y;
	int sq_alfa;
	long int location = 0;

	int fb_resx = 400;
	int fb_resy = 400;
	int fb_bpp = 32;

	/* setup all active FB layers */
	ret = setup_fb_layers(FB_NUM_LAYERS, fb_resx, fb_resy, fb_bpp, 0);
	DIE(ret != 0, "could not setup fb layers");

	sq_x = 50;
	sq_y = 30;
	sq_vx = 1;
	sq_vy = 1;
	sq_alfa = 16;


	/* write /dev/fb0 with gray color and transparency */
	for(x=50; x<350; x++)
			for(y=50; y<350; y++){
			location = (x + y * fb_resx) * 4;

			*(fb_ptr[0] + location) = 200;
			*(fb_ptr[0] + location + 1) = 200;
			*(fb_ptr[0] + location + 2) = 200;
			*(fb_ptr[0] + location + 3) = 200;
	}

	/* write /dev/fb1 with random gradients */
	do {
		sq_x += sq_vx;
		sq_y += sq_vy;

		if((sq_x > 140) || (sq_x < 4))
			sq_vx *= (-1);
		if((sq_y > 140) || (sq_y < 4))
			sq_vy *= (-1);

		for(x=100; x<400; x++)
			for(y=100; y<400; y++){
				 location = (x + y * fb_resx) * 4;

				*(fb_ptr[1] + location) = (y + sq_x + sq_y) % 255;
				*(fb_ptr[1] + location + 1) = (32 + x + sq_x + sq_y) % 255;
				*(fb_ptr[1] + location + 2) = (64 + x + y + sq_x + sq_y) % 255;
				*(fb_ptr[1] + location + 3) = 200;
			}
	}while(1);
	/* END FB/DCU processing */

	/* cleanup */
	clean_fb_layers(FB_NUM_LAYERS);

	return 0;
}

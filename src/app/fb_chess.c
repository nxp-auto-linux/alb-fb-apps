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
 * MAIN
 * Entry point
 *
 * USAGE:
 * 	./fb_chess
 *
 * ABOUT:
 * Fill the /dev/fb0 layer with black and white squares
 *****************************************/
int main(int argc, char** argv)
{
	int i, j, k;
	int ret;
	int black;
	int offset = 0;

	int fb_resx = FB_RESX;
	int fb_resy = FB_RESY;
	int fb_bpp = 24;

	/* setup all active FB layers */
	ret = setup_fb_layers(FB_NUM_LAYERS, fb_resx, fb_resy, fb_bpp, 0);
	DIE(ret != 0, "could not setup fb layers");

	/* START FB/DCU processing */
	for(i=0; i<fb_resy; i++)
		for(j=0; j<fb_resx; j++){

		black = 0;
		offset = (i * fb_resx + j) * 3;

		if(((i/64) % 2) == 0)
			black++;
		if(((j/64) % 2) == 0)
			black++;
		if(black % 2 == 0){
			*(fb_ptr[0] + offset + 0) = 180;
			*(fb_ptr[0] + offset + 1) = 180;
			*(fb_ptr[0] + offset + 2) = 180;
		}
	}
	/* END FB/DCU processing */

	sleep(10000);

	/* cleanup */
	clean_fb_layers(FB_NUM_LAYERS);

	return 0;
}


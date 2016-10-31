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

/************************************************************
 * FUNCTION: draw_chessboard
 * Draws a 8x8 black&white chessboard using /dev/fb0
 ************************************************************/
void draw_chessboard(int fb_resx, int fb_resy)
{
	int i, j, k;
	int offset = 0;
	int black;
	
	/* DRAW chess board */
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
}

/************************************************************
 * USAGE:
 * 	./fb_transition
 *
 * ABOUT:
 * Draw a rectangle in /dev/fb0 and /dev/fb1 per pixel transparency activated
 * Perform layer transitions on /dev/fb0 and /dev/fb1
 ************************************************************/
int main(int argc, char** argv)
{
	int i, j, k, disp_i;
	int ret;
	int offset = 0;
	int fb_resx = 512;
	int fb_resy = 512;
	int fb_bpp = 24;
	
	int disp_cfg_fd;
	struct IOCTL_DISPLAY_CFG ioctl_display_cfg;
	
	/* perform display change */
	disp_cfg_fd = open("/dev/dcu0", O_RDWR);
	DIE(disp_cfg_fd < 0, "could not open /dev/dcu0");
	
	if((argc != 11) && (argc !=4)){ 
		printf("FB DISPLAY \n");
		printf("Change and test FB/DCU display interface\n\n");
		printf("./fb_display hdmi resx resy\n");
		printf("./fb_display disp_type pixclock resx resy hbp hfp vbp vfp hsync vsync\n\n");
		printf("EXAMPLES:\n");
		printf("./fb_display hdmi 1024 768\n");
		printf("./fb_display hdmi 800 600\n");
		printf("./fb_display hdmi 640 480\n");
		printf("./fb_display lvds 56000000 1920 1080 30 60 30 3 60 3\n");
		printf("./fb_display hdmi 12690000 1024 768 28 1 176 16 96 3\n");
		printf("HDMI monitor display modes\n");
		printf("pixclock resx resy hbp hfp vbp vfp hsync vsync\n\n");
		printf("------------- IOCTL QUERY EDID -------------------\n");
		
		if (ioctl(disp_cfg_fd, IOCTL_PRINT_DISPLAY_INFO, 0) == -1)
			printf("DCU0 IOCTL_GET_DISPLAY_INFO failed: %s\n", strerror(errno));
		
		return 0;
	}
	
	/* setup parameters */
	if(!strcmp(argv[1], "lvds"))
		ioctl_display_cfg.disp_type = IOCTL_DISPLAY_LVDS;
	else
		ioctl_display_cfg.disp_type = IOCTL_DISPLAY_HDMI;
		
	if(argc == 11){
		ioctl_display_cfg.clock_freq = atoi(argv[2]);
		ioctl_display_cfg.hactive = atoi(argv[3]);
		ioctl_display_cfg.vactive = atoi(argv[4]);
		ioctl_display_cfg.hback_porch = atoi(argv[5]);
		ioctl_display_cfg.hfront_porch = atoi(argv[6]);
		ioctl_display_cfg.vback_porch = atoi(argv[7]);
		ioctl_display_cfg.vfront_porch = atoi(argv[8]);
		ioctl_display_cfg.hsync_len = atoi(argv[9]);
		ioctl_display_cfg.vsync_len = atoi(argv[10]);
	}
	else if(argc == 4)
	{
		ioctl_display_cfg.disp_type = IOCTL_DISPLAY_HDMI;
		ioctl_display_cfg.hactive = atoi(argv[2]);
		ioctl_display_cfg.vactive = atoi(argv[3]);
		ioctl_display_cfg.hback_porch = 0;
		ioctl_display_cfg.hfront_porch = 0;
		ioctl_display_cfg.vback_porch = 0;
		ioctl_display_cfg.vfront_porch = 0;
		ioctl_display_cfg.hsync_len = 0;
		ioctl_display_cfg.vsync_len = 0;
	}
	
	/* DCU resolution change */
	printf("Config dcu display - %dx%d\n",
		ioctl_display_cfg.hactive, ioctl_display_cfg.vactive); 
	if (ioctl(disp_cfg_fd, IOCTL_SET_DISPLAY_CFG, &ioctl_display_cfg) == -1)
		printf("DCU0 IOCTL_SET_DISPLAY_CFG failed: %s\n", strerror(errno));
		
	/* setup all active FB layers */
	ret = setup_fb_layers(FB_NUM_LAYERS, fb_resx, fb_resy, fb_bpp, 0);
	DIE(ret != 0, "could not setup fb layers");
	
	/* DRAW chess board */
	draw_chessboard(fb_resx, fb_resy);
	
	/* cleanup */
	close(disp_cfg_fd);

	return 0;
}


/*********************************************************************
* Copyright (C) 2014-2016 by Freescale, Inc.
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

/*
 * Prefix for bmp file format
 */
#define BMP_HEADER_SIZE 54
#define BMP_HEADER_OFFSET_DATA 10
#define BMP_HEADER_OFFSET_HEADER_VERSION 14
#define BMP_HEADER_OFFSET_WIDTH 18
#define BMP_HEADER_OFFSET_HEIGHT 22
#define BMP_HEADER_OFFSET_BPP 28
#define BMP_HEADER_HEADER_VERSION_WIN31 40


struct bmp {
	char header[BMP_HEADER_SIZE];
	void* filemmp;
	char* ifmem;
	int ifd;
	int width;
	int height;
	int bpp;
};

int open_bmp_file(const char* path, struct bmp* handle)
{
	int ret;
	int data_offset;

	memset(handle, 0, sizeof(struct bmp));

	handle->ifd = open(path, O_RDWR);
	DIE(handle->ifd == -1, "Failed to open input file");

	ret = read(handle->ifd, handle->header, sizeof(handle->header));
	DIE(ret < sizeof(handle->header), "Failed to read BMP file header");

	/* check if this bmp version is supported */
	data_offset = *((unsigned long*) (handle->header + BMP_HEADER_OFFSET_HEADER_VERSION));
	DIE(data_offset != BMP_HEADER_HEADER_VERSION_WIN31, "Unsupported BMP version.");

	handle->width = *((unsigned long*) (handle->header + BMP_HEADER_OFFSET_WIDTH));
	handle->height = *((unsigned long*) (handle->header + BMP_HEADER_OFFSET_HEIGHT));
	handle->bpp = *((unsigned short*) (handle->header + BMP_HEADER_OFFSET_BPP));
	data_offset = *((unsigned long*) (handle->header + BMP_HEADER_OFFSET_DATA));

	DIE((handle->width > FB_RESX) || (handle->height > FB_RESY),
			"BMP resolution is larger then supported layer size");

	handle->filemmp = mmap(0, handle->width * handle->height * handle->bpp/8 + data_offset,
		PROT_READ , MAP_SHARED, handle->ifd, 0);
	DIE(handle->filemmp == MAP_FAILED, "Failed to map BMP file into memory");
	handle->ifmem = ((char *)handle->filemmp) + data_offset;

	return 0;
}

int close_bmp_file(struct bmp* handle)
{
	if (handle != NULL && handle->ifd != 0) {
		munmap(handle->ifmem, handle->width * handle->height * handle->bpp/8);
		close(handle->ifd);
	}
	return 0;
}

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
	int x, y, k;
	int ret;
	int black;
	int offset = 0;

	/* input file descriptor */
	struct bmp bmpHandle = {0};

	/*  output file descriptor */
	char* ofmem = NULL;
	int ofd = 0;
	char* output;

	int fb_resx = FB_RESX;
	int fb_resy = FB_RESY;
	int fb_bpp = 32; /* Use ARGB8888 /  24 for RGB888 */

	int tileSize = 1; /* width of the tile in pixels */
	int layerToDraw = 0;

	int argIndx = 1;

	if((argc < 2) || (argc > 4)){
		printf("FB CHESS\n");
		printf("Test FB/DCU display interface by drawing a chess-board/input image on specified layer.\n");
		printf("./fb_chess nlayer bpp [tiled] [if={file}] [of={file}] \n");
		printf("Where:\n\tbpp=24/32 - DCU pixel format\n");
		printf("\ttiled - if specified then DCU layer will be changed to expect image in GPU tiled format\n");
		printf("\tif={file} - path to a bitmap file with bpp format matching DCU format\n");
		printf("EXAMPLES:\n");
		printf("./fb_chess 0 24 if={file}\n");
		printf("./fb_chess 0 32 tiled if={file}\n");
	}

	if(argc > argIndx ) {
		layerToDraw = atoi(argv[argIndx++]);
		if (layerToDraw > FB_NUM_LAYERS || layerToDraw < 0) {
			printf("Supports maximum %d layers", FB_NUM_LAYERS);
			layerToDraw = 0;
		}
	}

	if(argc > argIndx ) {
		int bpp = atoi(argv[argIndx++]);
		if (bpp != 32 && bpp != 24) {
			printf("Supports only 32 / 24 values for bpp, i.e. ARGB8888 / RGB888\n");
		} else {
			fb_bpp = bpp;
		}
	}

	if(argc > argIndx) {
		char* val = argv[argIndx];
		if (strcmp("tiled", val) == 0) {
			tileSize = 4;
			fb_bpp = 32;
			argIndx++;
		}
	}

	if(argc > argIndx) {
		char* val = argv[argIndx];
		if (strncmp(val, "if=", 3) == 0) {
			char* fpath=val+3;
			argIndx++;
			open_bmp_file(fpath, &bmpHandle);
			if (bmpHandle.bpp != 32 && bmpHandle.bpp != 24) {
				DIE(bmpHandle.bpp != 32 && bmpHandle.bpp != 24, "Only 24bpp or 32bpp BMP format is supported");
			}
		}
	}
	if(argc > argIndx) {
		char* val = argv[argIndx];
		if (strncmp(val, "of=", 3) == 0) {
			char* fpath=val+3;
			argIndx++;
			ofd = open(fpath, O_RDWR | O_CREAT);
			DIE(ofd == -1, "Failed to open output file");
			ofmem = malloc(fb_resy*fb_resx*fb_bpp/8);
			DIE(ofmem == NULL, "Failed to allocate output buffer");
		}
	}

	printf("Draw %s on DCU layer %d configured with size %dx%d and %d bpp and tile size %d:%d\n",
			((bmpHandle.ifmem == NULL) ? "chess-board" : "file"),
			layerToDraw, fb_resx, fb_resy, fb_bpp,  tileSize, tileSize);
	if (bmpHandle.ifmem != NULL) {
		printf("Input file configuration:\n\tbpp=%d\n\tx_res=%d\n\ty_res=%d\n",
				bmpHandle.bpp, bmpHandle.width, bmpHandle.height);
	}

	/* setup all active FB layers */
	ret = setup_fb_layers(layerToDraw+1, fb_resx, fb_resy, fb_bpp, tileSize > 1 ? 1 : 0);
	DIE(ret != 0, "could not setup fb layers");

	output = ofmem != NULL ? ofmem : fb_ptr[layerToDraw];

	/* START FB/DCU processing */
	for(y=0; y<fb_resy; y++)
		for(x=0; x<fb_resx; x++){
			int bppOffset = 0;
			int tileW = tileSize;
			int tileH = tileSize;
			int widthInTiles = (fb_resx + tileW - 1) / tileW;
			int tileX = x / tileW;
			int tileY = y / tileH;
			int inTileX = x % tileW;
			int inTileY = y % tileH;

			offset = ((tileY * widthInTiles  + tileX) * (tileW * tileH) + inTileY * tileW + inTileX ) * fb_bpp/8;

			/* pixel chess-draw */
			if (bmpHandle.ifmem == NULL) {
				black = 0;
				if(((x/64) % 2) == 0)
					black++;
				if(((y/64) % 2) == 0)
					black++;
				if(black % 2 == 0){
					*(output + offset + bppOffset++) = 180;
					*(output + offset + bppOffset++) = 180;
					*(output + offset + bppOffset++) = 180;
					if (fb_bpp == 32) {
						/* put the Alpha */
						*(fb_ptr[layerToDraw] + offset + bppOffset++) = 255;
					}
				}
			} else {
				/*
				 Image copy:
				 input file is linear, regardless of output tiled format

				 In case input file has alpha byte and fb is configured without, then just ignore it
				 */
				char file_pixel_alpha=0;

				int ifbppOffset = (y * bmpHandle.width + x) * bmpHandle.bpp/8;

				/* if coordinate outside of input file skip drawing it */
				if (y > bmpHandle.height || x > bmpHandle.width)
					continue;

				if (bmpHandle.bpp == 32) {
					file_pixel_alpha = *(bmpHandle.ifmem + ifbppOffset++);
				}

				if (fb_bpp == 32) {
					*(output + offset + bppOffset++) = file_pixel_alpha;
				}
				*(output + offset + bppOffset++) = *(bmpHandle.ifmem + ifbppOffset++);
				*(output + offset + bppOffset++) = *(bmpHandle.ifmem + ifbppOffset++);
				*(output + offset + bppOffset++) = *(bmpHandle.ifmem + ifbppOffset++);
			}
		}
	/* END FB/DCU processing */


	/* cleanup */
	if (ofmem != NULL) {
		/* just reuse header from input file */
		ret = write(ofd, bmpHandle.header, sizeof(bmpHandle.header));
		ret = write(ofd, output, bmpHandle.height * bmpHandle.width * bmpHandle.bpp/8);
		DIE(ret != bmpHandle.height * bmpHandle.width * bmpHandle.bpp/8, "Failed to write to output buffer");
		close(ofd);
		free(ofmem);
	}

	close_bmp_file(&bmpHandle);

	clean_fb_layers(FB_NUM_LAYERS);

	return 0;
}


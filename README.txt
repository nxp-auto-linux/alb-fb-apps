---------------------------
Intro
---------------------------
These demos represent example framebuffer/DCU applications for the Freescale SAC58R EVB customer platform.

---------------------------
Description of the demos
---------------------------
EXAMPLE 1: fb_chess
Chess black&white board, 8x8 squares, drawn onto /dev/fb0

EXAMPLE 2: fb_gradient
Per pixel transparency and blending with 2 layers (/dev/fb0 and /dev/fb1)

EXAMPLE 3: fb_transition
Layer transitions through DCU IOCTLs (/dev/dcu0) with transparency for 3 layers (/dev/fb0, /dev/fb1 and /dev/fb2)

EXAMPLE 4: fb_chromakey
DCU chroma color keying demo through IOCTLS - one fixed layer (/dev/fb1) and one moving layer (/dev/fb0) with chroma color keying.

EXAMPLE 5: fb_display
Query, configure and test the display output (LVDS/HDMI) through the DCU driver IOCTLs

---------------------------
Compiling
---------------------------
Set the environment variable LINUX_INCLUDE_DIR variable to point towards the linux header files.
The cross compiler environment variable CROSS_COMPILE must also be set. Issue the following:
$(fbdcu_apps_dir)/build$ make clean
$(fbdcu_apps_dir)/build$ make LINUX_INCLUDE_DIR=<path/to/linux-headers/directory>

---------------------------
About FB
---------------------------
Access to the framebuffer is provided through the device file /dev/fbX (where X is a number, e.g. 0,1,2). The application would include headers for definitions of various framebuffer operations (e.g. linux/fb.h) and obtain a descriptor towards the file /dev/fbX. Operations are performed through ioctl calls having as prime argument the previously obtained file descriptor. An example of such an operation is to get or set the screen info (width, height, bits_per_pixel). Finally the framebuffer device allows mapping a region of memory of which contents would be updated on the referred display.

In order to perform operations on a FB device one must first aquire a file descriptor for it (/dev/fbX FB device coresponds to layer X from the DCU). Hence perform the following:
int fb_fd;
fb_fd = open("/dev/fb0", O_RDWR);

Configure a FB device (e.g. /dev/fb0)
To change or aquire certain attributes of a FB device ioctl calls will be used along with the structures fb_var_screeninfo (GET/PUT) and fb_fix_screeninfo (GET). For instance in order to set the bits per pixel to 24 the following should be issued:
ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var_info);
DIE(ret == (-1), "Error reading variable information /dev/fb");
fb_var_info.bits_per_pixel = 24;
ret = ioctl(fb_fd, FBIOPUT_VSCREENINFO, &fb_var_info);
DIE(ret == (-1), "Error writing fixed information /dev/fb");

--------------------------
Display configuration
--------------------------
Due to the different architecture of the DCU/FB, the HDMI driver for SII9022A is limited in functionality. The driver starts by forwarding the DCU signal directly towards the HDMI display with not adjustemnts. Any display changes occur from the DCU hardware and not the HDMI controller. The plug and play feature along with auto-configuration of the display is not supported. Implementing this requires multiple changes due to the nature of the a framebuffer device (/dev/fbX) which represents a layer and not the screen. The HDMI driver can query the EDID from the screen and return the information to the DCU driver. 

In order query, test or configure the display output (LVDS/HDMI) parameters the fb_display application should be used. 
During boot the only information about the display is from the DTS and the bootargs ("hdmi" or no specification). For a dynamic configuration the fb_display application should be used. The application displays a black&white chessboard of 512x512 pixels for validation of the new display configuration.  

The format of the fb_display is as follows:
./fb_display disp_type pixclock resx resy hbp hfp vbp vfp hsync vsync 
where "disp_type" - either "hdmi" or "lvds"
where "pixclock" - pixel clock in Hz
where "resx", "resy" - pixel resolution of the monitor
where "hpf", "hfp", "vbp", "vfp", "hsync", "vsync" - timming settings of the monitor

For the display settings to be queried fb_display should be called without any arguments. Sample output:
------------- IOCTL QUERY EDID -------------------
[   18.894235] hdmi 31746000 640 480 16 1 120 16 64 3
[   18.899256] hdmi 25000000 800 600 23 1 88 40 128 4
[   18.904053] hdmi 20202000 800 600 21 1 160 16 80 3
[   18.909068] hdmi 15384000 1024 768 29 3 160 24 136 6
[   18.914037] hdmi 12690000 1024 768 28 1 176 16 96 3

For settings to be applied fb_display can either be executed with all parameters (type, resolution, timmings) or only using a subset of parameters for HDMI (only resolution). Since there can be multiple variants to the same resolution if setting the resolution doesn't work try selecting a precise configuration.
The following can be done using fb_display:

1. LVDS - to output the display to LVDS all the parameters are required. 
Example: ./fb_display lvds 56000000 1024 768 242 40 21 7 60 10

2. HDMMI - to output the display to HDMI either supply all the parameters or only the resolution.
Tested/working resolutions: 1024x768, 800x600, 720x480, 640x480
Example: ./fb_display hdmi 1024 768
Example: ./fb_display hdmi  800 600
Example: ./fb_display hdmi 37037000 720 480 30 9 60 16 62 6








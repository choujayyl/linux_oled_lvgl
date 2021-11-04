/**
 * @file fbdev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "fbdev.h"
#if USE_FBDEV || USE_BSD_FBDEV

#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#if USE_BSD_FBDEV
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/consio.h>
#include <sys/fbio.h>
#else /* USE_BSD_FBDEV */
#include <linux/fb.h>
#endif /* USE_BSD_FBDEV */

/*********************
 *      DEFINES
 *********************/
#ifndef FBDEV_PATH
#define FBDEV_PATH "/dev/fb0"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

struct bsd_fb_var_info
{
    uint32_t xoffset;
    uint32_t yoffset;
    uint32_t xres;
    uint32_t yres;
    int bits_per_pixel;
};

struct bsd_fb_fix_info
{
    long int line_length;
    long int smem_len;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
#if USE_BSD_FBDEV
static struct bsd_fb_var_info vinfo;
static struct bsd_fb_fix_info finfo;
#else
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
#endif /* USE_BSD_FBDEV */
static char * fbp          = 0;
static long int screensize = 0;
static int fbfd            = 0;

/**********************
 *      MACROS
 **********************/

#if USE_BSD_FBDEV
#define FBIOBLANK FBIO_BLANK
#endif /* USE_BSD_FBDEV */

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void fbdev_init(void)
{
    // Open the file for reading and writing
    fbfd = open(FBDEV_PATH, O_RDWR);
    if(fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        return;
    }
    //    printf("The framebuffer device was opened successfully.\n");
    //
    //    // Make sure that the display is on.
    //    if (ioctl(fbfd, FBIOBLANK, FB_BLANK_UNBLANK) != 0) {
    //        perror("ioctl(FBIOBLANK)");
    //        return;
    //    }

#if USE_BSD_FBDEV
    struct fbtype fb;
    unsigned line_length;

    // Get fb type
    if(ioctl(fbfd, FBIOGTYPE, &fb) != 0) {
        perror("ioctl(FBIOGTYPE)");
        return;
    }

    // Get screen width
    if(ioctl(fbfd, FBIO_GETLINEWIDTH, &line_length) != 0) {
        perror("ioctl(FBIO_GETLINEWIDTH)");
        return;
    }

    vinfo.xres           = (unsigned)fb.fb_width;
    vinfo.yres           = (unsigned)fb.fb_height;
    vinfo.bits_per_pixel = fb.fb_depth;
    vinfo.xoffset        = 0;
    vinfo.yoffset        = 0;
    finfo.line_length    = line_length;
    finfo.smem_len       = finfo.line_length * vinfo.yres;
#else  /* USE_BSD_FBDEV */

//    // Get fixed screen information
//    if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
//        perror("Error reading fixed information");
//        return;
//    }
//
//    // Get variable screen information
//    if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
//        perror("Error reading variable information");
//        return;
//    }
#endif /* USE_BSD_FBDEV */

    //    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    //
    //    // Figure out the size of the screen in bytes
    //    screensize =  finfo.smem_len; //finfo.line_length * vinfo.yres;
    //
    //    // Map the device to memory
    //    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    //    if((intptr_t)fbp == -1) {
    //        perror("Error: failed to map framebuffer device to memory");
    //        return;
    //    }
    //    memset(fbp, 0, screensize);
    //
    //    printf("The framebuffer device was mapped to memory successfully.\n");
}

void fbdev_exit(void)
{
    close(fbfd);
}

/**
 * Flush a buffer to the marked area
 * @param drv pointer to driver where this function belongs
 * @param area an area where to copy `color_p`
 * @param color_p an array of pixel to copy to the `area` part of the screen
 */

unsigned char dis_buf[128 * 4] = {0};
void fbdev_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*Truncate the area to the screen*/
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > 128 - 1 ? 128 - 1 : area->x2;
    int32_t act_y2 = area->y2 > 32 - 1 ? 32 - 1 : area->y2;

    long int location          = 0;
    long int byte_location     = 0;
    unsigned char bit_location = 0;
    int32_t x;
    int32_t y;
    int xx = 0;

    // dis_buf 128*4 bytes
    // w : 128,act_y1:0,act_y2:31,act_x1:0,act_x2:127,sizeof(lv_color_t):1

    int nindex = 0;
    for(y = act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {

            int tmp = x + (y / 8) * 128;
            if(tmp < 128) {
                tmp += 384;
            } else if(tmp < 256) {
                tmp += 128;
            } else if(tmp < 384) {
                tmp -= 128;
            } else {
                tmp -= 384;
            }

            if(lv_color_to1(*color_p) == 0) {
                dis_buf[tmp] &= ~(1 << (7 - (y % 8)));
                // printf("0");
            } else {
                dis_buf[tmp] |= (1 << (7 - (y % 8)));
                // printf("1");
            }
            color_p++;

            //            if(nindex >=0 && nindex < 1020 || nindex == 2047 || (nindex >= 1204 && nindex <= 1204+15 ) ||
            //            (nindex >= 2048 && nindex <= 2048+10 ) || (nindex >= 3072 && nindex <= 3072+5 ))
            //            {
            //                dis_buf[x + (y / 8)*128] |= (1 << (7 - (y % 8)));
            //            }
            //
            //            nindex++;

            // printf("(y*128+x)/8:%d\n",x + (y / 8)*128);
        }
        color_p += area->x2 - act_x2; /*Next row*/
        // printf("\n");
    }

    write(fbfd, dis_buf, 128 * 4);

    // May be some direct update command is required
    // ret = ioctl(state->fd, FBIO_UPDATE, (unsigned long)((uintptr_t)rect));

    lv_disp_flush_ready(drv);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif

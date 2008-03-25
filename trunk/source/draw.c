/*
 * Drawing functions
 * pulled from various works (ie mcbackup, some work by emu_kidid, others maybe)
 */

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <string.h>
#include <ogc/color.h>
#include "freetype.h"

GXRModeObj *rmode = NULL;
void *xfb[2] = { NULL, NULL };
int whichfb = 0;

/****************************************************************************
 * getColor
 *
 * Simply converts RGB to Y1CbY2Cr format
 * I got this from a pastebin, so thanks to whoever originally wrote it!
 *****************************************************************************/
unsigned int GetColor(u8 r1, u8 g1, u8 b1) {
    int y1, cb1, cr1, y2, cb2, cr2, cb, cr;
    u8 r2, g2, b2;

    r2 = r1;
    g2 = g1;
    b2 = b1;

    y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
    cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
    cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;

    y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
    cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
    cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;

    cb = (cb1 + cb2) >> 1;
    cr = (cr1 + cr2) >> 1;

    return ((y1 << 24) | (cb << 16)| (y2 << 8)| cr);
}

/****************************************************************************
 *  Draw functions
 ****************************************************************************/
void DrawHLine (int x1, int x2, int y, int color)
{
    int i;
    y = 320 * y;
    x1 >>= 1;
    x2 >>= 1;
    for (i = x1; i <= x2; i++) {
        int *tmpfb = xfb[whichfb];
        tmpfb[y+i] = color;
    }
}

void DrawVLine (int x, int y1, int y2, int color)
{
    int i;
    x >>= 1;
    for (i = y1; i <= y2; i++) {
        int *tmpfb = xfb[whichfb];
        tmpfb[x + (640 * i) / 2] = color;
    }
}

void DrawBox (int x1, int y1, int x2, int y2, int color)
{
    DrawHLine (x1, x2, y1, color);
    DrawHLine (x1, x2, y2, color);
    DrawVLine (x1, y1, y2, color);
    DrawVLine (x2, y1, y2, color);
}

void DrawBoxFilled (int x1, int y1, int x2, int y2, int color)
{
    int h;
    for (h = y1; h <= y2; h++)
        DrawHLine (x1, x2, h, color);
}


/****************************************************************************
 *  Display functions
 ****************************************************************************/
void SetScreen ()
{
    VIDEO_SetNextFramebuffer((void *)xfb[whichfb]);
    VIDEO_Flush ();
    VIDEO_WaitVSync ();
}


void WaitButtonA ()
{
    while (PAD_ButtonsDown(0) & PAD_BUTTON_A) {};
    while (!(PAD_ButtonsDown(0) & PAD_BUTTON_A));
}

/****************************************************************************
 * Initialise Video
 *
 * Before doing anything in libogc, it's recommended to configure a video
 * output.
 ****************************************************************************/
void Initialize(void) {
    VIDEO_Init (); /*** ALWAYS CALL FIRST IN ANY LIBOGC PROJECT!
                     Not only does it initialise the video
                     subsystem, but also sets up the ogc os
                    ***/

    PAD_Init (); /*** Initialise pads for input ***/

    /*** Try to match the current video display mode
      using the higher resolution interlaced.

      So NTSC/MPAL gives a display area of 640x480
      PAL display area is 640x528
     ***/
    switch (VIDEO_GetCurrentTvMode ()) {
        case VI_NTSC:
            rmode = &TVNtsc480IntDf;
            break;

        case VI_PAL:
            rmode = &TVPal528IntDf;
            break;

        case VI_MPAL:
            rmode = &TVMpal480IntDf;
            break;

        default:
            rmode = &TVNtsc480IntDf;
            break;
    }

    /*** Let libogc configure the mode ***/
    VIDEO_Configure (rmode);

    /*** Now configure the framebuffer.
      Really a framebuffer is just a chunk of memory
      to hold the display line by line.
     ***/

    xfb[0] = (void *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (rmode));
    /*** I prefer also to have a second buffer for double-buffering.
      This is not needed for the console demo.
     ***/
    xfb[1] = (void *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (rmode));

    /*** Define a console ***/
    console_init (xfb[0], 20, 20, rmode->fbWidth, rmode->xfbHeight,
            rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    /*** Clear framebuffer to black ***/
    VIDEO_ClearFrameBuffer (rmode, xfb[0], COLOR_BLACK);
    VIDEO_ClearFrameBuffer (rmode, xfb[1], COLOR_BLACK);

    /*** Set the framebuffer to be displayed at next VBlank ***/
    VIDEO_SetNextFramebuffer (xfb[0]);

    /*** Get the PAD status updated by libogc ***/
    VIDEO_SetPostRetraceCallback(PAD_ScanPads);

    VIDEO_SetBlack(FALSE);

    /*** Update the video for next vblank ***/
    VIDEO_Flush ();

    VIDEO_WaitVSync (); /*** Wait for VBL ***/
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync ();

    /* Initialize Freetype */
    FT_Init();
}


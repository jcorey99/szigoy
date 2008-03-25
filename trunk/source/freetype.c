/****************************************************************************
* libFreeType
*
* Needed to show the user what's the hell's going on !!
* These functions are generic, and do not contain any Memory Card specific
* routines.
****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "freetype.h"
#include "draw.h"

/*** Globals ***/
FT_Library ftlibrary;
FT_Face face;
FT_GlyphSlot slot;
FT_UInt glyph_index;

static int fonthi, fontlo;

/*** The actual TrueType Font ***/
extern char fontface[];		/*** From fontface.s ***/
extern int fontsize;			/*** From fontface.s ***/

#define PAGESIZE 18

/****************************************************************************
* FT_Init
*
* Initialise freetype library
****************************************************************************/
int FT_Init() {
    int err;

    err = FT_Init_FreeType(&ftlibrary);
    if (err)
        return 1;

    err = FT_New_Memory_Face(ftlibrary, (FT_Byte *) fontface,
        fontsize, 0, &face);
    if (err)
        return 1;

    setfontsize (16);
    setfontcolour (0xff, 0xff, 0xff);

    slot = face->glyph;
    return 0;
}

/****************************************************************************
* setfontsize
****************************************************************************/
void setfontsize(int pixelsize) {
    int err;

    err = FT_Set_Pixel_Sizes (face, 0, pixelsize);
    if (err)
        printf ("Error setting pixel sizes!");
}

static void DrawCharacter(FT_Bitmap * bmp, FT_Int x, FT_Int y) {
    FT_Int i, j, p, q;
    FT_Int x_max = x + bmp->width;
    FT_Int y_max = y + bmp->rows;
    int spos;
    unsigned int pixel;
    int c;

    for (i = x, p = 0; i < x_max; i++, p++) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < 0 || j < 0 || i >= 640 || j >= rmode->xfbHeight)
                continue;

            /*** Convert pixel position to GC int sizes ***/
            spos = (j * 320) + (i >> 1);

            unsigned int *ppixel = xfb[whichfb];
            pixel = ppixel[spos];
            c = bmp->buffer[q * bmp->width + p];

            /*** Cool Anti-Aliasing doesn't work too well at hires on GC ***/
            if (c > 128) {
                if (i & 1)
                    pixel = (pixel & 0xffff0000) | fontlo;
                else
                    pixel = ((pixel & 0xffff) | fonthi);

                ppixel[spos] = pixel;
            }
        } // for j
    } // for i
}

/**
 * DrawText
 *
 * Place the font bitmap on the screen
 */
void DrawText(int x, int y, char *text) {
    int px, n;
    int i;
    int err;
    int value, count;

    n = strlen (text);
    if (n == 0)
        return;

    /*** x == -1, auto centre ***/
    if (x == -1) {
        value = 0;
        px = 0;
    } else {
        value = 1;
        px = x;
    }

    for (count = value; count < 2; count++) {
        /*** Draw the string ***/
        for (i = 0; i < n; i++) {
            err = FT_Load_Char (face, text[i], FT_LOAD_RENDER);
            if (err) {
                //printf ("Error %c %d\n", text[i], err);
                continue;  /*** Skip unprintable characters ***/
            }

            if (count)
                DrawCharacter (&slot->bitmap, px + slot->bitmap_left,
                        y - slot->bitmap_top);

            px += slot->advance.x >> 6;
        } // for i

        px = (640 - px) >> 1;

    } // for count
}

/**
 * setfontcolour
 *
 * Uses RGB triple values.
 */
void setfontcolour(u8 r, u8 g, u8 b) {
    u32 fontcolour;

    fontcolour = GetColor(r, g, b);
    fonthi = fontcolour & 0xffff0000;
    fontlo = fontcolour & 0xffff;
}

/**
 * Show an action in progress
 */
void ShowAction(char *msg) {
    int ypos = rmode->xfbHeight >> 1;
    ypos += 16;

    //ClearScreen ();
    whichfb ^= 1;
    VIDEO_ClearFrameBuffer(rmode, xfb[whichfb], COLOR_BLACK);

    DrawText (-1, ypos, msg);
    //ShowScreen ();
    VIDEO_SetNextFramebuffer (xfb[whichfb]);
    VIDEO_Flush ();
    VIDEO_WaitVSync ();
}

/**
 * Show a prompt
 */
void WaitPrompt(char *msg) {
    int ypos = (rmode->xfbHeight) >> 1;

    //ClearScreen ();
    whichfb ^= 1;
    VIDEO_ClearFrameBuffer(rmode, xfb[whichfb], COLOR_BLACK);

    DrawText (-1, ypos, msg);
    ypos += 20;
    DrawText (-1, ypos, "Press A to continue");
    //ShowScreen ();
    VIDEO_SetNextFramebuffer (xfb[whichfb]);
    VIDEO_Flush ();
    VIDEO_WaitVSync ();
    //WaitButtonA ();
    while (PAD_ButtonsDown (0) & PAD_BUTTON_A);
    while (!(PAD_ButtonsDown (0) & PAD_BUTTON_A));
}

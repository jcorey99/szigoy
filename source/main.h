#ifndef MAIN_H
#define MAIN_H

// Power/Reset events
static u8 event = 0;

// Global variables
struct paddle_struct {
    int x, oldx;
    int y, oldy;
    int color;
};

struct ball_struct {
    int x, y;
    int oldx, oldy;
    int vx, vy;
    int color;
    int enabled;
};
/*int ballx, bally;
int ballvx, ballvy;
int ballcolor;*/

/*
 * Reboot back to geckoloader
 */
void (*reload) () = (void (*) ()) 0x80001800;

/*
 * Called when Power button is pressed (libOGC CVS)
 */
static void power_cb () { event = 1; }

/*
 * Called when Reset button is pressed
 */
static void reset_cb () { event = 2; }

void drawpaddles();
void drawball();
void paddlemove(int pad, int x);
void moveball();
void loseball(int ballnum);
void winball(int ballnum);
void drawscore();
#endif

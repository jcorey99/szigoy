#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <math.h>
#include "freetype.h"

#include <ogc/pad.h>
#include <ogc/system.h>

#include "draw.h"
#include "main.h"

#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 15
#define BALL_WIDTH 20
#define BALL_HEIGHT 20

#define BORDER_LEFT 20
#define BORDER_RIGHT rmode->fbWidth-20
#define BORDER_TOP 70
#define BORDER_BOTTOM rmode->xfbHeight-50

#define NUM_PADDLES 2
#define NUM_BALLS 7
#define POINTS_PER_LEVEL 15

struct paddle_struct paddles[NUM_PADDLES];
struct ball_struct balls[NUM_BALLS];

int cpuscore, yourscore, level;

int main(int argc, char **argv) {
    Initialize();

    cpuscore = yourscore = 0;
    level = 1;

    // draw border
    DrawBox(BORDER_LEFT-1, BORDER_TOP-1, BORDER_RIGHT+2, BORDER_BOTTOM+1,
        COLOR_WHITE);

    setfontsize(16);
    setfontcolour(55,255,55);
    DrawText(20, rmode->xfbHeight-25, "Szigoy v0.1");
    DrawText(500, rmode->xfbHeight-25, "by dsbomb");

    setfontsize(32);
    setfontcolour(255, 0, 0);
    DrawText(30, 50, "You");
    setfontcolour(0, 0, 255);
    DrawText(470, 50, "CPU");
    setfontcolour(255,255,255);
    DrawText(220, 50, "Level");

    //SYS_SetPowerCallback (power_cb);
    SYS_SetResetCallback (reset_cb);

    //printf("This is THREE v0.1\n");
    SetScreen();

    u32 button = 0;
    int colors[] = { COLOR_MAROON, COLOR_GREEN, COLOR_OLIVE, COLOR_NAVY, COLOR_PURPLE,
        COLOR_TEAL, COLOR_GRAY, COLOR_SILVER, COLOR_RED, COLOR_LIME, COLOR_YELLOW,
        COLOR_BLUE, COLOR_FUCHSIA, COLOR_AQUA, COLOR_WHITE, COLOR_MONEYGREEN,
        COLOR_SKYBLUE, COLOR_CREAM, COLOR_MEDGRAY };
    int num_colors = 19;

    paddles[0].x = BORDER_LEFT+20;
    paddles[0].y = BORDER_BOTTOM-PADDLE_HEIGHT-5;
    paddles[0].oldx = paddles[0].oldy = 0;
    paddles[0].color = COLOR_NAVY;
    paddles[1].x = rmode->fbWidth / 2;
    paddles[1].y = BORDER_BOTTOM-PADDLE_HEIGHT-5;
    paddles[1].color = COLOR_PURPLE;
    paddles[1].oldx = paddles[1].oldy = 0;

    int i;
    for(i=0; i<NUM_BALLS; i++) {
        //balls[i].x = rmode->fbWidth/2 - BALL_WIDTH/2;
        balls[i].x = (rand() % (BORDER_RIGHT-BORDER_LEFT)) + BORDER_LEFT;
        //balls[i].y = i * 40 + 10;
        balls[i].y = (rand() % (BORDER_BOTTOM-BORDER_TOP-PADDLE_HEIGHT*2)) + BORDER_TOP;
        balls[i].color = colors[i % num_colors];
        balls[i].vx = balls[i].vy = 2;
        // change vx for every other ball
        if (i % 2)
            balls[i].vx *= -1;
        balls[i].enabled = 1;
    }

    drawball();
    drawpaddles();
    drawscore();
    for(i=0; i<50; i++)
        VIDEO_WaitVSync();

    while(1) {
        moveball();
        drawball();
        drawpaddles();
        if (event == 1) {
            printf("POWER!\n");
            reload();
        } else if (event == 2) {
            printf("RESET!\n");
            reload();
        }

        //WaitButtonA();
        PAD_ScanPads();
        button = PAD_ButtonsDown(0);
        if (button & PAD_BUTTON_START)
            reload();
        /*if (button & PAD_BUTTON_A) { // increase x
            for(i=0; i<NUM_BALLS; i++) {
                if (balls[i].vx > 0) balls[i].vx++;
                if (balls[i].vx < 0) balls[i].vx--;
            }
        }
        if (button & PAD_BUTTON_B) { // increase y
            for(i=0; i<NUM_BALLS; i++) {
                if (balls[i].vy > 0) balls[i].vy++;
                if (balls[i].vy < 0) balls[i].vy--;
            }
        }*/
        if (PAD_TriggerR(0) > 0) {
            for(i=0;i<10;i++)
                VIDEO_WaitVSync();
        }
        if (PAD_TriggerR(0) > 149) {
            while(PAD_TriggerR(0) > 149);
        }
        int stickx = PAD_StickX(0);
        if ( (stickx > 10) || (stickx < 10) ) {
            paddlemove(0, stickx / 5);
        }
        int substickx = PAD_SubStickX(0);
        if ( (substickx > 10) || (substickx < 10) ) {
            paddlemove(1, substickx / 5);
        }

        SetScreen();
    } // while 1
    return 0;
}

void drawpaddles() {
    int i;
    for(i=0; i<NUM_PADDLES; i++) {
        // Erase old
        DrawBoxFilled(paddles[i].oldx, paddles[i].oldy,
            paddles[i].oldx+PADDLE_WIDTH, paddles[i].oldy+PADDLE_HEIGHT, COLOR_BLACK);
        paddles[i].oldx = paddles[i].x;
        paddles[i].oldy = paddles[i].y;
        //x1 = paddle1x;
        //y1 = paddle1y;
        // Draw new
        DrawBoxFilled(paddles[i].x, paddles[i].y, paddles[i].x+PADDLE_WIDTH, paddles[i].y+PADDLE_HEIGHT, paddles[i].color);
    }
        //whichfb ^= 1;
}

void drawball() {
    int i;
    for(i=0; i<NUM_BALLS; i++) {
        if (balls[i].enabled) {
            // Erase old
            DrawBoxFilled(balls[i].oldx, balls[i].oldy, balls[i].oldx+BALL_WIDTH, balls[i].oldy+BALL_HEIGHT, COLOR_BLACK);
            balls[i].oldx = balls[i].x;
            balls[i].oldy = balls[i].y;
            // Draw new
            DrawBoxFilled(balls[i].x, balls[i].y, balls[i].x+BALL_WIDTH, balls[i].y+BALL_HEIGHT, balls[i].color);
        }
    }
}

void paddlemove(int pad, int x) {
    paddles[pad].x += x;
    if (paddles[pad].x < BORDER_LEFT)
        paddles[pad].x = BORDER_LEFT;
    if (paddles[pad].x > BORDER_RIGHT-PADDLE_WIDTH)
        paddles[pad].x = BORDER_RIGHT-PADDLE_WIDTH;
    if (pad == 0) {
        if (paddles[0].x+PADDLE_WIDTH > paddles[1].x)
            paddles[0].x = paddles[1].x-PADDLE_WIDTH;
    } else if (pad == 1) {
        if (paddles[1].x < paddles[0].x+PADDLE_WIDTH)
            paddles[1].x = paddles[0].x+PADDLE_WIDTH;
    }
    //drawpaddles();
}

void moveball() {
    int i;
    for(i=0; i<NUM_BALLS; i++) {
        balls[i].x += balls[i].vx;
        balls[i].y += balls[i].vy;

        if (balls[i].x > BORDER_RIGHT - BALL_WIDTH) {
            balls[i].x = BORDER_RIGHT - BALL_WIDTH;
            balls[i].vx *= -1;
        }
        // won't be needed later, I suppose
        if (balls[i].y > BORDER_BOTTOM - BALL_HEIGHT) {
            /*balls[i].y = BORDER_BOTTOM - BALL_HEIGHT;
            balls[i].vy *= -1;*/
            loseball(i);
        }
        if (balls[i].y < BORDER_TOP) {
            balls[i].y = BORDER_TOP;
            balls[i].vy *= -1;
        }
        if (balls[i].x < BORDER_LEFT) {
            balls[i].x = BORDER_LEFT;
            balls[i].vx *= -1;
        }

        // Detect other balls
        int k;
        for(k=0; k<NUM_BALLS; k++) {
            if (k != i) {
                double cx1, cx2, cy1, cy2;
                cx1 = balls[i].x + BALL_WIDTH / 2;
                cy1 = balls[i].y + BALL_HEIGHT / 2;
                cx2 = balls[k].x + BALL_WIDTH / 2;
                cy2 = balls[k].y + BALL_HEIGHT / 2;
                double d = sqrt( pow(cx2 - cx1, 2) + pow(cy2 - cy1, 2) );
                if (d < BALL_WIDTH) {
                    balls[i].vx *= -1;
                    balls[i].vy *= -1;
                    balls[k].vx *= -1;
                    balls[k].vy *= -1;
                }
            }
        }
        // Detect paddles
        int j;
        for(j=0; j<NUM_PADDLES; j++) {
            if (balls[i].y+BALL_HEIGHT > paddles[j].y) {
            // at or below paddle
                if ( ( (balls[i].x >= paddles[j].x) && (balls[i].x+BALL_WIDTH <= paddles[j].x+PADDLE_WIDTH) ) || // totally within paddle
                    ( (balls[i].x+BALL_WIDTH >= paddles[j].x) && (balls[i].x <= paddles[j].x+PADDLE_WIDTH) ) ) { // hanging off either side
                    // if ball center y is below top of paddle, lose it
                    if (balls[i].y+BALL_HEIGHT/2 <= paddles[j].y) {
                        balls[i].vy *= -1;
                        balls[i].y = paddles[j].y - BALL_HEIGHT; // reset position to above paddle
                        winball(i);
                    }
                }
            }
        } // for j
    }
}

void loseball(int ballnum) {
    if (balls[ballnum].enabled) {
        cpuscore++;
        //balls[ballnum].enabled = 0;
        balls[ballnum].vy *= -1;
        balls[ballnum].y -= 5;
    }

    drawscore();
}

void winball(int ballnum) {
    yourscore++;
    drawscore();
}

void drawscore() {
    char str[1024];

    sprintf(str, "%d", yourscore);
    DrawBoxFilled(110, 20, 200, 50, COLOR_BLACK);
    DrawText(110, 50, str);

    sprintf(str, "%d", cpuscore);
    DrawBoxFilled(550, 20, 635, 50, COLOR_BLACK);
    DrawText(550, 50, str);

    // when cpu gets enough points, speed things up
    if ( (level < 10) && (cpuscore > level*POINTS_PER_LEVEL) ) {
        level++;
        int i;
        for(i=0; i<NUM_BALLS; i++) {
            // speed up the balls
            if (balls[i].vx < 0)
                balls[i].vx--;
            else balls[i].vx++;
            if (balls[i].vy < 0)
                balls[i].vy--;
            else balls[i].vy++;
        }
    }

    sprintf(str, "%d", level);
    DrawBoxFilled(325, 20, 400, 50, COLOR_BLACK);
    DrawText(325, 50, str);
}

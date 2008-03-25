extern int whichfb;
extern void *xfb[2];
extern GXRModeObj *rmode;

/****************************************************************************
 * getColor
 * Simply converts RGB to Y1CbY2Cr format
 *****************************************************************************/
unsigned int GetColor(u8 r1, u8 g1, u8 b1);

/****************************************************************************
 *  Draw functions - lines, boxes
 ****************************************************************************/
void DrawHLine (int x1, int x2, int y, int color);
void DrawVLine (int x, int y1, int y2, int color);
void DrawBox (int x1, int y1, int x2, int y2, int color);
void DrawBoxFilled (int x1, int y1, int x2, int y2, int color);

/****************************************************************************
 *  Display functions
 ****************************************************************************/
void Initialize();
void SetScreen();
void WaitButtonA();

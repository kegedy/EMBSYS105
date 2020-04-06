/************************************************************************************

Copyright (c) 2001-2016  University of Washington Extension.

Module Name:

    tasks.c

Module Description:

    The tasks that are executed by the test application.

2016/2 Nick Strathy adapted it for NUCLEO-F401RE 

************************************************************************************/
#include <stdarg.h>
#include <string.h>
#include "bsp.h"
#include "print.h"
#include "mp3Util.h"
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#define PENRADIUS 3   // Parameter used for drawing on screen

Adafruit_ILI9341 lcdCtrl = Adafruit_ILI9341(); // LCD controller
Adafruit_FT6206 touchCtrl = Adafruit_FT6206(); // Touch controller

long MapTouchToScreen(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// MUSIC FILES -> add more to this
#include "train_crossing.h"
#include "SweetCaroline.h"
#include "DontStopBelieving.h"

// Song Group
struct SongGroup{
    char *title;
    INT32U size;
    INT32U pos;
    INT8U *pStart;
    INT8U *pStream;
};
struct SongGroup group0;
struct SongGroup group1;  
struct SongGroup group2;

// Linked List
struct Node { 
    SongGroup data; 
    Node* next; 
};
struct Node *curNode;
struct Node node0;
struct Node node1;
struct Node node2;

// Definition for PrintWithBuf
#define BUFSIZE 256

typedef enum {
    INPUTCOMMAND_NONE,
    INPUTCOMMAND_NEXTSONG,
    INPUTCOMMAND_PLAY,
    INPUTCOMMAND_PREVSONG,
    SONG_COMPLETE,
} CommandEnum;


// INPUT BUTTONS
Adafruit_GFX_Button PREV;
Adafruit_GFX_Button PLAY;
Adafruit_GFX_Button NEXT;


// DEFINE LOCATIONS FOR INPUTS
typedef struct{
    int16_t X, Y;
} Point;

typedef struct{
    Point Title;        // Song Title
    Point Progress;     // Song Progress
    Point TL, TM, TR;   // Button Layout
} Grid;
static Grid grid;

/************************************************************************************

   Allocate the stacks for each task.
   The maximum number of tasks the application can have is defined by OS_MAX_TASKS in os_cfg.h

************************************************************************************/
static OS_STK   CommandTaskStk[APP_CFG_TASK_START_STK_SIZE];
static OS_STK   DisplayTaskStk[APP_CFG_TASK_START_STK_SIZE];
static OS_STK   Mp3TaskStk[APP_CFG_TASK_START_STK_SIZE];
static OS_STK   ProgressTaskStk[APP_CFG_TASK_START_STK_SIZE];
static OS_STK   TouchInputTaskStk[APP_CFG_TASK_START_STK_SIZE];

//OS_EVENT * semStream;
OS_EVENT * mboxCommand;
OS_EVENT * mboxDisplay;
OS_EVENT * mboxProgress;
OS_FLAG_GRP *rxFlags = 0;

// Task prototypes
void CommandTask(void* pdata);
void DisplayTask(void* pdata);
void ProgressTask(void* pdata);
void Mp3Task(void* pdata);
void TouchInputTask(void* pdata);

// Useful functions
void DrawTitle(char *title);
void DrawProgress(int8_t percentage);
void ResetProgress(void);
void TogglePlayBtn(void);
void DrawNextBtn(void);
void DrawPrevBtn(void);
void DefineBtns(Adafruit_GFX_Button *buttonCtrl, int16_t x, int16_t y, CommandEnum CMD);
void PrintToLcdWithBuf(char *buf, int size, char *format, ...);

// Globals
// BOOLEAN NEXTSONG = OS_FALSE;
BOOLEAN STREAMING = OS_FALSE;
BOOLEAN LAST_STATE = STREAMING;

/************************************************************************************

   This task is the initial task running, started by main(). It starts
   the system tick timer and creates all the other tasks. Then it deletes itself.

************************************************************************************/
void StartupTask(void* pdata)
{
    INT8U err;
    char buf[BUFSIZE];
    PjdfErrCode pjdfErr;
    INT32U length;
    static HANDLE hSD = 0;
    static HANDLE hSPI = 0;

    PrintWithBuf(buf, BUFSIZE, "StartupTask: Begin\n");
    PrintWithBuf(buf, BUFSIZE, "StartupTask: Starting timer tick\n");
    
    //Songs
    group0.title = "Train Crossing";
    group0.size = sizeof(Train_Crossing);
    group0.pos = 0;
    group0.pStart = (INT8U*)Train_Crossing;
    group0.pStream = group0.pStart;
    node0.data = group0;
    node0.next = NULL;
    
    group1.title = "Sweet Caroline";
    group1.size = sizeof(SweetCaroline);
    group1.pos = 0;
    group1.pStart = (INT8U*)SweetCaroline;
    group1.pStream = group1.pStart;
    node1.data = group1;
    node1.next = NULL;
    
    group2.title = "Don't Stop Believing";
    group2.size = sizeof(DontStopBelieving);
    group2.pos = 0;
    group2.pStart = (INT8U*)DontStopBelieving;
    group2.pStream = group2.pStart;
    node2.data = group2;
    node2.next = NULL;
    
    node0.next = &node1;
    node1.next = &node2;
    node2.next = &node0;
    curNode = &node0;

    // Start the system tick
    OS_CPU_SysTickInit(OS_TICKS_PER_SEC);
    
    // Create Semaphores, Mailboxes, rxFlags
    //semStream = OSSemCreate(1);
    mboxCommand = OSMboxCreate((void*)NULL);
    mboxProgress = OSMboxCreate((void*)NULL);
    mboxDisplay = OSMboxCreate((void*)NULL);
    rxFlags = OSFlagCreate((OS_FLAGS)0, &err);
    
    // Initialize SD card
    PrintWithBuf(buf, PRINTBUFMAX, "Opening handle to SD driver: %s\n", PJDF_DEVICE_ID_SD_ADAFRUIT);
    hSD = Open(PJDF_DEVICE_ID_SD_ADAFRUIT, 0);
    if (!PJDF_IS_VALID_HANDLE(hSD)) while(1);

    // We talk to the SD controller over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to the SD driver.
    PrintWithBuf(buf, PRINTBUFMAX, "Opening SD SPI driver: %s\n", SD_SPI_DEVICE_ID);
    hSPI = Open(SD_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);
    
    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hSD, PJDF_CTRL_SD_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);

    // Create the test tasks
    PrintWithBuf(buf, BUFSIZE, "StartupTask: Creating the application tasks\n");

    // The maximum number of tasks the application can have is defined by OS_MAX_TASKS in os_cfg.h
    OSTaskCreate(DisplayTask, (void*)0, &DisplayTaskStk[APP_CFG_TASK_START_STK_SIZE-1], 9);
    OSTaskCreate(ProgressTask, (void*)0, &ProgressTaskStk[APP_CFG_TASK_START_STK_SIZE-1], 8);
    OSTaskCreate(Mp3Task, (void*)0, &Mp3TaskStk[APP_CFG_TASK_START_STK_SIZE-1], 7);
    OSTaskCreate(TouchInputTask, (void*)0, &TouchInputTaskStk[APP_CFG_TASK_START_STK_SIZE-1], 6);
    OSTaskCreate(CommandTask, (void*)0, &CommandTaskStk[APP_CFG_TASK_START_STK_SIZE-1], 5);

    // Delete ourselves, letting the work be done in the new tasks.
    PrintWithBuf(buf, BUFSIZE, "StartupTask: deleting self\n");
    OSTaskDel(OS_PRIO_SELF);
}

/************************************************************************************

   DISPLAY HELPER FUNCTIONS

************************************************************************************/
static int32_t len(char* s) { 
    int32_t length = 0; 
    while (*s != '\0') { 
        length++; 
        s++; 
    } 
    return length; 
} 
void DrawTitle(char *title) {    
    // Define Characteristics
    char buf[BUFSIZE];
    int16_t x = grid.Title.X; 
    int16_t y = grid.Title.Y;
    lcdCtrl.setTextColor(ILI9341_WHITE);
    lcdCtrl.setTextSize(2);
    
    // Clear screen
    lcdCtrl.fillRect(0, y-10, 240, 30, 0x0000);
       
    // Title
    int16_t cursorX = x-(len(title)*12)/2;
    if (cursorX<0) cursorX = 1;
    lcdCtrl.setCursor(cursorX, y);
    PrintToLcdWithBuf(buf, BUFSIZE, title); 
}
void TogglePlayBtn() {
    int16_t x = grid.TM.X; 
    int16_t y = grid.TM.Y;
    if (STREAMING) {
        lcdCtrl.fillCircle(x, y, 23, 0xFFFF);
        lcdCtrl.fillRect(x-6, y-8, 4, 16, 0x0000);
        lcdCtrl.fillRect(x+2, y-8, 4, 16, 0x0000);
    }
    else {
        lcdCtrl.fillCircle(x, y, 23, 0xFFFF);
        lcdCtrl.fillTriangle(x-4, y+8,
                            x-4, y-8,
                            x+6, y, 0x0000);        
    }
}
void DrawNextBtn() {
    int16_t x = grid.TR.X; 
    int16_t y = grid.TR.Y;
    uint16_t color = 0xFFFF;
    int8_t offsetX = -15;
    lcdCtrl.fillTriangle((x-5)+offsetX, y+8, 
                        (x-5)+offsetX, y-8,
                        (x+5)+offsetX, y, color);
    lcdCtrl.fillRect(x+6+offsetX, y-8, 3, 16, color);
}
void DrawPrevBtn() {
    int16_t x = grid.TL.X; 
    int16_t y = grid.TL.Y;
    uint16_t color = 0xFFFF;
    int8_t offsetX = 15;
    lcdCtrl.fillTriangle((x+5)+offsetX, y+8,
                        (x+5)+offsetX, y-8,
                        (x-5)+offsetX, y, color);
    lcdCtrl.fillRect(x-8+offsetX, y-8, 3, 16, color);
}
void DefineBtns(Adafruit_GFX_Button *buttonCtrl, CommandEnum CMD) {
    // Creates button on LCD screen and returns Adafruit_GFX_Button control
    //  void initButton(Adafruit_GFX *gfx, int16_t x, int16_t y, 
    //		      uint8_t w, uint8_t h, 
    //		      uint16_t outline, uint16_t fill, uint16_t textcolor,
    //		      char *label, uint8_t textsize);
    int16_t x = 0;
    int16_t y = 0;
    if (CMD==INPUTCOMMAND_PREVSONG)      {x = grid.TL.X; y = grid.TL.Y;}
    else if (CMD==INPUTCOMMAND_PLAY)     {x = grid.TM.X; y = grid.TM.Y;}
    else if (CMD==INPUTCOMMAND_NEXTSONG) {x = grid.TR.X; y = grid.TR.Y;}
    
    // Define Characteristics and Boundary
    uint8_t W = 60;
    uint8_t H = 40;
    uint16_t OUTLINE = 0x0000;
    uint16_t FILL = 0x0000;
    uint16_t TEXTCOLOR = 0xFFFF;
    char * LABEL = "";
    uint8_t TEXT_SIZE = 1;
    buttonCtrl->initButton(&lcdCtrl,x,y,W,H,OUTLINE,FILL,TEXTCOLOR,LABEL,TEXT_SIZE);
    buttonCtrl->drawButton();
    
    // Button on LCD is bigger than what is displayed
    if (CMD==INPUTCOMMAND_PREVSONG)      DrawPrevBtn();
    else if (CMD==INPUTCOMMAND_PLAY)     TogglePlayBtn();
    else if (CMD==INPUTCOMMAND_NEXTSONG) DrawNextBtn();
}
void DrawProgress(int8_t percentage) {
    int16_t x = grid.Progress.X; 
    int16_t y = grid.Progress.Y;
    int16_t progW = 200;
    int16_t progH = 6;
    
    lcdCtrl.drawRect(x-progW/2, y, progW, progH, 0xFFFF);
    lcdCtrl.fillRect(x-progW/2, y, percentage*2, progH-1, 0xFFFF);
}
void ResetProgress() {
    int16_t x = grid.Progress.X; 
    int16_t y = grid.Progress.Y;
    int16_t progW = 200;
    int16_t progH = 6;

    lcdCtrl.fillRect(x-progW/2, y, progW, progH, 0x0000);
    lcdCtrl.drawRect(x-progW/2, y, progW, progH, 0xFFFF);
}
static void DrawLcdContents()
{
    // Init
    lcdCtrl.fillScreen(ILI9341_BLACK);
    
    // Define Grid; (X,Y) is middle of container
    grid.Title.X    = 120; grid.Title.Y    =  30;
    grid.Progress.X = 120; grid.Progress.Y =  80;
    grid.TL.X       =  50; grid.TL.Y       = 120;
    grid.TM.X       = 120; grid.TM.Y       = 120;
    grid.TR.X       = 190; grid.TR.Y       = 120;

    // Song Title; TASK: Read song from SD 
    DrawTitle(curNode->data.title);
    
    // Progress Bar; TASK: Update as song plays
    DrawProgress(1);
    
    // Buttons
    PREV = Adafruit_GFX_Button();
    PLAY = Adafruit_GFX_Button();
    NEXT = Adafruit_GFX_Button();
    DefineBtns(&PREV,INPUTCOMMAND_PREVSONG);
    DefineBtns(&PLAY,INPUTCOMMAND_PLAY);
    DefineBtns(&NEXT,INPUTCOMMAND_NEXTSONG);
}

/************************************************************************************

   PROGRESS

************************************************************************************/
void ProgressTask(void* pdata) {
    INT8U err;
    INT8U *msgReceived;
    while(1) {
        msgReceived = (INT8U*)OSMboxPend(mboxProgress, 0, &err);
        DrawProgress(*msgReceived);
    }
}
/************************************************************************************

   DISPLAY

************************************************************************************/
void DisplayTask(void* pdata)
{
    INT8U err;
    char buf[BUFSIZE];
    CommandEnum *msgReceived;
    PjdfErrCode pjdfErr;
    INT32U length;
    PrintWithBuf(buf, BUFSIZE, "DisplayTask: starting\n");
    
    // Open handle to the LCD driver
    PrintWithBuf(buf, BUFSIZE, "Opening LCD driver: %s\n", PJDF_DEVICE_ID_LCD_ILI9341);
    HANDLE hLcd = Open(PJDF_DEVICE_ID_LCD_ILI9341, 0);
    if (!PJDF_IS_VALID_HANDLE(hLcd)) while(1);
    
    // We talk to the LCD controller over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to the LCD driver.
    PrintWithBuf(buf, BUFSIZE, "Opening LCD SPI driver: %s\n", LCD_SPI_DEVICE_ID);
    HANDLE hSPI = Open(LCD_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);

    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hLcd, PJDF_CTRL_LCD_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);

    PrintWithBuf(buf, BUFSIZE, "Initializing LCD controller\n");
    lcdCtrl.setPjdfHandle(hLcd);
    lcdCtrl.begin();

    // DISPLAY
    DrawLcdContents();

    // Update Display
    while (1) { 
        msgReceived = (CommandEnum*)OSMboxPend(mboxDisplay, 0, &err);
        if (*msgReceived == INPUTCOMMAND_PREVSONG) {
            lcdCtrl.fillCircle(grid.TL.X+15, grid.TL.Y, 15, 0x0000);
            OSTimeDly(150);
            DrawPrevBtn();
            ResetProgress();
            DrawTitle(curNode->data.title);
            OSFlagPost(rxFlags, 0x1, OS_FLAG_CLR, &err);
        }
        if (*msgReceived == INPUTCOMMAND_PLAY) {
            lcdCtrl.fillCircle(grid.TM.X, grid.TM.Y, 23, 0x0000);
            OSTimeDly(100);
            TogglePlayBtn();
            OSFlagPost(rxFlags, 0x1, OS_FLAG_CLR, &err);
        }
        if (*msgReceived == INPUTCOMMAND_NEXTSONG) {
            lcdCtrl.fillCircle(grid.TR.X-15, grid.TR.Y, 15, 0x0000);
            OSTimeDly(150);
            DrawNextBtn();
            ResetProgress();
            DrawTitle(curNode->data.title);
            OSFlagPost(rxFlags, 0x1, OS_FLAG_CLR, &err);
        }
        if (*msgReceived == SONG_COMPLETE) {
            ResetProgress();
            DrawTitle(curNode->data.title);
            OSFlagPost(rxFlags, 0x1, OS_FLAG_CLR, &err);
        }
    }
}
/************************************************************************************

   TOUCH

************************************************************************************/
void TouchInputTask(void* pdata)
{
    CommandEnum INPUT;
    INT8U err;
    char buf[BUFSIZE];
    PrintWithBuf(buf, BUFSIZE, "Initializing FT6206 touchscreen controller\n");
    if (! touchCtrl.begin(40)) {  // pass in 'sensitivity' coefficient
        PrintWithBuf(buf, BUFSIZE, "Couldn't start FT6206 touchscreen controller\n");
        while (1);
    }
    while (1) {
        // Wait for DisplayTask to finish
        OSFlagPend(rxFlags, 0x1, OS_FLAG_WAIT_CLR_ALL, 0, &err);
        boolean touched = false;
        
        // Poll for a touch on the touch panel
        touched = touchCtrl.touched(); 
        if (! touched) {
            PREV.press(false);
            PLAY.press(false);
            NEXT.press(false);
            OSTimeDly(5);
            continue;
        }
        
        // Retrieve a point
        TS_Point rawPoint;
        rawPoint = touchCtrl.getPoint();
      
        // transform touch orientation to screen orientation.
        TS_Point p = TS_Point();
        p.x = MapTouchToScreen(rawPoint.x, 0, ILI9341_TFTWIDTH, ILI9341_TFTWIDTH, 0);
        p.y = MapTouchToScreen(rawPoint.y, 0, ILI9341_TFTHEIGHT, ILI9341_TFTHEIGHT, 0);
        
        PREV.contains(p.x,p.y) ? PREV.press(true) : PREV.press(false);
        PLAY.contains(p.x,p.y) ? PLAY.press(true) : PLAY.press(false);
        NEXT.contains(p.x,p.y) ? NEXT.press(true) : NEXT.press(false);
        
        INPUT = INPUTCOMMAND_NONE;
        if (PREV.justPressed()) {
            INPUT = INPUTCOMMAND_PREVSONG;
        }
        if (PLAY.justPressed()) {
            INPUT = INPUTCOMMAND_PLAY;
        }
        if (NEXT.justPressed())  {
            INPUT = INPUTCOMMAND_NEXTSONG;
        }
        OSMboxPost(mboxCommand, (void *)&INPUT);
    }
}

/************************************************************************************

   MP3

************************************************************************************/
void Mp3Task(void* pdata)
{
    INT8U err;
    char buf[BUFSIZE];
    CommandEnum INPUT;
    PjdfErrCode pjdfErr;
    INT32U length;

    //OSTimeDly(2000); // Allow other task to initialize LCD before we use it.
    PrintWithBuf(buf, BUFSIZE, "Mp3Task: starting\n");
    
    // Open handle to the MP3 decoder driver
    PrintWithBuf(buf, BUFSIZE, "Opening MP3 driver: %s\n", PJDF_DEVICE_ID_MP3_VS1053);
    HANDLE hMp3 = Open(PJDF_DEVICE_ID_MP3_VS1053, 0);
    if (!PJDF_IS_VALID_HANDLE(hMp3)) while(1);
    
    // We talk to the MP3 decoder over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to the MP3 driver.
    PrintWithBuf(buf, BUFSIZE, "Opening MP3 SPI driver: %s\n", MP3_SPI_DEVICE_ID);
    HANDLE hSPI = Open(MP3_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);

    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hMp3, PJDF_CTRL_MP3_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);

    // Send initialization data to the MP3 decoder and run a test
    PrintWithBuf(buf, BUFSIZE, "Starting MP3 device test\n");
    Mp3Init(hMp3);
    OSFlagPost(rxFlags, 0x2, OS_FLAG_SET, &err);
    
    while (1)
    {
        INPUT = INPUTCOMMAND_NONE;
        // MP3 Stream
//        Mp3Stream(hMp3, &pStream, iBufPos, pSize, mboxProgress, rxFlags);
//        void Mp3Stream(HANDLE hMp3, INT8U **pBuf, INT32U *iBufPos, INT32U *bufLen, 
//               OS_EVENT * mbox, OS_FLAG_GRP *rxFlags)
        Mp3Stream(hMp3, &curNode, mboxProgress, rxFlags);
        // File Complete
        INPUT = SONG_COMPLETE;  
        OSMboxPost(mboxCommand, (void *)&INPUT);
    }
        
}
            
/************************************************************************************

   COMMAND

************************************************************************************/
void CommandTask(void* pdata)
{
    INT8U err;
    char buf[BUFSIZE];
    CommandEnum INPUT;
    CommandEnum *msgReceived;
    while (1) {
        msgReceived = (CommandEnum*)OSMboxPend(mboxCommand, 0, &err);
        Print_uint32((uint32_t)(*msgReceived));PrintWithBuf(buf, BUFSIZE, "\n");
        if (*msgReceived==INPUTCOMMAND_PREVSONG) {
            // turn off touch inputs until display is done doing it's work
            OSFlagPost(rxFlags, 0x1, OS_FLAG_SET, &err);
            // update streaming pointers 
            curNode->data.pos = 0;
            curNode->data.pStream = curNode->data.pStart;
            // update display
            OSMboxPost(mboxDisplay, (void *)msgReceived);
        }
        if (*msgReceived==INPUTCOMMAND_PLAY) {
            // turn off touch inputs until display is done doing it's work
            OSFlagPost(rxFlags, 0x1, OS_FLAG_SET, &err);
            STREAMING=!STREAMING;
            if (!STREAMING) {
                // pause audio
                OSFlagPost(rxFlags, 0x2, OS_FLAG_SET, &err); 
            }
            if (STREAMING) {
                // resume audio
                OSFlagPost(rxFlags, 0x2, OS_FLAG_CLR, &err);
            }
            OSMboxPost(mboxDisplay, (void *)msgReceived);
        }
        if (*msgReceived==INPUTCOMMAND_NEXTSONG) {
            // turn off touch inputs until display is done doing it's work
            OSFlagPost(rxFlags, 0x1, OS_FLAG_SET, &err);
            // update streaming pointers
            curNode->data.pos = 0;
            curNode->data.pStream = curNode->data.pStart;
            curNode = curNode->next;
            // update display
            OSMboxPost(mboxDisplay, (void *)msgReceived);
        }
        if (*msgReceived==SONG_COMPLETE) {
            // turn off touch inputs until display is done doing it's work
            OSFlagPost(rxFlags, 0x1, OS_FLAG_SET, &err); 
            // update streaming pointers
            curNode->data.pos = 0;
            curNode->data.pStream = curNode->data.pStart;
            curNode = curNode->next;
            // update display
            INPUT = SONG_COMPLETE; OSMboxPost(mboxDisplay, (void *)&INPUT);
        }
    }
}

// Renders a character at the current cursor position on the LCD
static void PrintCharToLcd(char c)
{
    lcdCtrl.write(c);
}

/************************************************************************************

   Print a formated string with the given buffer to LCD.
   Each task should use its own buffer to prevent data corruption.

************************************************************************************/
void PrintToLcdWithBuf(char *buf, int size, char *format, ...)
{
    va_list args;
    va_start(args, format);
    PrintToDeviceWithBuf(PrintCharToLcd, buf, size, format, args);
    va_end(args);
}




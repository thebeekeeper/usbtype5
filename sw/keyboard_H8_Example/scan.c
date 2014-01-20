#include "scan.h"
#include "msp430f5529.h"
#include "main.h"

#define blank 0x00
#define altGraph 0x00
#define find 0x00
#define cut 0x00
#define open 0x00
#define paste 0x00
#define front 0x00
#define copy 0x00
#define props 0x00
#define undo 0x00
#define stop 0x00
#define again 0x00
#define help 0x00


int key_map[8][19] = {
    { usbUsageRightAlt, usbUsageWindowsKey, usbUsageCapsLock, find, cut, usbUsageLeftShift, usbUsageSpacebar, blank, usbUsageC, usbUsageV, usbUsageB, blank, blank, usbUsageLeftArrow, usbUsageRightArrow, usbUsage2, usbUsage3, usbUsageDeleteForward, blank },
    { altGraph, usbUsageWindowsKey, blank, open, usbUsageLeftControl, paste, usbUsageRightShift, usbUsageComma, usbUsageZ, usbUsageX, usbUsageN, usbUsageM, usbUsagePeriod, blank, usbUsageDownArrow, usbUsageUpArrow, usbUsage5, usbUsage6, usbUsage0 },
    { blank, blank, blank, front, copy, blank, usbUsageJ, usbUsageA, usbUsageS, usbUsageD, usbUsageH, usbUsageK, usbUsageL, blank, usbUsage4, usbUsage7, usbUsage8, usbUsage1, blank },
    { blank, blank, blank, props, undo, blank, usbUsageO, usbUsageE, usbUsageR, usbUsageT, undo, usbUsage0, usbUsageLeftBracket, usbUsageBackspace, usbUsageDeleteForward, usbUsageHome, usbUsagePageUp, usbUsageKeypadPlus, blank },
    { blank, blank, blank, stop, blank, again, blank, usbUsage8, usbUsage3, usbUsage5, usbUsage6, usbUsage7, usbUsage9, usbUsageMinus, usbUsageBackslash, blank, blank, blank, blank },
    { blank, blank, blank, help, blank, blank, blank, usbUsageF8, usbUsageF1, usbUsageF2, usbUsageF3, usbUsageF6, usbUsageF10, usbUsageF11, usbUsageF12, usbUsagePrintScreen, usbUsageMute, usbUsageVolumeDown, usbUsageVolumeUp },
    { blank, blank, blank, usbUsageEscape, blank, usbUsage1, blank, usbUsageF7, usbUsage2, usbUsage4, usbUsageF3, usbUsageF5, usbUsageF9, usbUsageEqual, usbUsageTilde, usbUsageInsert, blank, blank, usbUsageKeypadMinus },
    { blank, blank, blank, usbUsageTab, blank, usbUsageQ, blank, usbUsageI, usbUsageW, usbUsageF, usbUsageG, usbUsageY, usbUsageP, usbUsageSemicolon, usbUsageRightBracket, usbUsageEnter, usbUsageEnd, usbUsagePageDown, usbUsageKeypadEnter }
};


void scan(int column)
{
  if(column == 0)
  {
    /* Find	1	5
      Cut	1	8
      Shift	1	10
      Caps Lock	1	3
      Alt	1	1
      Diamond Left	1	2
      Combose	1	16
      C	        1	13
      V 	1	14
      B 	1	15
      LEFT	1	18
      RIGHT 	1	19
      num 2	1	23
      num 3	1	24
      num del	1	25
      space	1	11
    */
    if(P1IN & BIT2) {
      // find - return ctrl+f
    } else if(P1IN & BIT7) {
      // return cut (ctrl+x)
    } else if(P2IN & BIT1) {
      // return shift
    } else if(P1IN & BIT1) {
      // return  caps lock;
    } else if(P1IN & BIT0) {
      // return alt
    } else if(P1IN & BIT4) {
      // lhs diamond
    } else if(P2IN & BIT7) {
      // compose
    } else if(P2IN & BIT4) { 
      // return usbUsageC
    } else if(P2IN & BIT5) {
      // return usbUsageV
    } else if(P2IN & BIT6) {
      // return usbUsageB
    } else if(P3IN & BIT1) {
      // return left arrow
    } else if(P3IN & BIT0) {
      // ah shit 19 isn't connected
      // return right arrow
    } else if(P3IN & BIT5) {
      // return 2
    } else if(P3IN & BIT6) {
      // return 3
    } else if(P3IN & BIT7) {
      // return delete
    } else if(P2IN & BIT2) {
      // return space
    }
    
  }
}
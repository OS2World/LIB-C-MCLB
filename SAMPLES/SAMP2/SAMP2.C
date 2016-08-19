/***************************************************************************/
/***************************************************************************/
/*                        DISCLAIMER OF WARRANTIES.                        */
/***************************************************************************/
/***************************************************************************/
/*                                                                         */
/*  Copyright (C) 1995 IBM Corporation                                     */
/*                                                                         */
/*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is        */
/*      sample code created by IBM Corporation. This sample code is not    */
/*      part of any standard or IBM product and is provided to you solely  */
/*      for  the purpose of assisting you in the development of your       */
/*      applications.  The code is provided "AS IS", without               */
/*      warranty of any kind.  IBM shall not be liable for any damages     */
/*      arising out of your use of the sample code, even if they have been */
/*      advised of the possibility of such damages.                        */
/***************************************************************************/
/*                                                                         */
/* Author:         Mark McMillan                                           */
/*                 IBM Corporation                                         */
/*                                                                         */
/***************************************************************************/

/*---------------------------------------------------------------------------
                             Sample #2

  This is a sample of an owner-drawn MCLB.  In this example, the first
  column of the MCLB is drawn with bitmaps.  We let PM draw the text
  into the 2nd column, so in effect we have a MCLB which is partially
  owner-drawn and partially drawn by PM.  This sample demonstrates:

    - Use of item handles
    - Ownerdrawing one or more columns
    - Use of LS_NOADJUSTPOS style (allows partial view of last row)

---------------------------------------------------------------------------*/


#define  INCL_BASE        
#define  INCL_WIN         
#define  INCL_DOS         
#define  INCL_WINSTDSPIN  
#define  INCL_GPI
                          
#include <os2.h>          
#include <string.h>       
#include <stdio.h>        
#include <stdlib.h>       

#include "mclb.h"               // MCLB definitions
                          
#include "DIALOG.H"       

#define  ID_MCLB   500          // ID of MCLB control
#define  V_MARGIN  2            // Vertical margin around bitmaps
#define  H_MARGIN  2            // Horizontal margin
                          
/* General Dialog Helper Macros */                                                                 
#define CONTROLID               SHORT1FROMMP(mp1)                                                  
#define CONTROLCODE             SHORT2FROMMP(mp1)                                                  
#define CONTROLHWND(ID)         WinWindowFromID(hwnd,ID)                                           
#define MSGBOX(Owner,Title,Msg) WinMessageBox(HWND_DESKTOP,Owner,Msg,Title,0,MB_OK|MB_INFORMATION) 
#define ERRBOX(Owner,Title,Msg) WinMessageBox(HWND_DESKTOP,Owner,Msg,Title,0,MB_OK|MB_ERROR)       
#define WARNBOX(Owner,Title,Msg) WinMessageBox(HWND_DESKTOP,Owner,Msg,Title,0,MB_OKCANCEL|MB_WARNING)
#define INSERTITEM(ID,Text)     (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_INSERTITEM,MPFROMSHORT(LIT_END),Text)
#define QUERYSELECTION(ID)      (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0L)
#define QUERYITEMCOUNT(ID)      (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMCOUNT,0L,0L)         
#define SETSELECTION(ID,Index)  WinSendDlgItemMsg(hwnd,ID,LM_SELECTITEM,MPFROMSHORT(Index),MPFROMSHORT(TRUE))
#define SETITEMHANDLE(ID,Index,Hand)  WinSendDlgItemMsg(hwnd,ID,LM_SETITEMHANDLE,MPFROMSHORT(Index),MPFROMLONG(Hand))
#define SETITEMTEXT(ID,Index,Text)  WinSendDlgItemMsg(hwnd,ID,LM_SETITEMTEXT,MPFROMSHORT(Index),MPFROMP(Text))
#define QUERYITEMHANDLE(ID,Index)  (ULONG)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMHANDLE,MPFROMSHORT(Index),0L)

/* Global static data.  Usually this would be in window instance */
/* data or defined only with the scope of use, but we put it     */
/* here just to keep the sample code simpler.                    */

HAB hab;                        // Application anchor block
HMQ MyQ;                        // Application message queue
HWND MCLBHwnd;                  // Handle of MCLB control
char Buff[100];                 // Msg buffer
SHORT Item;                     // Item index
LONG BmpVSize;                  // Vertical size of bitmaps
LONG BmpHSize;                  // Horizontal size of bitmaps

MRESULT EXPENTRY ID_MAINDLG_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*----------------------------------------------------------------------------*/
MRESULT EXPENTRY ID_MAINDLG_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
/*----------------------------------------------------------------------------*/
/* Dialog procedure for main window dialog.                                   */
/*----------------------------------------------------------------------------*/
{                                                                               
                                                                                
  switch (msg) {                                                                
                                                                                
    case WM_INITDLG: {
      /* Create the MCLB before the dialog window is displayed.  We don't */
      /* have to worry about sizing it now since we will get and process  */
      /* a WM_WINDOWPOSCHANGED message before the window becomes visible. */

      MCLBINFO InitData;
      LONG InitSizeList[2]= {1L, 5L};  // Make col 2 twice as big as column 1
      BITMAPINFOHEADER2 BmpHeader;     // Bitmap header
      USHORT       Index;              // Index of listbox item
      int          i;
      HPS          Hps;                // Presentation space for bitmaps
      HBITMAP      HBmp;               // Bitmap handle
      char         *ColText;           // String pointer

      /* Before we create the MCLB, we should determine how big the items */
      /* have to be.  PM will send us a WM_MEASUREITEM when the listboxes */
      /* are created so we must be prepared with the size of the items.   */

      /* Load one bitmap and save its size for use in WM_MEASUREITEM.     */
      /* All our bitmaps are the same size.                               */

      Hps  = WinGetPS(hwnd);
      HBmp = GpiLoadBitmap(Hps, NULLHANDLE, ID_BITMAPS+0, 0L, 0L);  // Load 1st bmp from resources
      BmpHeader.cbFix = sizeof(BITMAPINFOHEADER2);
      GpiQueryBitmapInfoHeader(HBmp, &BmpHeader);                   // Get header info
      BmpVSize = BmpHeader.cy + (2*V_MARGIN);                       // Save size + margin
      BmpHSize = BmpHeader.cx + (2*H_MARGIN);
      GpiDeleteBitmap(HBmp);

      /* Initialize MCLB create data structure */

      memset(&InitData, 0x00, sizeof(MCLBINFO));
      // These are the only required initialization values:
      InitData.Size = sizeof(MCLBINFO);
      InitData.Cols = 2;
      InitData.TabChar = '!';
      InitData.Titles = "Icon!Description";
      InitData.InitSizes= InitSizeList;  // Initial sizes (proportions)

      // Play with these for colors and fonts:
      // InitData.TitleBColor = 0x00FFFF00;     
      // InitData.TitleFColor = 0x00000000;
      // InitData.ListBColor = 0x0000FFFF;
      // InitData.ListFColor = 0x00000000;
      // strcpy(InitData.ListFont, "8.Helv");
      // strcpy(InitData.TitleFont, "10.Helvetica Bold Italic");

      /* Now create the MCLB.  The dialog window is the parent (so it */
      /* draws on the dialog), and owner (so this dialog proc will    */
      /* get messages from it).                                       */

      MCLBHwnd = MCLBCreateWindow(
                 hwnd,                    // Parent window
                 hwnd,                    // Owner to recv messages
                 WS_VISIBLE|              // Styles: Make it visible
                   WS_TABSTOP|              // Let user TAB to it
                   MCLBS_SIZEMETHOD_PROP|   // Proportional sizing when window is sized
                   LS_OWNERDRAW|            // All columns will be owner-draw style
         //        LS_NOADJUSTPOS|          // ---> If this is used, blank unpainted area can appear after last item in the list
                   LS_HORZSCROLL,           // Give each column a horizontal scroll bar
                 0,0,100,100,             // Will set size later, but must have large horz size now
                 HWND_TOP,                // Put on top of any sibling windows
                 ID_MCLB,                 // Window ID
                 &InitData);              // MCLB create structure

      /* Insert data into the MCLB.  For each row we load the bitmaps we */
      /* will use in the first column and save the bitmap handle in the  */
      /* item handle (there is one item handle available per row).  Note */
      /* that we will only use the text for column 2 so we just insert   */
      /* null strings in column 1.                                       */

      for (i=0; i<7; i++) {
        HBmp = GpiLoadBitmap(Hps, NULLHANDLE, ID_BITMAPS+i, 0L, 0L);  // Load bmp for this row
        switch (i) {
          case 0:  ColText = "!System clock setting"                                 ; break;
          case 1:  ColText = "!Keyboard mappings"                                    ; break;
          case 2:  ColText = "!Mouse speed and button mappings"                      ; break;
          case 3:  ColText = "!System sound settings and volume control"             ; break;
          case 4:  ColText = "!Country setting"                                      ; break;
          case 5:  ColText = "!Display resolution, background, and lockup settings"  ; break;
          case 6:  ColText = "!Default color and font schemes"                       ; break;
        }
        Index = INSERTITEM(ID_MCLB, ColText);   // Insert text to create a new row
        SETITEMHANDLE(ID_MCLB, Index, HBmp);    // Set row's handle to bitmap handle
      }
      WinReleasePS(Hps);  // Done with the PS now

      return FALSE;                                                             
      } // end of WM_INITDLG

    case WM_DESTROY: {
      /* Cleanup bitmaps when the dialog terminates */

      HBITMAP Handle;
      USHORT  i;

      for (i=QUERYITEMCOUNT(ID_MCLB); i>0; i--) {      // For each row
        Handle = (HBITMAP)QUERYITEMHANDLE(ID_MCLB, i); // Get bitmap handle
        GpiDeleteBitmap(Handle);                       // Delete it
      }
      break;
      }

    case WM_MEASUREITEM:
      /* We get this message whenever a listbox needs to know how big */
      /* the owner-drawn items are.  All listboxes in the MCLB must   */
      /* have the same vertical height.                               */

      /* PM docs indicate that the items cannot be smaller than the   */
      /* font size of the listbox.  So we must find out how big the   */
      /* font is, and set the item size to be the larger of the font  */
      /* size and the bitmap size.                                    */

      if (SHORT1FROMMP(mp1) == ID_MCLB) {
        USHORT Col;
        HWND   ColHwnd;
        HPS    Hps;
        RECTL  Rect;
        USHORT BmpSize;
        char   QText[100];

        /* Get handle of listbox -- it's ID is it's column number */

        Col = SHORT2FROMMP(mp1);
        ColHwnd = WinWindowFromID(CONTROLHWND(ID_MCLB), Col);

        /* Use WinDrawText(DT_QUERYEXTENT) to discover the bounding */
        /* box of a single blank character.  Note we must use a PS  */
        /* for the listbox window to get the right font.            */
        /* (For col 2 we get the size of the actual text).          */

        strcpy(QText, " ");
        if (Col == 2)
          WinSendMsg(ColHwnd, LM_QUERYITEMTEXT, MPFROM2SHORT(SHORT1FROMMP(mp2), 100), MPFROMP(QText));

        Hps = WinGetPS(ColHwnd);
        memset(&Rect, 0x00, sizeof(RECTL));
        Rect.yTop   = 32000;       // Make rectangle really big
        Rect.xRight = 32000;
        WinDrawText(Hps, -1, QText, &Rect, 0L, 0L, DT_QUERYEXTENT|DT_LEFT|DT_BOTTOM);
        WinReleasePS(Hps);

        /* Return larger of the vertical font size and vertical bitmap size. */
        /* For horizontal, return actual width.                              */

        if (Col == 1)
          return MRFROM2SHORT(max(BmpVSize ,Rect.yTop - Rect.yBottom), // Max of font or bmp size
                              BmpHSize);                               // Bmp size
        else
          return MRFROM2SHORT(max(BmpVSize ,Rect.yTop - Rect.yBottom), // Same as col 1
                              Rect.xRight - Rect.xLeft);               // Text size
      }
      break;

    case WM_DRAWITEM:
      /* We get this message when an item in a column needs to be drawn. */
      /* For column 1 we paint the supplied presentation space with a    */
      /* background and the bitmap.  For column 2 we just return FALSE   */
      /* so the listbox will draw it's own text.                         */

      if (SHORT1FROMMP(mp1) == ID_MCLB) {  // Draw is from our MCLB
        OWNERITEM *OwnerInfo;
        POINTL    Point;
        USHORT    Col;

        OwnerInfo = (OWNERITEM *)mp2;     // mp2 is ptr to OWNERITEM structure

        // Get column this draw is for
        Col = SHORT2FROMMP(mp1);
        if (Col != 1)  // Let listbox draw text in 2nd column
          return (MRESULT)FALSE;

        if (OwnerInfo->fsState == OwnerInfo->fsStateOld) {  // Redraw needed?
          WinFillRect(OwnerInfo->hps, &(OwnerInfo->rclItem), CLR_CYAN);
          // Center bitmaps in the item rectangle
          Point.x = (OwnerInfo->rclItem.xRight - OwnerInfo->rclItem.xLeft)/2 
                    - BmpHSize/2 + OwnerInfo->rclItem.xLeft;
          Point.y = (OwnerInfo->rclItem.yTop   - OwnerInfo->rclItem.yBottom)/2
                    - BmpVSize/2 + OwnerInfo->rclItem.yBottom;
          // Bitmap handle is item handle set when item was inserted
          WinDrawBitmap(OwnerInfo->hps, (HBITMAP)OwnerInfo->hItem,
                        NULL, &Point, 0L, 0L, DBM_NORMAL);

          if (OwnerInfo->fsState) // Let PM do hilighting
            OwnerInfo->fsStateOld = 0;
        }
        return (MRESULT)TRUE;  // Tell listbox that we drew the item
      }
      break;
                                                                                
    case WM_COMMAND:
      switch (SHORT1FROMMP(mp1)) {                                              
        case DID_OK: /*----------~OK (PUSHBUTTON)----------*/                   
          /* Since our main window is a dialog, don't let default dialog */
          /* proc dismiss us or focus will not always return to the      */
          /* proper application.  Instead, destroy ourselves.  This is   */
          /* a trick to properly using a dialog for a main window.       */
          WinDestroyWindow(hwnd);
          return 0;
      }                                                                         
      break;                                                                    
                                                                                
    case WM_CONTROL:                                                            
      switch SHORT1FROMMP(mp1) {                                                
        case ID_MCLB: 

          /* Process control messages from the MCLB.  Most of them are */
          /* standard listbox messages with the same meaning as usual. */
          /* There are also a few MCLB-specific control messages.      */

          switch SHORT2FROMMP(mp1) {
            case LN_SELECT:

              /* User selected or unselected an entry in the MCLB.  Our   */
              /* sample uses a single-select listbox.                     */

              Item = SHORT1FROMMR(WinSendMsg(MCLBHwnd, LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), MPVOID));
              if (Item >= 0)
                sprintf(Buff, "Item %i selected", Item);
              else
                strcpy(Buff, "Unable to query selection.");
              WinSetDlgItemText(hwnd, ID_MSG, Buff);
              return 0;

            case LN_ENTER:

              /* User double-clicked or pressed ENTER in the MCLB. */

              Item = SHORT1FROMMR(WinSendMsg(MCLBHwnd, LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), MPVOID));
              if (Item >= 0)
                sprintf(Buff, "Item %i entered", Item);
              else
                strcpy(Buff, "Unable to query selection.");
              WinSetDlgItemText(hwnd, ID_MSG, Buff);
              return 0;

            case MCLBN_COLSIZED: {

              /* The user has changed the relative column sizings using one */
              /* of the split bars.  Note we do not get this message when   */
              /* the entire MCLB is resized with the window, only when the  */
              /* user moves a splitbar.                                     */

              LONG SizeList[3];
              WinSendMsg(MCLBHwnd, MCLB_QUERYCOLSIZES, MPFROMP(SizeList), MPVOID);
              sprintf(Buff, "Columns resized to (%li, %li, %li)", SizeList[0], SizeList[1], SizeList[2]);
              WinSetDlgItemText(hwnd, ID_MSG, Buff);
              return 0;
              }
          } // switch on MCLB notify code
          break;
      }                                                                         
      break; /* End of WM_CONTROL */                                            

    case WM_WINDOWPOSCHANGED: {

      /* Dialog was resized, so we resize/move the dialog controls as needed. */
      /* When we resize the MCLB, it will adjust it's column widths according */
      /* the the MCLBS_SIZEMETHOD_* algorithm we specified in the style bits. */
      /* Resizing the control does NOT cause a MCLBN_COLSIZED control message.*/

      SWP  Pos1, Pos2;

      /* Let def dlg proc set frame controls */
      WinDefDlgProc(hwnd, msg, mp1, mp2);

      /* Size/place MCLB above OK button, centered in dialog.  Note that the */
      /* dialog window is the frame, so we must account for frame controls.  */

      WinQueryWindowPos(CONTROLHWND(DID_OK), &Pos1);
      WinQueryWindowPos(hwnd, &Pos2);
      Pos2.cy = Pos2.cy - WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR) - WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
      Pos2.cx = Pos2.cx - (2 * Pos1.x),
      WinSetWindowPos(MCLBHwnd, HWND_TOP,
         Pos1.x,
         Pos1.y+Pos1.cy+5,
         Pos2.cx,
         Pos2.cy - (Pos1.y+Pos1.cy+7+Pos1.y),
         SWP_MOVE|SWP_SIZE);

      /* Size/place message control */
      WinQueryWindowPos(CONTROLHWND(ID_MSG), &Pos1);
      WinQueryWindowPos(hwnd, &Pos2);
      WinSetWindowPos(CONTROLHWND(ID_MSG), HWND_TOP,
         0,0,
         Pos2.cx-Pos1.x-(2*WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER))-3,
         Pos1.cy,
         SWP_SIZE);

      return 0;  // Already called default proc, so just return.
      }
                                                                                
  } /* End of (msg) switch */                                                   
                                                                                
  return WinDefDlgProc(hwnd, msg, mp1, mp2);                                    
                                                                                
} /* End dialog procedure */                                                    

/*----------------------------------------------------------------------------*/
int main(int argc,char **argv, char **envp)                                     
/*----------------------------------------------------------------------------*/
/*  Main routine just runs a dialog box (no main window).                     */
/*----------------------------------------------------------------------------*/
{                                                                               
  /* Initialize PM window environment, get a message queue */                   
                                                                                
  hab = WinInitialize(0L);                                                      
  MyQ = WinCreateMsgQueue(hab, 0) ;                                             
                                                                                
  WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, (PFNWP)                                 
            ID_MAINDLG_Proc,                                                    
            NULLHANDLE,                                                         
            ID_MAINDLG,                                                         
            NULL);                                                              
                                                                                
  /* Cleanup when dialog terminates */                                          
                                                                                
  WinDestroyMsgQueue(MyQ);                                                      
  WinTerminate(hab);                                                            

  return 0;
}                                                                               

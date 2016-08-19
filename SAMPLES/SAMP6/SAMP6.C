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
                             Sample #6

  This is just a basic sample that shows a non-sizable MCLB using some
  different fonts and colors.

---------------------------------------------------------------------------*/


#define  INCL_BASE        
#define  INCL_WIN         
#define  INCL_DOS         
#define  INCL_WINSTDSPIN  
                          
#include <os2.h>          
#include <string.h>       
#include <stdio.h>        
#include <stdlib.h>       

#include "mclb.h"               // MCLB definitions
                          
#include "DIALOG.H"       

#define  ID_MCLB   500          // ID of MCLB control
                          
/* General Dialog Helper Macros */    
#define CONTROLID               SHORT1FROMMP(mp1)                                                  
#define CONTROLCODE             SHORT2FROMMP(mp1)                                                  
#define CONTROLHWND(ID)         WinWindowFromID(hwnd,ID)                                           

HAB hab;                        // Application anchor block
HMQ MyQ;                        // Application message queue
HWND MCLBHwnd;                  // Handle of MCLB control
char Buff[100];                 // Msg buffer
SHORT Item;                     // Item index

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
      LONG InitSizeList[3]= {3L, 1L};  // Make col 1 twice as big as 2 and 3

      /* Initialize MCLB create data structure */

      memset(&InitData, 0x00, sizeof(MCLBINFO));
      // These are the only required initialization values:
      InitData.Size = sizeof(MCLBINFO);
      InitData.Cols = 2;
      InitData.TabChar = '!';
      InitData.Titles = "Component!Installed";
      InitData.InitSizes= InitSizeList;  // Initial sizes (proportions)

      InitData.TitleBColor = 0x00FFFF00;     
      InitData.TitleFColor = 0x00000000;
      InitData.ListBColor = 0x0080FFFF;
      InitData.ListFColor = 0x00000000;
      strcpy(InitData.ListFont, "8.Helv");
      strcpy(InitData.TitleFont, "10.Helvetica Bold Italic");

      /* Now create the MCLB.  The dialog window is the parent (so it */
      /* draws on the dialog), and owner (so this dialog proc will    */
      /* get messages from it).                                       */

      MCLBHwnd = MCLBCreateWindow(
                 hwnd,                    // Parent window
                 hwnd,                    // Owner to recv messages
                 WS_VISIBLE|              // Styles: Make it visible
                   WS_TABSTOP|              // Let user TAB to it
                   MCLBS_SIZEMETHOD_PROP|   // Proportional sizing when window is sized
                   MCLBS_NOCOLRESIZE,       // Give each column a horizontal scroll bar
                 0,0,100,100,             // Will set size later, but must have large horz size now
                 HWND_TOP,                // Put on top of any sibling windows
                 ID_MCLB,                 // Window ID
                 &InitData);              // MCLB create structure

      /* Insert data into the MCLB.  Each LM_INSERTITEM inserts a single */
      /* row.  Columns are separated with the TabChar supplied above     */
      /* (an "!" for our sample here).                                   */

      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("Comm Subsystem!Yes"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("User Interface!Yes"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("Utilites!No"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("Import/Export!No"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("XIO Support!Yes"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("System Links!No"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("Online Help!Yes"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("Publications!Yes"));
      WinSendMsg(MCLBHwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), MPFROMP("Tutorial!No"));

      return FALSE;                                                             
      } // end of WM_INITDLG
                                                                                
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
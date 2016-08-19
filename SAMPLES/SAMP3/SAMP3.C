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
                             Sample #3

  This is just a basic drag/drop listbox sample that shows how to:

     - Add direct-manipulation capability to a listbox
     - Respond to drag/drop related DMLB messages
     - Display a context menu on the listbox
     - Process context menu messages

---------------------------------------------------------------------------*/
#define  INCL_BASE                   
#define  INCL_WIN                    
#define  INCL_DOS                    
#define  INCL_WINSTDSPIN             
                                     
#include <os2.h>                     
#include <string.h>                  
#include <stdio.h>                   
#include <stdlib.h>                  
                                     
#include "DIALOG.H"                  // IDs for dialogs
#include "dmlb.h"                    // DMLB definitions
#include "samp3.h"                   // Context menu IDs

#define  MAX_LEN  250                // Max length of listbox text

/* General Dialog Helper Macros */
#define CONTROLHWND(ID)         WinWindowFromID(hwnd,ID)                                           
#define INSERTITEM(ID,Text)     (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_INSERTITEM,MPFROMSHORT(LIT_END),Text)
#define INSERTITEMAT(ID,At,Text) (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_INSERTITEM,MPFROMSHORT(At),Text) 
#define DELETEITEM(ID,Index)    WinSendDlgItemMsg(hwnd,ID,LM_DELETEITEM,MPFROMSHORT(Index),0L)     
#define QUERYSELECTION(ID)      (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0L)
#define QUERYITEMCOUNT(ID)      (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMCOUNT,0L,0L)     
#define QUERYITEMTEXT(ID,Index,Buff,Size) (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMTEXT,MPFROM2SHORT(Index,Size),MPFROMP(Buff))
#define QUERYITEMTEXTLEN(ID,Index) (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMTEXTLENGTH,MPFROM2SHORT(Index),0L)
#define SETSELECTION(ID,Index)  WinSendDlgItemMsg(hwnd,ID,LM_SELECTITEM,MPFROMSHORT(Index),MPFROMSHORT(TRUE))
#define SETITEMTEXT(ID,Index,Text)  WinSendDlgItemMsg(hwnd,ID,LM_SETITEMTEXT,MPFROMSHORT(Index),MPFROMP(Text))
#define SETTEXTLIMIT(ID,Size)   WinSendDlgItemMsg(hwnd,ID,EM_SETTEXTLIMIT,MPFROMSHORT(Size),0L)
#define SETTEXT(ID,Buff)        WinSetWindowText(WinWindowFromID(hwnd,ID),Buff)                
#define QUERYTEXT(ID,Buff,Size) WinQueryWindowText(WinWindowFromID(hwnd,ID),Size,Buff)         
#define QUERYTEXTLEN(ID)        WinQueryWindowTextLength(WinWindowFromID(hwnd,ID))             
                                                                                                   
MRESULT EXPENTRY DLG_EDIT_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);                   
MRESULT EXPENTRY DLG_MAIN_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);                   

/*----------------------------------------------------------------------------*/                   
MRESULT EXPENTRY DLG_MAIN_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)                    
/*----------------------------------------------------------------------------*/                   
/* This dialog proc initializes and handles all messages for the main         */
/* dialog.                                                                    */
/*----------------------------------------------------------------------------*/                   
{                                                                                                  
static HWND  ContextHwnd;  /* Handle of context menu                     */
static SHORT ContextIndx;  /* Index of item context menu was selected on */
                                                                                                   
  switch (msg) {                                                                                   
                                                                                                   
    case WM_INITDLG:                                                                               
      INSERTITEM(LIST, "Listbox item 0");
      INSERTITEM(LIST, "Listbox item 1");
      INSERTITEM(LIST, "Listbox item 2");
      INSERTITEM(LIST, "Listbox item 3");
      INSERTITEM(LIST, "Listbox item 4");
      INSERTITEM(LIST, "Listbox item 5");
      INSERTITEM(LIST, "Listbox item 6");
      INSERTITEM(LIST, "Listbox item 7");
      INSERTITEM(LIST, "Listbox item 8");
      INSERTITEM(LIST, "Listbox item 9");
      INSERTITEM(LIST, "Listbox item 10");
      INSERTITEM(LIST, "Listbox item 11");
      INSERTITEM(LIST, "Listbox item 12");
      INSERTITEM(LIST, "Listbox item 13");
      INSERTITEM(LIST, "Listbox item 14");
      INSERTITEM(LIST, "Listbox item 15");

      /* Add direct-manipulation capability to the listbox */
      DMLBInitialize(CONTROLHWND(LIST), NULLHANDLE);

      /* Load context menu for later use */
      ContextHwnd = WinLoadMenu(hwnd, NULLHANDLE, ID_MENU);

      return FALSE;                                           

    case WM_DESTROY:
      /* Free resources */
      WinDestroyWindow(ContextHwnd);
      break;
                                                              
    case WM_COMMAND:                                          
      switch (SHORT1FROMMP(mp1)) {                            

        /*******************************************************************/
        /* We get WM_COMMAND messages from the context menu.  The variable */
        /* ContextIndx already has the index of the item on which the      */
        /* context menu was requested (or LIT_NONE).                       */
        /*******************************************************************/

        case ID_MENU_EDIT: {  //------- Context menu "Edit" selected -----------
          char Buff[MAX_LEN];
          /* Run edit menu, update list if user selects OK button */
          QUERYITEMTEXT(LIST, ContextIndx, Buff, sizeof(Buff));
          if (WinDlgBox(HWND_DESKTOP, hwnd, (PFNWP)DLG_EDIT_Proc, NULLHANDLE, DLG_EDIT, Buff)
              == DID_OK)
            SETITEMTEXT(LIST, ContextIndx, Buff);
          return 0;
          }

        case ID_MENU_DELETE:  //------- Context menu "Delete" selected -----------
          WinSendMsg(CONTROLHWND(LIST), LM_DELETEITEM, MPFROMSHORT(ContextIndx), MPVOID);
          return 0;

        case ID_MENU_INSERT: {//------- Context menu "Insert" selected ----------
          char Buff[MAX_LEN];
          /* Run edit menu, insert into list if user selects OK button */
          Buff[0] = '\0';
          if (WinDlgBox(HWND_DESKTOP, hwnd, (PFNWP)DLG_EDIT_Proc, NULLHANDLE, DLG_EDIT, Buff)
              == DID_OK) {
            if (ContextIndx == LIT_NONE)  // Insert at end of list
              ContextIndx = LIT_END;
            else
              ContextIndx++;              // Insert AFTER selected item
            INSERTITEMAT(LIST, ContextIndx, Buff);
          }
          return 0;
          }

        case DID_OK: /*----------~OK (PUSHBUTTON)----------*/ 
          // Let default dialog proc terminate us
          break;                                              
      }                                                       
      break;                                                  
                                                              
    case WM_CONTROL:                                          
      switch SHORT1FROMMP(mp1) {                              
        case LIST: /*----------LIST (LISTBOX)----------*/     
 
          /*******************************************************************/
          /* Notification messages from the listbox come here.  These are    */
          /* the usual PM listbox messages, plus some new ones for DMLB.     */
          /*******************************************************************/
 
          switch (SHORT2FROMMP(mp1)) {
            case LN_SELECT:           
            case LN_ENTER:            
              /* User dbl-clicked an entry or pressed ENTER.  For this sample       */
              /* we do nothing.                                                     */
              return 0;

            case LN_DMLB_QRYDROP:
              /* A listbox item is about to be dropped on this listbox.  We must    */
              /* return a Ok-to-drop  indicator, and tell what to do with the       */
              /* original item (copy or move).  Since we have just one listbox in   */
              /* this sample, MOVE is the only thing that makes sense.              */
              return MRFROM2SHORT(TRUE, DROPMODE_MOVE);

            case LN_DMLB_CONTEXT: {
              /* The user has requested a context menu.  Short 1 of mp2 is the      */
              /* index of the listbox item under the pointer.                       */
              POINTL Point;
              ContextIndx = SHORT1FROMMP(mp2);
              if (ContextIndx == LIT_NONE) {
                // Disable Edit & Delete if pointer was not over a listbox item
                WinSendMsg(ContextHwnd, MM_SETITEMATTR, MPFROM2SHORT(ID_MENU_EDIT,FALSE),   MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
                WinSendMsg(ContextHwnd, MM_SETITEMATTR, MPFROM2SHORT(ID_MENU_DELETE,FALSE), MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
              } else {
                // Otherwise, enable Edit & Delete
                WinSendMsg(ContextHwnd, MM_SETITEMATTR, MPFROM2SHORT(ID_MENU_EDIT,FALSE),   MPFROM2SHORT(MIA_DISABLED, ~MIA_DISABLED));
                WinSendMsg(ContextHwnd, MM_SETITEMATTR, MPFROM2SHORT(ID_MENU_DELETE,FALSE), MPFROM2SHORT(MIA_DISABLED, ~MIA_DISABLED));
              }
              // It is not technically necessary to select the item on which the context
              // menu was selected, but it provides good visual feedback to the user to
              // show which item the context menu applies to.
              SETSELECTION(LIST, ContextIndx);

              // Popup the context menu at the pointer position.  The menu will
              // send us WM_COMMAND messages if a menu item is selected.
              WinQueryMsgPos((HAB)0, &Point);
              WinPopupMenu(HWND_DESKTOP, hwnd, ContextHwnd, Point.x, Point.y, 0, PU_HCONSTRAIN| PU_VCONSTRAIN
                                                            | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 | PU_MOUSEBUTTON3 
                                                            | PU_KEYBOARD );
              return (MRESULT)TRUE;
              }
          }                           
          break;                      
      }                               
      break; /* End of WM_CONTROL */          
                                              
  } /* End of (msg) switch */                 
                                              
  return WinDefDlgProc(hwnd, msg, mp1, mp2);  
                                              
} /* End dialog procedure */                  

/*----------------------------------------------------------------------------*/                   
MRESULT EXPENTRY DLG_EDIT_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)                    
/*----------------------------------------------------------------------------*/                   
/*                                                                            */                   
/*----------------------------------------------------------------------------*/                   
{                                                                                                  
char *Buff;

  Buff = WinQueryWindowPtr(hwnd, 0L); // We keep ptr to buffer in window words
                                                                                                   
  switch (msg) {                                                                                   
                                                                                                   
    case WM_INITDLG:                                                                               
      /* Initial string ptr is passed in mp2. */
      Buff = (char *)mp2;
      WinSetWindowPtr(hwnd, 0L, Buff);
      SETTEXTLIMIT(TEXT, MAX_LEN);
      SETTEXT(TEXT, Buff);
      return FALSE;                                                                                
                                                                                                   
    case WM_COMMAND:                                                                               
      switch (SHORT1FROMMP(mp1)) {                                                                 
        case DID_OK: /*----------~OK (PUSHBUTTON)----------*/                                      
          /* Update caller's buffer and end dialog */
          QUERYTEXT(TEXT, Buff, MAX_LEN);
          break;                                                                                   
      }                                                                                            
      break;                                                                                       
                                                                                                   
  } /* End of (msg) switch */                                                                      
                                                                                                   
  return WinDefDlgProc(hwnd, msg, mp1, mp2);                                                       
                                                                                                   
} /* End dialog procedure */                                                                       

/*----------------------------------------------------------------------------*/
int      main(int argc,char **argv, char **envp)                                
/*----------------------------------------------------------------------------*/
/*  Main routine just runs a dialog box (no main window).                     */
/*----------------------------------------------------------------------------*/
{                                                                               
HAB hab;                                                                        
HMQ MyQ;                                                                        
                                                                                
  /* Initialize PM window environment, get a message queue */                   
                                                                                
  hab = WinInitialize(0L);                                                      
  MyQ = WinCreateMsgQueue(hab, 0) ;                                             
                                                                                
  WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, (PFNWP)                                 
            DLG_MAIN_Proc,                                                      
            NULLHANDLE,                                                         
            DLG_MAIN,                                                           
            NULL);                                                              
                                                                                
  /* Cleanup when dialog terminates */                                          
                                                                                
  WinDestroyMsgQueue(MyQ);                                                      
  WinTerminate(hab);                                                            

  return 0;
}                                                                               

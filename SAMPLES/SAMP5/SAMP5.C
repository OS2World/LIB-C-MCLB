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
                             Sample #5

  This sample demonstrates drag/drop of listbox items between different
  listbox controls.

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
#define  MSG_UPDATE_SAMPLE           (WM_USER+200)

/* General Dialog Helper Macros */
#define CONTROLHWND(ID)         WinWindowFromID(hwnd,ID)                                           
#define INSERTITEM(ID,Text)     (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_INSERTITEM,MPFROMSHORT(LIT_END),Text)
#define INSERTITEMAT(ID,At,Text) (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_INSERTITEM,MPFROMSHORT(At),Text) 
#define DELETEITEM(ID,Index)    WinSendDlgItemMsg(hwnd,ID,LM_DELETEITEM,MPFROMSHORT(Index),0L)     
#define QUERYSELECTION(ID)      (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0L)
#define QUERYITEMCOUNT(ID)      (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMCOUNT,0L,0L)     
#define QUERYITEMTEXT(ID,Index,Buff,Size) (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMTEXT,MPFROM2SHORT(Index,Size),MPFROMP(Buff))
#define QUERYITEMTEXTLEN(ID,Index) (SHORT)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMTEXTLENGTH,MPFROM2SHORT(Index),0L)
#define SETITEMHANDLE(ID,Index,Hand)  WinSendDlgItemMsg(hwnd,ID,LM_SETITEMHANDLE,MPFROMSHORT(Index),MPFROMLONG(Hand))
#define QUERYITEMHANDLE(ID,Index)  (ULONG)WinSendDlgItemMsg(hwnd,ID,LM_QUERYITEMHANDLE,MPFROMSHORT(Index),0L)
#define SETSELECTION(ID,Index)  WinSendDlgItemMsg(hwnd,ID,LM_SELECTITEM,MPFROMSHORT(Index),MPFROMSHORT(TRUE))
#define SETITEMTEXT(ID,Index,Text)  WinSendDlgItemMsg(hwnd,ID,LM_SETITEMTEXT,MPFROMSHORT(Index),MPFROMP(Text))
#define SETTEXTLIMIT(ID,Size)   WinSendDlgItemMsg(hwnd,ID,EM_SETTEXTLIMIT,MPFROMSHORT(Size),0L)
#define SETTEXT(ID,Buff)        WinSetWindowText(WinWindowFromID(hwnd,ID),Buff)                
#define QUERYTEXT(ID,Buff,Size) WinQueryWindowText(WinWindowFromID(hwnd,ID),Size,Buff)         
#define QUERYTEXTLEN(ID)        WinQueryWindowTextLength(WinWindowFromID(hwnd,ID))             
                                                                                                   
MRESULT EXPENTRY DLG_MAIN_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);                   

/*----------------------------------------------------------------------------*/                   
MRESULT EXPENTRY DLG_MAIN_Proc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)                    
/*----------------------------------------------------------------------------*/                   
/* This dialog proc initializes and handles all messages for the main         */
/* dialog.                                                                    */
/*----------------------------------------------------------------------------*/                   
{                                                                                                  
SHORT Item;

// If the following is TRUE we generate code to support multiple copies of
// the database fields in the export list.  In this mode:
//   - Items are COPIED when dragged from the database list to the export list
//   - Items are DELETED when dragged from the export list to the database list
//
// If this is FALSE, we generate code such that only one copy of any given
// field is allowed in the export list:
//   - Items are MOVED when dragged from the database to the export list
//   - Items are MOVED when dragged from the export list to the database

static BOOL AllowMultiples = TRUE;
                                                                                                   
  switch (msg) {                                                                                   
                                                                                                   
    case WM_INITDLG:                                                                               

      /* Build some starter data in both listboxes.  The text in the */
      /* listbox is the field name, and we set the handle of each    */
      /* item to the sample data for that field.                     */

      Item = INSERTITEM(LIST2, "Last name");
      SETITEMHANDLE    (LIST2, Item, "McMillan");
      Item = INSERTITEM(LIST2, "First name");
      SETITEMHANDLE    (LIST2, Item, "Mark");
      Item = INSERTITEM(LIST2, "Middle initial");
      SETITEMHANDLE    (LIST2, Item, "A");
      Item = INSERTITEM(LIST2, "Street");
      SETITEMHANDLE    (LIST2, Item, "4922 Appletree Lane");
      Item = INSERTITEM(LIST2, "City");
      SETITEMHANDLE    (LIST2, Item, "Raleigh");
      Item = INSERTITEM(LIST2, "State");
      SETITEMHANDLE    (LIST2, Item, "NC");
      Item = INSERTITEM(LIST2, "Zip code");
      SETITEMHANDLE    (LIST2, Item, "27462");
      Item = INSERTITEM(LIST2, "Customer number");
      SETITEMHANDLE    (LIST2, Item, "382F-3922");
      Item = INSERTITEM(LIST2, "Date of entry");
      SETITEMHANDLE    (LIST2, Item, "03/21/95");
      Item = INSERTITEM(LIST2, "License ID");
      SETITEMHANDLE    (LIST2, Item, "22F84EC1");
      Item = INSERTITEM(LIST2, "Contact method");
      SETITEMHANDLE    (LIST2, Item, "Phone");
      Item = INSERTITEM(LIST2, "E-mail");
      SETITEMHANDLE    (LIST2, Item, "nobody@nowhere.net");
      Item = INSERTITEM(LIST2, "Version");
      SETITEMHANDLE    (LIST2, Item, "1.4.1");
      Item = INSERTITEM(LIST2, "Last upgrade");
      SETITEMHANDLE    (LIST2, Item, "00/00/00");

      /* Add direct-manipulation capability to both listboxs */
      DMLBInitialize(CONTROLHWND(LIST1), NULLHANDLE);
      DMLBInitialize(CONTROLHWND(LIST2), NULLHANDLE);

      /* Initialize sample display */
      WinSendMsg(hwnd, MSG_UPDATE_SAMPLE, MPVOID, MPVOID);

      return FALSE;                                           

    case WM_DESTROY:
      /* Free resources */
      break;

    case MSG_UPDATE_SAMPLE: {
      /* Update the sample text by extracting the sample data from */
      /* the left listbox item handles.                            */

      char BigBuff[400];
      SHORT i;

      BigBuff[0] = '\0';
      for (i=0; i<QUERYITEMCOUNT(LIST1); i++) {  // Build sample record
        strcat(BigBuff, (char *)QUERYITEMHANDLE(LIST1, i));
        strcat(BigBuff, " ,");
      }

      if (BigBuff[0] != '\0')
        BigBuff[strlen(BigBuff)-1] = '\0';  // Remove trailing delimiter

      SETTEXT(SAMPLE, BigBuff);  // Update display
      return 0;
      }
                                                              
    case WM_COMMAND:                                          
      switch (SHORT1FROMMP(mp1)) {                            
        case DID_OK: /*----------~OK (PUSHBUTTON)----------*/ 
          // Let default dialog proc terminate us
          break;                                              
      }                                                       
      break;                                                  
                                                              
    case WM_CONTROL:                                          
      switch SHORT1FROMMP(mp1) {                              

        /*******************************************************************/
        /* Notification messages from the listboxs come here.  These are   */
        /* the usual PM listbox messages, plus some new ones for DMLB.     */
        /*******************************************************************/
        case LIST1: /*----------LIST1 (LISTBOX)----------*/

          /* Messages from the "export fields" listbox */
 
          switch (SHORT2FROMMP(mp1)) {
            case LN_SELECT:           
            case LN_ENTER:            
              /* User dbl-clicked an entry or pressed ENTER.  For this sample       */
              /* we do nothing.                                                     */
              return 0;

            case LN_DMLB_QRYDROP:
              /* A listbox item is about to be dropped on this listbox.  We must    */
              /* return an Ok-to-drop indicator, and tell what to do with the       */
              /* original item (copy or move).                                      */

              if (HWNDFROMMP(mp2) == CONTROLHWND(LIST1))  // Always MOVE in my own list
                return MRFROM2SHORT(TRUE, DROPMODE_MOVE);

              if (AllowMultiples)
                return MRFROM2SHORT(TRUE, DROPMODE_COPY); // Copy from field list
              else
                return MRFROM2SHORT(TRUE, DROPMODE_MOVE); // Move from field list

            case LN_DMLB_REORDERED:
            case LN_DMLB_DELETE:
            case LN_DMLB_DELETE_MOVE:
            case LN_DMLB_INSERT_MOVE:
            case LN_DMLB_INSERT_COPY:
              /* The list of fields has been updated by drag/drop (an item has      */
              /* been moved in the list, dragged to the other list and deleted from */
              /* this one, or dragged from the other list and inserted here).  When */
              /* any of these occure we must update the sample display.  Note we    */
              /* must POST this because DELETE messages are sent before the delete  */
              /* actually occures (so we use POST to delay our update).             */
              WinPostMsg(hwnd, MSG_UPDATE_SAMPLE, MPVOID, MPVOID);
              return 0;
          }                           
          break;                      

        case LIST2: /*----------LIST2 (LISTBOX)----------*/

          /* Messages from the "all fields" listbox */
 
          switch (SHORT2FROMMP(mp1)) {
            case LN_SELECT:           
            case LN_ENTER:            
              /* User dbl-clicked an entry or pressed ENTER.  For this sample       */
              /* we do nothing.                                                     */
              return 0;

            case LN_DMLB_QRYDROP:
              /* A listbox item is about to be dropped on this listbox.  We must    */
              /* return an Ok-to-drop indicator,   and tell what to do with the     */
              /* original item (copy or move).                                      */

              if (HWNDFROMMP(mp2) == CONTROLHWND(LIST2))   // Drop on self?
                return MRFROM2SHORT(FALSE, 0);             // Don't allow
              else {
                if (AllowMultiples)  // Just delete if our list stays full
                  return MRFROM2SHORT(TRUE, DROPMODE_DELETE);
                else                 // Move field back to this list
                  return MRFROM2SHORT(TRUE, DROPMODE_MOVE);
              }
          }                           
          break;                      
      }                               
      break; /* End of WM_CONTROL */          
                                              
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
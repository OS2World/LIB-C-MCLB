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
/*--------- Direct Manipulation ListBox (DMLB) Subclass Procs -------------*/
/*                                                                         */
/*  Author:            Mark McMillan                                       */
/*                     IBM Corp.                                           */
/*                     Research Triangle Park, NC                          */
/*                     USA                                                 */
/*                                                                         */
/*  Original Concepts: Alan Warren,                                        */
/*                     IBM Yorktown Research                               */
/*                                                                         */
/*  Copyright (c) IBM Corp. 1995                                           */
/*-------------------------------------------------------------------------*/

/*===========================================================================
 
                     Direct Manipulation ListBox
 
   This is a set of functions to add direct-manipulation capability to a
   standard PM listbox control.  Several DM functions are provided:
 
     (1) Drag/drop reordering of the list using the system-defined
         drag button (usually button 2).

     (2) Drag/drop between different listboxes in the same application.
         Application can control copy/move functions.

     (3) Context menu popup capability using the system-define context
         menu action (usually click button 2).

   When dragging an item, zones are defined such that the item will
   be inserted at the closest item boundry.  Moving the mouse outside of the
   listbox to the north or south will initiate automatic scrolling in that  
   direction.                                                               

   When the context-menu button is selected on a listbox item, the
   application is notified so that it may present a popup context menu.

   The application will receive an extended set of WM_CONTROL messages
   giving information about DM-related operations on the listbox and to
   give the application control over inter-listbox drops. 

   These functions will also work with a Multi-Column listbox (MCLB).

   No additional application logic is required to use this function except
   for a single call to initialize the DM functions for a specific listbox
   control DMLBInitialize().  The window words of the listbox will be used
   by these functions and are not available to the application.  However,
   the after DMLBInitialize() the application may use the first pointer in
   the structured pointed to by the window words.  E.g.,

     MyAppPtr = *((void *)WinQueryWindowPtr(LBHwnd, 0L));

   The application needs to define the following resources which may be
   bound to the EXE or reside in a DLL (the .PTR files are supplied
   with this toolkit):

     POINTER ID_DMLB_DRAGMOVE "lbdrag.ptr"    
     POINTER ID_DMLB_DRAGCOPY "lbdragcp.ptr"  
     POINTER ID_DMLB_DRAGNONE "nodrag.ptr"    
     POINTER ID_DMLB_DRGNORTH "drgnorth.ptr"  
     POINTER ID_DMLB_DRGSOUTH "drgsouth.ptr"  
                                                                      
   Known Limitations:                                                 

     - The listbox control window words are used to store per-instance  
       data required by this routine.  The first pointer of that instance
       data is available for application use (initialized to NULL).     
       This data area is not available until after the application has  
       called the DMLBInitialize() function.

     - Multiple-select listboxes are not supported.  Only a single item 
       can be selected and moved at a time.  Some additional code logic 
       could be added to overcome this if necessary.                    

     - Items can be dragged between listboxes only in the same process. 

===========================================================================*/
                                                                        
/*-------------------------------------------------------------------------*/
/* Updates:                                                                */
/*   06/14/95 - Added capability to handle OWNERDRAW listbox style.        */
/*   06/15/95 - Use system drag messages instead of drag on button 2 dn.   */
/*   06/15/95 - Fixed some bounds checking when LB is empty.               */
/*   06/15/95 - Added context menu capability.                             */
/*   06/15/95 - Added ability to d/d between different list boxes.         */
/*   07/22/95 - Removed need for app to setup subclass, provide all        */
/*              setup/initialization in DMLBInitialize() API.              */
/*   10/24/95 - Insure only one selection for LN_DMLB_INSERT_COPY/MOVE.    */
/*-------------------------------------------------------------------------*/

#define  INCL_WIN
#define  INCL_DOS
#define  INCL_PM                               
#define  INCL_DOSMEMMGR
#define  INCL_ERRORS
#include <os2.h>                             

/* C standard Library header files   */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dmlb.h"             // Externalized definitions

#define LLI_UNDER       1     // Parameter values for DMLBLocateListboxItem()
#define LLI_INSERTPOINT 2

#define POINT_INSIDE    1     /* Pointer is inside a drop-accepting listbox */
#define POINT_OUTSIDE   2     /* Not in a drop-accepting listbox            */
#define POINT_NORTH     3     /* Just north of good listbox, scroll it      */
#define POINT_SOUTH     4     /* Just south of good listbox, scroll it      */
#define DRAG_TIMERID     TID_USERMAX-1  /* Timer ID */

#define LISTBOX_CLASS_STRING "#7"       /* Requst of WinQueryClassName()   */

typedef struct DMLBDataStruct {          /* Per-instance listbox data       */
       void     *AppData;               /* Place for application data      */
       HPOINTER DragMIcon;              /* Pointer icon for dragging (move)*/
       HPOINTER DragCIcon;              /* Pointer icon for dragging (copy)*/
       HPOINTER DragNoDrp;              /* Pointer icon for dragging (none)*/
       HPOINTER DeletIcon;              /* Pointer icon for draggind (del) */
       HPOINTER NorthIcon, SouthIcon;   /* Up/down pointer icons           */
       HMODULE  ResHMod;                /* Module with ptr resources       */
       PFNWP    OldProcAddr;            /* Previous proc for this listbox  */
       HWND     TargetHwnd;             /* Handle of drop-target listbox   */
       USHORT   TargetDropMode;         /* Type of drop target requests    */
       SHORT    PrevLocation;           /* Last known location of pointer  */
       BOOL     Dragging;               /* Drag is currently in progress   */
       } DMLBData, *PDMLBData;

/* Internal prototypes */
void DMLBCheckTargetLocation(HWND SrcHwnd, DMLBData *InstData, SHORT X, SHORT Y);
SHORT DMLBLocateListboxItem( HWND hwnd, HWND RelHwnd, SHORT Y, SHORT Option );
MRESULT EXPENTRY DMLBSubclassListboxProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);

/*----------------------------------------------------------------------*/
BOOL EXPENTRY DMLBInitialize(HWND LBHwnd, HMODULE ResourceHMod)
/*----------------------------------------------------------------------*/
/* Setup direct-manipulation capabilites on given listbox.  This        */
/* function takes care of creating the instance data needed by the      */
/* subclass procedure, and subclassing the listbox window.  This is     */
/* the only external API for DMLB.                                      */
/*----------------------------------------------------------------------*/
{
DMLBData *InstData;

  /* Create and initialize listbox instance data.  Note that we use */
  /* the listbox window words for a ptr to our instance data.  We   */
  /* preserve the current value of the window words in the first    */
  /* PVOID of our instance data where the app can use it.           */

  InstData = malloc(sizeof(DMLBData));
  memset(InstData, 0x00, sizeof(DMLBData));
  InstData->AppData      = WinQueryWindowPtr(LBHwnd, QWL_USER);
  InstData->TargetHwnd   = LBHwnd;         // Inital target is self
  InstData->TargetDropMode= DROPMODE_MOVE; // Default drop mode is MOVE
  InstData->ResHMod      = ResourceHMod;   // Module to load ptr resources

  /* Note: must set this before subclassing since the subclass */
  /* proc assumes it is set.                                   */

  WinSetWindowPtr(LBHwnd, QWL_USER, InstData);
  InstData->OldProcAddr = WinSubclassWindow(LBHwnd, (PFNWP)DMLBSubclassListboxProc);
  if (InstData->OldProcAddr == NULL) {
    free(InstData);
    return FALSE;
  }
  return TRUE;
}

/*----------------------------------------------------------------------*/
MRESULT EXPENTRY DMLBSubclassListboxProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
/*----------------------------------------------------------------------*/
/* This window procedure is used to subclass a standard PM listbox      */
/* control.  This procedure will intercept certain mouse events on the  */
/* listbox to implement direct-manipulation functions.                  */
/*----------------------------------------------------------------------*/
{
SHORT    Item;          /* Listbox item number                          */
DMLBData  *InstData;     /* This instance-specific data (per listbox)    */

   /* The lisbox window pointer is to our instance data. */

   InstData = WinQueryWindowPtr(hwnd, QWL_USER);
     
   switch (msg) {
      /* Since this is just a subclass setup after the listbox window */
      /* is created, we never get a WM_CREATE message here.           */

      case WM_DESTROY:
           /* The listbox window is being destroyed.  Cleanup any     */
           /* resources we have allocated in this subclass.           */
           if (InstData->DragMIcon != NULLHANDLE) {
             WinDestroyPointer(InstData->DragMIcon);
             WinDestroyPointer(InstData->DragCIcon);
             WinDestroyPointer(InstData->NorthIcon);
             WinDestroyPointer(InstData->SouthIcon);
             WinDestroyPointer(InstData->DragNoDrp);
             WinDestroyPointer(InstData->DeletIcon);
             InstData->DragMIcon = NULLHANDLE;
           }
           /* Cleanup other resources */
           if (InstData->Dragging) {
             WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,DRAG_TIMERID);
           }
           /* Release instance data */
           free(InstData);
           WinSetWindowPtr(hwnd, 0L, NULL);
           break;

      case WM_TIMER:
           /* We get timer messages during dragging to implement */
           /* auto-scrolling of listbox when pointer is placed   */
           /* north or south of the listbox while dragging.      */

           if (!InstData->Dragging)  /* Ignore if not dragging   */
             break;

           if (SHORT1FROMMP(mp1)==DRAG_TIMERID) {
             switch (InstData->PrevLocation) { // Last known location of the pointer
               case POINT_INSIDE:
               case POINT_OUTSIDE:
                 /* Do nothing */
                 break;

               case POINT_NORTH:
                 /* Scroll up one item */
                 Item = (SHORT)WinSendMsg(InstData->TargetHwnd, LM_QUERYTOPINDEX,  0L, 0L);
                 if ((Item != LIT_NONE) && (Item != 0))
                    WinPostMsg(InstData->TargetHwnd, LM_SETTOPINDEX, MPFROMSHORT(Item-1), 0L);
                 break;

               case POINT_SOUTH:
                 /* Scroll down one item */
                 Item = (SHORT)WinSendMsg(InstData->TargetHwnd, LM_QUERYTOPINDEX,  0L, 0L);
                 if (Item != LIT_NONE)
                    WinPostMsg(InstData->TargetHwnd, LM_SETTOPINDEX, MPFROMSHORT(Item+1), 0L);
                 break;
             } /* switch on PrevLocation */
             return 0;
           }
           break;

      case WM_CONTEXTMENU: {
           SHORT CursorIndx, Max;

           /* User requested context menu... notify our owner.          */

           /* First find out what item the pointer is over.             */
           Max = (SHORT)WinSendMsg( hwnd, LM_QUERYITEMCOUNT, 0L, 0L );
           CursorIndx = DMLBLocateListboxItem(hwnd, hwnd, SHORT2FROMMP(mp1), LLI_UNDER);
           if ((Max == 0) || (CursorIndx+1 > Max))
             CursorIndx = LIT_NONE;

           /* Tell our owner about it */
           return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                      MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), LN_DMLB_CONTEXT),
                      MPFROMSHORT(CursorIndx));
           }
 
      case WM_MOUSEMOVE:
           /* Monitor the position of the mouse relative to the listbox */
           /* so we can set the pointer icon correctly and note the     */
           /* position for use during WM_TIMER processing.              */

           if (!InstData->Dragging)   /* Ignore if not dragging */
             break;

           DMLBCheckTargetLocation(hwnd, InstData, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));

           /* Set pointer icon appropriate for location */
           if (InstData->DragMIcon == NULLHANDLE) {
             // Load all the pointers (one time only)
             InstData->DragMIcon= WinLoadPointer(HWND_DESKTOP, InstData->ResHMod, ID_DMLB_DRAGMOVE);
             InstData->DragCIcon= WinLoadPointer(HWND_DESKTOP, InstData->ResHMod, ID_DMLB_DRAGCOPY);
             InstData->DragNoDrp= WinLoadPointer(HWND_DESKTOP, InstData->ResHMod, ID_DMLB_DRAGNONE);
             InstData->NorthIcon= WinLoadPointer(HWND_DESKTOP, InstData->ResHMod, ID_DMLB_DRGNORTH);
             InstData->SouthIcon= WinLoadPointer(HWND_DESKTOP, InstData->ResHMod, ID_DMLB_DRGSOUTH);
             InstData->DeletIcon= WinLoadPointer(HWND_DESKTOP, InstData->ResHMod, ID_DMLB_DRGDEL);
           }
           switch (InstData->PrevLocation) {
             case POINT_INSIDE:
               switch (InstData->TargetDropMode) {
               case DROPMODE_MOVE:
                 WinSetPointer(HWND_DESKTOP, InstData->DragMIcon); // Use MOVE pointer 
                 break;
               case DROPMODE_COPY:
                 WinSetPointer(HWND_DESKTOP, InstData->DragCIcon); // Use COPY pointer
                 break;
               case DROPMODE_DELETE:
                 WinSetPointer(HWND_DESKTOP, InstData->DeletIcon); // Use DELETE pointer
                 break;
               }
               break;
             case POINT_OUTSIDE:
               WinSetPointer(HWND_DESKTOP, InstData->DragNoDrp);   // No-drop pointer
               break;
             case POINT_NORTH:
               WinSetPointer(HWND_DESKTOP, InstData->NorthIcon);   // Scroll-up pointer
               break;
             case POINT_SOUTH:
               WinSetPointer(HWND_DESKTOP, InstData->SouthIcon);   // Scroll-down poineter
               break;
           }
 
           return (MRESULT)TRUE;  /* Note we processed the message */

      case WM_BEGINDRAG:
           {
           SHORT  Max;
           SHORT i, CursorIndx, hit;

           /* User started dragging with the pointer on our window */

           Max = (SHORT)WinSendMsg( hwnd, LM_QUERYITEMCOUNT, 0L, 0L );

           /* If we are currently dragging, cancel it (should not happen) */
 
           if ( InstData->Dragging ) {
             InstData->Dragging = FALSE;
             WinSetCapture( HWND_DESKTOP, NULLHANDLE );
             return (MRESULT)FALSE;
           }

           /* Get index of item under the mouse pointer and check */
           /* for reasonable numeric bounds.                      */
 
           CursorIndx = DMLBLocateListboxItem(hwnd, hwnd, SHORT2FROMMP(mp1), LLI_UNDER);
           if ((Max == 0) || (CursorIndx+1 > Max)) {
             DosBeep( 440L, 50L );  // Don't allow drag if not on a listbox item
             return (MRESULT)FALSE;
           }

           /* Since we currently support dragging only a single item, */
           /* de-select all items and just select the one under the   */
           /* pointer.  To support multiple-drag we would probably    */
           /* need to notify the owner so they could set the selection*/
           /* status of all items to be dragged (which may or may not */
           /* include the item under the pointer).                    */

           WinSendMsg(hwnd, LM_SELECTITEM, MPFROMSHORT(LIT_NONE), MPVOID);
           WinSendMsg(hwnd, LM_SELECTITEM, MPFROMSHORT(CursorIndx), MPFROMSHORT(TRUE));

           /* Note we are now dragging and capture the pointer. */
 
           InstData->Dragging = TRUE;
           WinSetCapture( HWND_DESKTOP, hwnd );
           InstData->PrevLocation = POINT_INSIDE;
           WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,DRAG_TIMERID, WinQuerySysValue(HWND_DESKTOP, SV_SCROLLRATE));
           return (MRESULT)TRUE;
           break;
           }
 
      case WM_ENDDRAG:
            {
            SHORT DropIndx, CurrIndx;
            SHORT SourceMax, TargetMax;      /* Num of items in source/target listbox */
            char  *CopyText;                 /* Text to be copied/moved */
            USHORT CopyTextLen;              /* Length of text */
            void   *CopyHand;                /* Handle of item to be copied/moved */
            BOOL  SameList = FALSE;          /* Source and target are same listbox */
 
            if (!InstData->Dragging)         /* Ignore if we are not dragging */
              return (MRESULT)FALSE;
 
            /* Clear dragging indicators and release pointer */

            InstData->Dragging = FALSE;
            WinSetCapture( HWND_DESKTOP, NULLHANDLE );
            WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,DRAG_TIMERID);

            /* See if what is under the pointer will accept the drop */
            DMLBCheckTargetLocation(hwnd, InstData, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
            if (InstData->PrevLocation != POINT_INSIDE)
              return (MRESULT)TRUE;  /* Ignore drop outside a good listbox */

            if (hwnd == InstData->TargetHwnd)  // Source and target are same listbox
              SameList = TRUE;

            SourceMax = (SHORT)WinSendMsg(hwnd, LM_QUERYITEMCOUNT, 0L, 0L ) -1;
            TargetMax = (SHORT)WinSendMsg(InstData->TargetHwnd, LM_QUERYITEMCOUNT, 0L, 0L ) -1;

            /* Get drop point and original selected point */

            DropIndx = DMLBLocateListboxItem(InstData->TargetHwnd, hwnd, SHORT2FROMMP(mp1), LLI_INSERTPOINT);
            CurrIndx = (SHORT)WinSendMsg(hwnd, LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), 0L);

            /* Prevent move onto same item as source, in same listbox           */
            /* being careful of DropIndx > SourceMax when drop after last item. */
            if ((InstData->TargetDropMode==DROPMODE_MOVE) &&
                (SameList) &&
                ((min(DropIndx,SourceMax) == CurrIndx) || (DropIndx == CurrIndx+1))) {
              DosBeep( 700L, 50L );  /* Don't drop before or after original */
              return (MRESULT)TRUE;
            }

            /* Make a copy of original to insert */

            CopyTextLen = (SHORT)WinSendMsg(hwnd,LM_QUERYITEMTEXTLENGTH,MPFROMSHORT(CurrIndx), 0L) + 1;
            CopyText = malloc(CopyTextLen);
            WinSendMsg(hwnd, LM_QUERYITEMTEXT, MPFROM2SHORT(CurrIndx, CopyTextLen), MPFROMP(CopyText));
            CopyHand = WinSendMsg(hwnd, LM_QUERYITEMHANDLE, MPFROMSHORT(CurrIndx), 0L);

            /* Insert before insertion point, or at end of list */
            if (DropIndx > TargetMax)
              DropIndx = LIT_END;

            /* Disable update during insert/delete for smoother visual and */
            /* prevent ownerdraw from occuring before new handles are set. */

            WinEnableWindowUpdate(hwnd, FALSE);
            WinEnableWindowUpdate(InstData->TargetHwnd, FALSE);
            
            /* Insert into target list */
            if (InstData->TargetDropMode != DROPMODE_DELETE) {
              DropIndx = (SHORT)WinSendMsg(InstData->TargetHwnd, LM_INSERTITEM, MPFROMSHORT(DropIndx), MPFROMP(CopyText));
              WinSendMsg(InstData->TargetHwnd, LM_SETITEMHANDLE, MPFROMSHORT(DropIndx), MPFROMP(CopyHand));
            }
            free(CopyText);

            /* Tell owner of originating listbox what we are doing.  We must notify */
            /* the owner before we delete items because they may keep dynamic data  */
            /* in the item handles that has to be freed.  The item in question is   */
            /* the currently selected item in the listbox.                          */

            switch (InstData->TargetDropMode) {
              case DROPMODE_MOVE:
                 if (!SameList)
                   WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                           MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), LN_DMLB_DELETE_MOVE),
                           MPFROMHWND(InstData->TargetHwnd));
                 break;
              case DROPMODE_DELETE:
                 WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                           MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), LN_DMLB_DELETE),
                           MPFROMHWND(InstData->TargetHwnd));
                 break;
            }

            /* If this is a move, delete original.  If it is in the same  */
            /* listbox as target, get new index since it may have         */
            /* changed due to inserted copy.                              */

            if ((InstData->TargetDropMode == DROPMODE_MOVE) || (InstData->TargetDropMode == DROPMODE_DELETE)) {
              CurrIndx = (SHORT)WinSendMsg(hwnd, LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), 0L);
              WinSendMsg(hwnd, LM_DELETEITEM, MPFROMSHORT(CurrIndx), 0L);
            }

            /* Select the newly inserted item.  If the old copy was             */
            /* above it in the same list, then the item number has changed by 1 */

            if ((DropIndx > CurrIndx) && (SameList) && (InstData->TargetDropMode==DROPMODE_MOVE))
              DropIndx--;
            if (InstData->TargetDropMode != DROPMODE_DELETE) {
              WinSendMsg(InstData->TargetHwnd, LM_SELECTITEM, MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE));
              WinSendMsg(InstData->TargetHwnd, LM_SELECTITEM, MPFROMSHORT(DropIndx), MPFROMSHORT(TRUE));
            }

            WinEnableWindowUpdate(hwnd, TRUE);
            WinEnableWindowUpdate(InstData->TargetHwnd, TRUE);

            /* Notify target of inserted items if necessary */

            switch (InstData->TargetDropMode) {
              case DROPMODE_MOVE:
                if (SameList)
                   WinSendMsg(WinQueryWindow(InstData->TargetHwnd, QW_OWNER), WM_CONTROL,
                         MPFROM2SHORT(WinQueryWindowUShort(InstData->TargetHwnd, QWS_ID), LN_DMLB_REORDERED),
                         MPFROMHWND(hwnd));
                else
                   WinSendMsg(WinQueryWindow(InstData->TargetHwnd, QW_OWNER), WM_CONTROL,
                         MPFROM2SHORT(WinQueryWindowUShort(InstData->TargetHwnd, QWS_ID), LN_DMLB_INSERT_MOVE),
                         MPFROMHWND(hwnd));
                break;
              case DROPMODE_COPY:
                WinSendMsg(WinQueryWindow(InstData->TargetHwnd, QW_OWNER), WM_CONTROL,
                         MPFROM2SHORT(WinQueryWindowUShort(InstData->TargetHwnd, QWS_ID), LN_DMLB_INSERT_COPY),
                         MPFROMHWND(hwnd));
                break;
            }

            return (MRESULT)TRUE;
            }
 
      }
 
   /* Call previous window procedure to process this message */
   return ( (*(InstData->OldProcAddr)) ( hwnd, msg, mp1, mp2 )  );
   }


/*----------------------------------------------------------------------*/
SHORT DMLBLocateListboxItem( HWND hwnd, HWND RelHwnd, SHORT Y, SHORT Option )
/*----------------------------------------------------------------------*/
/* Parameters:                                                          */
/*   hwnd         Listbox of interest                                   */
/*   RelHwnd      Window to which Y is relative to                      */
/*   Y            Y position of the pointer                             */
/*   Option       LLI_UNDER to return index of item under pointer,      */
/*                LLI_INSERTPOINT to index of item nearest insert point */
/*----------------------------------------------------------------------*/
/* Given a Y window coordinate, this function will calculate the item   */
/* number of the listbox item at that position.  The LLI_UNDER option   */
/* will return the item directly under the pointer.  The LLI_INSERTPOINT*/
/* option will return the item number *before* which an insertion should*/
/* be made (drop zones are calculated as the half-way points through    */
/* each item).                                                          */
/*----------------------------------------------------------------------*/
{
   RECTL   Rect;
   POINTL  Points[2];
   HPS     hps;
   LONG    VertSize;
   SHORT   ItemNum;
   char    ClassName[10];

   /* Map coordinate from window it is relative to, to win of interst*/
   Points[0].x = 0;
   Points[0].y = Y;
   WinMapWindowPoints(RelHwnd, hwnd, &(Points[0]), 1L);
   Y = Points[0].y;

   /* If this is actually a MultiColumn ListBox, we must find the    */
   /* real listbox which is the first column and use it for our      */
   /* location calculations.  Since we only care about the Y         */
   /* coordinate we only need to look at the 1st column of the MCLB. */

   WinQueryClassName(hwnd, sizeof(ClassName), ClassName);
   if (!strcmp("MCLBCls", ClassName))
     hwnd = WinWindowFromID(hwnd, 1);   /* Get handle of 1st column's listbox */

   /* If this is an OWNERDRAW listbox, ask owner how big items are.  */

   VertSize = 0;
   if (WinQueryWindowULong(hwnd, QWL_STYLE) & LS_OWNERDRAW)
     VertSize = SHORT1FROMMR(                               // Take returned Height
                WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),  // Query owner of listbox
                   WM_MEASUREITEM,                          // Ask owner the size
                   MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),  // Listbox ID
                   MPFROMSHORT(0)));                        // First item

   if (VertSize == 0) {
     /* For a normal listbox, items are the size of the font.  To */
     /* determine the size we get the bounding box of a space.    */
     hps = WinGetPS(hwnd);
     GpiQueryTextBox(hps, 1L, " ", 2, Points);
     VertSize = Points[TXTBOX_TOPLEFT].y - Points[TXTBOX_BOTTOMLEFT].y;
     WinReleasePS(hps);
   }

   WinQueryWindowRect(hwnd, &Rect);
   Rect.yTop = Rect.yTop-2;         /* Listbox frame is 2 pixels */

   /* Calculate item number of item under the pointer */
   ItemNum = (SHORT)WinSendMsg(hwnd, LM_QUERYTOPINDEX, 0L, 0L)
             + ((Rect.yTop-Y)/VertSize);

   /* Return item under pointer, or insertion point */

   switch (Option) {
     case LLI_UNDER:
       return ItemNum;
     case LLI_INSERTPOINT:
       if (((Rect.yTop-Y) % VertSize) <= (VertSize / 2))
         return ItemNum;
       return ItemNum+1;  /* Note: May exceed num of items in list */
   }
}


/*----------------------------------------------------------------------*/
void DMLBCheckTargetLocation(HWND SrcHwnd, DMLBData *InstData, SHORT X, SHORT Y)
/*----------------------------------------------------------------------*/
/* X,Y is in SrcHwnd window coordinates.                                */
/* Given an Y,Y coordinate of the pointer, determine if the pointer is  */
/* over a listbox that will accept a drop.  If it is, we save the       */
/* handle of the listbox and set the PrevLocation to POINT_INSIDE in    */
/* the instance data.  Otherwise, we see if the position is near the    */
/* top/bottom edge of the last good target listbox we had.  If so, set  */
/* the PrevLocation to POINT_NORTH/SOUTH as appropriate.  Finally, if   */
/* none of the above, just set the PrevLocation to POINT_OUTSIDE.       */
/*----------------------------------------------------------------------*/
{
RECTL Rect;
POINTL ScreenPoint;
HWND  OverHwnd;
char  OverClass[10];

  InstData->PrevLocation = POINT_OUTSIDE; // Assume this until we know otherwise

  /* See if window under the pointer is a listbox.             */
  ScreenPoint.x = X;
  ScreenPoint.y = Y;
  WinMapWindowPoints(SrcHwnd, HWND_DESKTOP, &ScreenPoint, 1L);
  OverHwnd = WinWindowFromPoint(HWND_DESKTOP, &ScreenPoint, TRUE);
  WinQueryClassName(OverHwnd, sizeof(OverClass), OverClass);

  if (!strcmp(OverClass, LISTBOX_CLASS_STRING)) {
    /* Yes, it is a listbox -- see if it will accept our drop */
    MRESULT Answer;

    /* If this is part of an MCLB, the MCLB is really the target */
    WinQueryClassName(WinQueryWindow(OverHwnd, QW_OWNER), sizeof(OverClass), OverClass);
    if (!strcmp(OverClass, "MCLBCls"))
      OverHwnd = WinQueryWindow(OverHwnd, QW_OWNER);

    Answer = WinSendMsg(WinQueryWindow(OverHwnd, QW_OWNER),
                        WM_CONTROL,
                        MPFROM2SHORT(WinQueryWindowUShort(OverHwnd, QWS_ID), LN_DMLB_QRYDROP),
                        MPFROMHWND(SrcHwnd));
    if ((BOOL)SHORT1FROMMR(Answer)) {
      /* Yes, it will accept the drop */
      InstData->TargetHwnd = OverHwnd;                 // Note new target
   // if (SrcHwnd == OverHwnd)
   //   InstData->TargetDropMode = DROPMODE_MOVE;      // Force MOVE for same-listbox drops
   // else
        InstData->TargetDropMode = SHORT2FROMMR(Answer); // Copy, Move, or Delete
      InstData->PrevLocation = POINT_INSIDE;           // We are inside the listbox now
      return;
    }
    /* It will not accept a drop, proceed as point outside */
    return;
  }

  /* Not a listbox class window.  See if pointer is near north/south */
  /* edge of last good target listbox.  ("Near" is defined as the    */
  /* height of a menu bar).                                          */

  WinQueryWindowRect(InstData->TargetHwnd, &Rect);
  /* Translate rectangle to screen coordinates */
  WinMapWindowPoints(InstData->TargetHwnd, HWND_DESKTOP, (POINTL *)&Rect, 2L);

  if ((ScreenPoint.x >= Rect.xLeft) && (ScreenPoint.x <= Rect.xRight)) {
    if ((ScreenPoint.y < Rect.yTop + WinQuerySysValue(HWND_DESKTOP, SV_CYMENU)) &&
        (ScreenPoint.y > Rect.yBottom - WinQuerySysValue(HWND_DESKTOP, SV_CYMENU))) {
       if (ScreenPoint.y > Rect.yTop)
         InstData->PrevLocation = POINT_NORTH;
       else if (ScreenPoint.y < Rect.yBottom)
         InstData->PrevLocation = POINT_SOUTH;
       return;
    } // Within a menu-size distance of north/south edge
  } // Within the horizontal boundries of the listbox

  /* Pointer is outside the range of interest */
  return;
}

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
/*------- Multi-Column ListBox (MCLB) custom PM control ------------*/
/*                                                                  */
/* Author:            Mark McMillan                                 */
/*                    IBM Corp.                                     */
/*                    Research Triangle Park, NC                    */
/*                    USA                                           */
/*                                                                  */
/* Original Concepts: Charles Cooper                                */
/*                    Warwick Development Group                     */
/*                    IBM UK Ltd., PO Box 31, Birmingham Road       */
/*                                                                  */
/* Copyright (c) IBM Corp. 1995                                     */
/*------------------------------------------------------------------*/

/******************************************************************************/
/* CHANGES                                                                    */
/*   July 1995  : Complete rewrite                                            */
/*   Aug 04 1995: Added "Size" field to all structures passed on              */
/*                the WinCreateWindow() API.                                  */
/*   Aug 17 1995: Added MCLBS_CUASELECT style to de-select all items on left  */
/*                mouse click like the container does.                        */
/*   Sep 15 1995: Fixed handling of LM_SELECTITEM with mp1=LIT_NONE           */
/*   Oct 09 1995: Allow null InitSizes in MCLBINFO during create.  All column */
/*                sizes will be equal.                                        */
/******************************************************************************/

#define INCL_BASE
#define INCL_PM
#define INCL_WINSTDDRAG

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

/*--- Macros ---*/
#define QUERYSELECTION(ListHwnd, Index) (SHORT)WinSendMsg(ListHwnd, LM_QUERYSELECTION, MPFROMSHORT(Index), MPVOID)

/*--- Constants ---*/
#define MCLB_CLASS     "MCLBCls"        /* Class name of MCLB control  */
#define MCLB_CLASS_SEP "MCLBSep"        /* Class name of separators    */
#define MCLB_CLASS_FRAME "MCLBFrm"      /* Class name of frame window  */
#define RESERVED_SIZE  sizeof(PVOID)*2  /* Size of MCLBCls reserved window area */
#define MAX_INT        32767            /* Max short integer */
#define MCLB_FRAME_ID  MAX_INT          /* ID of frame window */
#define TITLEMARGIN_LEFT   1            /* Margin on title text */
#define TITLEMARGIN_RIGHT  1

/* Internally used messages -- must not collide with MCLB messages in MCLB.H  */
#define MSG_PROCESS_SCROLL (WM_USER+500) /* Process an LN_SCROLL notification */
#define MSG_FOCUSCHILD     (WM_USER+501) /* Set focus to a child window       */

typedef struct _GENERICLIST {           /* Generic linked-list structure */
  ULONG  Size;                          /* Structures start with size of data */
  struct _GENERICLIST *Next, *Prev;     /* All must have links as next 2 elements */
  } GENERICLIST;

#include "mclb.h"                       /* Externalized definitions */

/* Internal definitions */

typedef struct _MCLBINSTDATA MCLBINSTDATA;
typedef struct _MCLBCOLDATA  MCLBCOLDATA;

struct _MCLBINSTDATA {                  /* Per-MCLB instance data                                  */
  ULONG  Size;                          /* Size (reqd since ptr is passed in WinCreateWindow())    */
  MCLBCOLDATA *ColList;                 /* Head of linked list of per-column data                  */
  char   *Title;                        /* Title strings                                           */
  char   TitleFont[MAX_FONTLEN];        /* Title font (null for default system font)               */
  char   ListFont[MAX_FONTLEN];         /* List  font (null for default system font)               */
  ULONG  TitleBColor;                   /* Title background color                                  */
  ULONG  TitleFColor;                   /* Title foreground color                                  */
  ULONG  ListBColor;                    /* List  background color                                  */
  ULONG  ListFColor;                    /* List  foreground color                                  */
  char   TabChar;                       /* Data column separator character                         */
  char   _Padd[3];                      /* Padding for TabChar to 4 bytes                          */
  HWND   Parent, Owner;                 /* Parent and owner windows of MCLB                        */
  HWND   MCLBHwnd;                      /* Control window handle                                   */
  HWND   Frame;                         /* Title/frame window handle                               */
  HWND   FocusChild;                    /* Last child window to have the focus                     */
  ULONG  Style;                         /* MCLB + LS_* style flags                                 */
  LONG   ListBoxCy;                     /* Vertical size of listboxes, in pixels                   */
  LONG   UsableCx;                      /* Usable width of window (cx - 1 scrollbar - separators)  */
  BOOL   InControl;                     /* Currently processing a WM_CONTROL message               */
  BOOL   PPChanging;                    /* Currently changing all listboxs presentation params.    */
  BOOL   AdjustingSize;                 /* Currently adjusting window size                         */
  BOOL   Initializing;                  /* Currently creating/initializing the MCLB.               */
  USHORT Cols;                          /* Number of columns                                       */
  USHORT Id;                            /* Control ID                                              */
  USHORT ControlCol;                    /* ID of column during WM_CONTROL processing               */
};

struct _MCLBCOLDATA {                   /* Per-column data, linked list element                    */
  ULONG  Size;                          /* Length of this structure                                */
  struct _MCLBCOLDATA *Next, *Prev;     /* Forward/backward list pointers                          */
  struct _MCLBCOLDATA *RightCol;        /* Ptr to col that follows visually, left to right         */
  MCLBINSTDATA *InstData;               /* Ptr to main MCLB instance data                          */
  HWND   BoxHwnd;                       /* Listbox control window handle                           */
  HWND   SepHwnd;                       /* Separator to right of this column                       */
  PFNWP  ListBoxProc;                   /* Original listbox window procedure                       */
  char   *Title;                        /* Column title string                                     */
  LONG   CurrSize;                      /* Current size (pixels) of this column                    */
  ULONG  ColStyle;                      /* Column style (not currently used, could be used for non-sizable cols) */
  ULONG  ColFColor;                     /* Column color (not currently used)                       */
  ULONG  ColBColor;                     /* Column color (not currently used)                       */
  BOOL   InScroll;                      /* Currently processing LN_SCROLL message                  */
  USHORT ColNum;                        /* Column number (sequential), 1-based  -- also window ID of listbox */
};

/* Internal function prototypes */

BOOL _Optlink    MCLBRegisterClasses(HAB Hab);
void _Optlink    MCLBCreateColumn(HWND MCLBHwnd, ULONG Style, MCLBCOLDATA *ColData);
void _Optlink    MCLBSizeChildren(MCLBINSTDATA *InstData, ULONG Cx, ULONG Cy);
PVOID _Optlink   MCLBListInsert(PVOID Element, ULONG Size);
void _Optlink    MCLBListDelete(PVOID Element);
LONG _Optlink    MCLBColorIndex(HPS Hps, LONG RGBColor);
void _Optlink    MCLBTracking(HWND hwnd, MCLBCOLDATA *LColData, USHORT Option);
void _Optlink    MCLBSubStr(char *Source, char *Target, int WordNum, char Delim);
void _Optlink    MCLBSelectAllColumns(MCLBINSTDATA *InstData, HWND SourceHwnd, SHORT Index, BOOL Select);
SHORT _Optlink   MCLBLocateListboxItem( HWND hwnd, SHORT Y);

/* Window procedures */
MRESULT EXPENTRY MCLBMainProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY MCLBFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY MCLBSepProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY MCLBListBoxSubclassProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


/******************************************************************************/
PVOID _Optlink MCLBListInsert(PVOID Element, ULONG Size)
/******************************************************************************/
/* Generic linked-list insert routine.  Given an element of a double-linked   */
/* list, a new element will be inserted after it.  The new element will be    */
/* allocated to the size specified.  The first two pointers of the data       */
/* element are used for forward and backward pointers.  The remainder of the  */
/* new element is initialized to binary zeros.                                */
/******************************************************************************/
{
GENERICLIST *New, *Current;

  Current = (GENERICLIST *)Element;  /* Avoid lots of typcasting */

  /* Allocate new element of specified size and zero it */
  New = malloc(Size);
  memset(New, 0, Size);

  /* Insert new element after current element */
  New->Size = Size;           /* Initialize first ULONG to size of element */
  New->Next = Current->Next;  /* Fixup forward links */
  Current->Next = New;
  if (New->Next != NULL)      /* Fixup backward links */
    (New->Next)->Prev = New;
  New->Prev = Current;

  return New;
}

/******************************************************************************/
void _Optlink MCLBListDelete(PVOID Element)
/******************************************************************************/
/* Generic linked-list delete routine.  This routine will delete the element  */
/* of the linked list specified.  The storage for the element will be freed.  */
/* Note that there is no head-pointer fixup -- it is assumed the first node   */
/* of the list is never deleted.  This simplifies the parameters and code at  */
/* the slight expense of extra storage for one unused list element.           */
/******************************************************************************/
{
GENERICLIST *Current;

  Current = (GENERICLIST *)Element;  /* Avoid lots of typcasting */

  /* Fixup forward links -- no need to test for head of list */
  (Current->Prev)->Next = Current->Next;

  /* Fixup reverse links -- check for end of list */
  if ((Current->Next) != NULL)
    (Current->Next)->Prev = Current->Prev;
   
  free(Current);
}

/*****************************************************************************/
HWND EXPENTRY MCLBCreateWindow(
                HWND      Parent,   // Parent window
                HWND      Owner,    // Owner to recv messages
                ULONG     Style,    // Style flags (MCLBS_*)
                LONG      x,        // Window position
                LONG      y,
                LONG      cx,       // Window size
                LONG      cy,
                HWND      Behind,   // Place behind this window
                USHORT    Id,       // Window ID
                MCLBINFO  *Info)    // MCLB create structure
/*****************************************************************************/
/* This procedure creates the MCLB custom control window.                    */
/*****************************************************************************/
{
  /* Register classes we need with PM */
  if (!MCLBRegisterClasses(WinQueryAnchorBlock(Owner)))
    return NULLHANDLE;

  /* Do some sanity checking before creating the window. */
  if ((Info == NULL) ||            // Bad create pointer
      (Info->Cols < 1) ||          // No columns
      (Info->Titles == NULL) ||    // No titles
      (Info->Size != sizeof(MCLBINFO))) // Incorrect structure size
      return NULLHANDLE;

  /* Fixup some style flags for consistency */
  if (Style & (MCLBS_CUASELECT | LS_MULTIPLESEL | LS_EXTENDEDSEL))
    Style = Style | (LS_MULTIPLESEL | LS_EXTENDEDSEL);

  /* Create the main control window.  It will do all the */
  /* initialization work during WM_CREATE.               */

  return WinCreateWindow(Parent,          // Parent of this control
                         MCLB_CLASS,      // Our custom class
                         "",              // No title
                         Style,           // Our style flags
                         x,y,             // Position
                         cx,cy,           // Size
                         Owner,           // Owner for messages
                         Behind,          // Z order
                         Id,              // ID
                         Info,            // Ctrl data (mp2 in WM_CREATE)
                         NULL);           // No pres params
}

/*****************************************************************************/
BOOL _Optlink MCLBRegisterClasses(HAB Hab)
/*****************************************************************************/
/* Register classes needed for the MCLB control.  Note we must always        */
/* register the class, even if it already exists.  Otherwise an app using    */
/* this control in a DLL may free and reload the DLL and the class           */
/* window proc pointer would be out of date.  Reregistration will replace    */
/* existing class parms.                                                     */
/*****************************************************************************/
{
  WinRegisterClass(Hab,                           
                   MCLB_CLASS_SEP,     // Separator windows
                   (PFNWP)MCLBSepProc,
                   CS_SIZEREDRAW,
                   sizeof(PVOID));     // 1 ptr in window words

  WinRegisterClass(Hab,                           
                   MCLB_CLASS_FRAME,   // Frame/title window
                   (PFNWP)MCLBFrameProc,
                   CS_SIZEREDRAW,
                   sizeof(PVOID));     // 1 ptr in window words

  WinRegisterClass(Hab,
                   MCLB_CLASS,         // Main control window
                   (PFNWP)MCLBMainProc,
                   CS_CLIPCHILDREN|CS_SIZEREDRAW,
                   RESERVED_SIZE);
  return TRUE;
}


/*****************************************************************************/
MRESULT EXPENTRY MCLBMainProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
/*****************************************************************************/
/* Main window procedure for the MCLB class.  This window does no painting,  */
/* all painting is done by child windows which are sized/positioned to cover */
/* all of the window area.                                                   */
/*****************************************************************************/
{
MCLBINSTDATA *InstData;

  /* Our inst data is +1 ptr in the window words, first ptr */
  /* is reserved for the application to use.                */

  InstData = WinQueryWindowPtr(hwnd, sizeof(PVOID));

  switch (msg) {

    case WM_CREATE: {
      MCLBINFO *Info;                  // Ptr to create data from caller
      MCLBCOLDATA *ColData, *PrevCol;  // Per-column data
      int      i;
      char     *CurrTtl;               // For parsing title strings
      SWP      Pos;

      /* Create and initialize the instance data */
      InstData = malloc(sizeof(MCLBINSTDATA));
      WinSetWindowPtr(hwnd, sizeof(PVOID), InstData);

      memset(InstData, 0x00, sizeof(MCLBINSTDATA));
      /* Copy create parameters to the instance data */
      Info = (MCLBINFO *)mp1;
      InstData->Size        = sizeof(MCLBINSTDATA);
      InstData->Initializing= TRUE;
      InstData->MCLBHwnd    = hwnd;
      InstData->Cols        = Info->Cols;
      InstData->TabChar     = Info->TabChar;
      InstData->Id          = WinQueryWindowUShort(hwnd, QWS_ID);
      InstData->Style       = WinQueryWindowULong(hwnd, QWL_STYLE);
      InstData->Parent      = WinQueryWindow(hwnd, QW_PARENT);
      InstData->Owner       = WinQueryWindow(hwnd, QW_OWNER);
      InstData->TitleBColor = Info->TitleBColor;
      InstData->TitleFColor = Info->TitleFColor;
      InstData->ListBColor  = Info->ListBColor;
      InstData->ListFColor  = Info->ListFColor;
      strcpy(InstData->TitleFont, Info->TitleFont);
      strcpy(InstData->ListFont,  Info->ListFont);

      /* Create dummy head-of-list item */
      InstData->ColList = malloc(sizeof(MCLBCOLDATA));
      memset(InstData->ColList, 0x00, sizeof(MCLBCOLDATA));
      InstData->ColList->Size = sizeof(MCLBCOLDATA);

      CurrTtl = malloc(strlen(Info->Titles)+1);  // Space to extract titles

      /* Create linked list of per-column data.  We use a linked */
      /* list to make it easier to add column insert/delete      */
      /* capabilites later (not currently implemented).          */

      ColData = InstData->ColList;
      for (i=0; i<InstData->Cols; i++) {
        PrevCol = ColData;                                      // Note previous column
        ColData = MCLBListInsert(ColData, sizeof(MCLBCOLDATA)); // Create linked list node
        if (Info->InitSizes != NULL)
          ColData->CurrSize = Info->InitSizes[i];               // Caller set initial size
        else
          ColData->CurrSize = 100;                              // Make all same size
        ColData->ColNum   = i+1;  // 1-based column number      // Column num is also window ID
        ColData->InstData = InstData;                           // Save ptr to inst data also
        MCLBSubStr(Info->Titles, CurrTtl, i+1, InstData->TabChar);
        ColData->Title    = strdup(CurrTtl);                    // Save title of this column

        /* Update prev columns RightCol ptr to new column */
        PrevCol->RightCol  = ColData;                           // Create forward node links
        MCLBCreateColumn(hwnd, InstData->Style, ColData);       // Create the column window(s)

        /* Set column pres parameters if any supplied */
        if ((InstData->ListFont)[0] != '\0')
          WinSetPresParam(ColData->BoxHwnd, PP_FONTNAMESIZE, strlen(InstData->ListFont)+1, InstData->ListFont);
        if (InstData->ListFColor != InstData->ListBColor) {
          WinSetPresParam(ColData->BoxHwnd, PP_FOREGROUNDCOLOR, sizeof(LONG), &(InstData->ListFColor));
          WinSetPresParam(ColData->BoxHwnd, PP_BACKGROUNDCOLOR, sizeof(LONG), &(InstData->ListBColor));
        }

      }
      free(CurrTtl);

      /* Create window for drawing title area and set presentation parameters. */
      InstData->Frame = WinCreateWindow(hwnd,   // Parent of this control
                         MCLB_CLASS_FRAME,// Our custom class
                         "",              // No title
                         WS_VISIBLE|WS_CLIPSIBLINGS, // Style flags
                         0,0,0,0,         // Position
                         hwnd,            // Owner for messages
                         HWND_BOTTOM,     // Z order (under all other children)
                         MCLB_FRAME_ID,   // ID
                         InstData,        // Ctrl data (mp2 in WM_CREATE)
                         NULL);           // No pres params

      if ((InstData->TitleFont)[0] != '\0')
        WinSetPresParam(InstData->Frame, PP_FONTNAMESIZE, strlen(InstData->TitleFont)+1, InstData->TitleFont);

      if (InstData->TitleFColor != InstData->TitleBColor) {
        WinSetPresParam(InstData->Frame, PP_FOREGROUNDCOLOR, sizeof(LONG), &(InstData->TitleFColor));
        WinSetPresParam(InstData->Frame, PP_BACKGROUNDCOLOR, sizeof(LONG), &(InstData->TitleBColor));
      }
      else {
        /* Use default colors */
        InstData->TitleBColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOW, 0L);
        InstData->TitleFColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0L);
      }

      /* Finally, size and position the child windows we just created */

      WinQueryWindowPos(hwnd, &Pos);
      MCLBSizeChildren(InstData, Pos.cx, Pos.cy);
      InstData->Initializing = FALSE;
      return (MRESULT)FALSE;
      } // end of WM_CREATE

    case WM_DESTROY: {
      MCLBCOLDATA *CurrCol;

      /* Delete each node in the column list */
      for (CurrCol = InstData->ColList->Next; CurrCol!=NULL; CurrCol=InstData->ColList->Next) {
        free(CurrCol->Title);     // Free title string
        MCLBListDelete(CurrCol);  // Delete node of list, next comes to top of list
      }

      /* Free dummy listhead node and instance data */
      free(InstData->ColList);
      free(InstData);
      break;
      }

    case WM_SIZE: 
      /* MCLB window has been resized, so resize/pos children within it. */
      MCLBSizeChildren(InstData, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));

      break;

    /*-------------------------------------------------------------------------*/
    /* Process WM_CONTROL related messages from listbox children               */
    /*-------------------------------------------------------------------------*/

    case WM_CONTROL: {
      MRESULT Rc;
      /* Received a notification messages from one of the list boxes.*/
      /* We keep the ID (col num) of the listbox in our instance     */
      /* data so it can be queried by the owner during processing    */
      /* of the WM_CONTROL messages we send.  This is the only way   */
      /* for the owner to know which column caused the WM_CONTROL.   */

      /* For most WM_CONTROL messages, we modify mp1 with the ID     */
      /* of the MCLB and mp2 with the window handle so the message   */
      /* appears to be comming from the MCLB window.                 */

      /* If we are currently processing a previous control message,  */
      /* ignore this one (e.g. it was generated by our own actions). */
      if (InstData->InControl)
        return 0;

      InstData->InControl = TRUE; // Note we are now processing a WM_CONTROL
      InstData->ControlCol= SHORT1FROMMP(mp1); // Note column that sent it

      switch (SHORT2FROMMP(mp1)) {
        case LN_ENTER:
        case LN_KILLFOCUS:
        case LN_SETFOCUS:
          /* For these messages, just pass it on to the owner changing the ID to */
          /* the MCLB control ID, and putting our handle in mp2.                 */
          Rc = WinSendMsg(InstData->Owner, WM_CONTROL,
                     MPFROM2SHORT(InstData->Id, SHORT2FROMMP(mp1)),
                     MPFROMHWND(hwnd));
          break;

        case LN_SCROLL:
          /* One of the listboxes is about to scroll (but has not done it yet).  */
          /* We *post* a message to ourselves so we can look at the new scroll   */
          /* position after the scroll is done.  Also pass up to owner.          */
          WinPostMsg(hwnd, MSG_PROCESS_SCROLL, mp1, mp2);
          Rc = WinSendMsg(InstData->Owner, WM_CONTROL,
                     MPFROM2SHORT(InstData->Id, SHORT2FROMMP(mp1)),
                     MPFROMHWND(hwnd));
          break;

        case LN_SELECT: {
          USHORT SelectCol;    // Column of original selection
          SHORT  SelectIndex;  // Selected item
          MCLBCOLDATA *CurrCol;

          /* An item was selected or de-selected in one of the listboxes.  For   */
          /* single-select listboxes, just make the same selection in all the    */
          /* other listboxes.                                                    */
          SelectCol = SHORT1FROMMP(mp1);
          if (!(InstData->Style & (LS_MULTIPLESEL|LS_EXTENDEDSEL))) {
            SHORT TopIndex;    // Top item of selected column

            SelectIndex = (SHORT)WinSendMsg((HWND)mp2, LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), MPVOID);
            TopIndex    = (SHORT)WinSendMsg((HWND)mp2, LM_QUERYTOPINDEX, MPVOID, MPVOID);

            /* Now make same selection in all other listboxes.  Using the LM_SELECTITEM */
            /* message can cause the listbox to scroll, so we reset the top index       */
            /* after each insert.  This does cause a little flashing due to the scroll  */
            /* but it is better than complete disable/repaint of the window.            */
            for (CurrCol = InstData->ColList->Next; CurrCol!=NULL; CurrCol=CurrCol->Next) {
              if (CurrCol->ColNum != SelectCol) {  // Don't select in original column
                WinSendMsg(CurrCol->BoxHwnd, LM_SELECTITEM,  MPFROMSHORT(SelectIndex), MPFROMSHORT(TRUE));
                WinSendMsg(CurrCol->BoxHwnd, LM_SETTOPINDEX, MPFROMSHORT(TopIndex), MPVOID);
              }
            }
          } // end single select
          else {
            /* For multiple select, we must compare two columns -- the one on which a   */
            /* select/deselect was made, and another with the previous selections in it.*/
            /* PM does not tell us what item was selected or deselected, so the only    */
            /* way to find out is to compare columns.  Ugh.                             */
            HWND NewHwnd, OldHwnd;
            SHORT  FindIndex;

            NewHwnd = (HWND)mp2;
            if (SelectCol == 1)  // Locate some other column
              OldHwnd = WinWindowFromID(hwnd, 2);
            else
              OldHwnd = WinWindowFromID(hwnd, 1);

            /* Find all selected items in current column and make sure they are selected */
            /* in the other (old) columns.                                               */
            SelectIndex = QUERYSELECTION(NewHwnd, LIT_FIRST);
            if (SelectIndex == LIT_NONE)    // Easy -- deselect everything
              MCLBSelectAllColumns(InstData, NewHwnd, LIT_NONE, FALSE);
            else while (SelectIndex != LIT_NONE) {
              if (SelectIndex == 0)
                FindIndex = LIT_FIRST;   // Backup one to find in other column
              else
                FindIndex = SelectIndex-1;
              if (QUERYSELECTION(OldHwnd, FindIndex) != SelectIndex) { // Not selected in old cols
                MCLBSelectAllColumns(InstData, NewHwnd, SelectIndex, TRUE);
              }
              SelectIndex = QUERYSELECTION(NewHwnd, SelectIndex);
            }

            /* Search old column for selections not in new column (e.g.    */
            /* this must have been a de-select action).                    */
            SelectIndex = QUERYSELECTION(OldHwnd, LIT_FIRST);
            if (SelectIndex != LIT_NONE) {  // Should always be true
              while (SelectIndex != LIT_NONE) {
                if (SelectIndex == 0)
                  FindIndex = LIT_FIRST;   // Backup one to find in other column
                else
                  FindIndex = SelectIndex-1;
                if (QUERYSELECTION(NewHwnd, FindIndex) != SelectIndex) { // Not selected in new col
                  MCLBSelectAllColumns(InstData, NewHwnd, SelectIndex, FALSE);
                }
                SelectIndex = QUERYSELECTION(OldHwnd, SelectIndex);
              }
            }
          } // if multi-select

          /* Tell owner that a select occured */
          Rc = WinSendMsg(InstData->Owner, WM_CONTROL,
                     MPFROM2SHORT(InstData->Id, SHORT2FROMMP(mp1)),
                     MPFROMHWND(hwnd));

          } // end LN_SELECT processing
          break;

        default:
          /* This a control message we don't recognize, just pass it */
          /* up to the owner to handle.  Note we do not modify mp2   */
          /* since it may be used for something other than the std   */
          /* PM usage.                                               */
          Rc = WinSendMsg(InstData->Owner, WM_CONTROL,
                     MPFROM2SHORT(InstData->Id, SHORT2FROMMP(mp1)),
                     mp2);

      } // switch on LN_* message

      InstData->InControl = FALSE;
      return Rc;

      } // end WM_CONTROL


    case MSG_PROCESS_SCROLL: {
      /* This message is posted from an LN_SCROLL notification from one of the   */
      /* listboxes.  By the time we get this the scrolling is done.  Mp1 and mp2 */
      /* are passed from the original LN_SCROLL.  We now scroll all the listbox  */
      /* controls to match the one that did the scrolling.                       */
      SHORT NewTop;
      HWND  OrigHwnd;
      MCLBCOLDATA *CurrCol;

      OrigHwnd = WinWindowFromID(hwnd, SHORT1FROMMP(mp1));  // Listbox that generated LN_SCROLL

      NewTop = (SHORT)WinSendMsg(OrigHwnd, LM_QUERYTOPINDEX, MPVOID, MPVOID);

      /* Walk list of columns and set each topindex */
      for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {
        if (NewTop != (SHORT)WinSendMsg(CurrCol->BoxHwnd, LM_QUERYTOPINDEX, MPVOID, MPVOID))
          WinSendMsg(CurrCol->BoxHwnd, LM_SETTOPINDEX, MPFROMSHORT(NewTop), MPVOID);
      }
      return 0;
      } // end of MSG_PROCESS_SCROLL

    case WM_MEASUREITEM:
    case WM_DRAWITEM:
      /* These messages are sent from the listbox controls for OWNERDRAW style */
      /* listboxes.  The owner must handle them.  We place the column number   */
      /* in SHORT 2 of mp1 which is unused by PM.                              */

      return WinSendMsg(InstData->Owner, msg, MPFROM2SHORT(InstData->Id, SHORT1FROMMP(mp1)), mp2);

    /*-------------------------------------------------------------------------*/
    /* Process LM_* messages on behalf of listbox children                     */
    /*-------------------------------------------------------------------------*/

    case LM_INSERTITEM: {
      /* Application is inserting an item into the MCLB.  The message symantics */
      /* are extended for MCLB to include the column by which to sort in SHORT2 */
      /* of mp1 (unused by normal LM_INSERTITEM message).  The item text is     */
      /* separated into columns by the tab value specified when the MCLB was    */
      /* first created.                                                         */

      USHORT SortCol;    // Column number we must first insert to get sorting
      SHORT  InsIndex;   // Index at which inserts must be done for other cols
      MCLBCOLDATA *CurrCol;
      char   *TextBuff;  // Buffer to extract text strings
      int    i;

      /* Sanity check */
      SortCol = max(1,SHORT2FROMMP(mp1));    // 1-based column number
      if (SortCol > InstData->Cols)
        return MRFROMSHORT(LIT_ERROR);

      /* Find the sort column.  It is the N'th in the (non-display order) list */
      CurrCol = InstData->ColList->Next;
      for (i=1; i<SortCol; i++)
        CurrCol = CurrCol->Next;

      /* Get string to insert in the sort column */
      TextBuff = malloc(strlen((char *)mp2)+1);
      if (TextBuff == NULL)
        return MRFROMSHORT(LIT_MEMERROR);
      MCLBSubStr((char *)mp2, TextBuff, SortCol, InstData->TabChar);

      /* Insert it in the sort column and note index where it went (or err) */
      /* In Warp, should only fail if >32K entries.                         */
      InsIndex = (SHORT)WinSendMsg(CurrCol->BoxHwnd, LM_INSERTITEM, mp1, MPFROMP(TextBuff));
      if ((InsIndex == LIT_ERROR) || (InsIndex == LIT_MEMERROR)) {
        free(TextBuff);
        return MRFROMSHORT(InsIndex);
      }

      /* Now insert strings into other listboxes at same position */
      for (CurrCol = InstData->ColList->Next; CurrCol!=NULL; CurrCol=CurrCol->Next) {
        if (CurrCol->ColNum != SortCol) { // Don't insert sort column twice!
          /* Get this columns text and insert it */
          MCLBSubStr((char *)mp2, TextBuff, CurrCol->ColNum, InstData->TabChar);
          WinSendMsg(CurrCol->BoxHwnd, LM_INSERTITEM, MPFROMSHORT(InsIndex), MPFROMP(TextBuff));
        }
      }

      /* Cleanup and return index of insertion to caller */
      free(TextBuff);
      return MRFROMSHORT(InsIndex);

      } // end LM_INSERTITEM

    #if defined(LM_INSERTMULTITEMS)  // New listbox msg for Warp 3.0
    case LM_INSERTMULTITEMS: {
      /* Application is inserting multiple items into the MCLB.  This message is   */
      /* a bit of a problem because if sorting is specified then PM will change    */
      /* the item order of (all) the listbox items and we will not be able to      */
      /* keep the other columns in sync.  A performance-poor solution is to sort   */
      /* each entry as it is added (somewhat defeating the purpose of this         */
      /* high-performance multiple-insert message).  A fancier solution might be   */
      /* to set the item handles before the insert to a sequence number, then      */
      /* read them after the sort and discover the new order.  We would have to    */
      /* save/restore the application's item handles somewhere else.  The          */
      /* "real" solution is an MCLB-specific sorting message which is independant  */
      /* of all the INSERT messages.                                               */

      /* So basically, the poor-mans solution is to just call LM_INSERT for each   */
      /* message in the list using the sorting options specified.                  */

      /* Note we use one of the reserved slots in the LBOXINFO structure to spec   */
      /* the sort column.                                                          */

      int    i;
      SHORT  Rc;
      LBOXINFO *MultInfo;   // Ptr to mult-insert info
      char   **MultStrings; // Ptr to array of string pointers
      BOOL   Enabled;       // Window was enabled

      MultInfo = PVOIDFROMMP(mp1);
      MultStrings = PVOIDFROMMP(mp2);

      Enabled = WinIsWindowVisible(InstData->MCLBHwnd);
      if (Enabled)
        WinEnableWindowUpdate(InstData->MCLBHwnd, FALSE);

      for (i=0, Rc=0; ((i<MultInfo->ulItemCount) && (Rc>0)) ; i++) {
        Rc=SHORTFROMMR(WinSendMsg(hwnd, LM_INSERTITEM, MPFROM2SHORT(MultInfo->lItemIndex, MultInfo->reserved2), MPFROMP(MultStrings[i])));
      } /* endfor */

      if (Enabled)
        WinEnableWindowUpdate(InstData->MCLBHwnd, TRUE);

      return Rc;
      #endif  // Warp 3.0

    case LM_SETITEMTEXT: {
      /* Set text of each column from delimited string.  Note order of fields */
      /* is the numerical column order, not the display order.                */
      MCLBCOLDATA *CurrCol;
      char        *Buff;
      MRESULT     Rc;

      Buff = malloc(strlen((char *)mp2)+1);

      for (CurrCol = InstData->ColList->Next; CurrCol!=NULL; CurrCol=CurrCol->Next) {
        MCLBSubStr((char *)mp2, Buff, CurrCol->ColNum, InstData->TabChar);
        Rc = WinSendMsg(CurrCol->BoxHwnd, LM_SETITEMTEXT, mp1, MPFROMP(Buff));
      }
      return Rc;
      }

    case LM_DELETEALL:
    case LM_DELETEITEM:
    case LM_SETITEMHEIGHT:
    case LM_SETTOPINDEX: {
      /* Send these messages directly to each listbox for processing */
      MCLBCOLDATA *CurrCol;
      MRESULT     Rc;

      for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol)
        Rc = WinSendMsg(CurrCol->BoxHwnd, msg, mp1, mp2);
      return Rc;
      }

    case LM_QUERYITEMCOUNT: 
    case LM_QUERYSELECTION: 
    case LM_QUERYITEMHANDLE:
    case LM_QUERYTOPINDEX:  
    case LM_SETITEMHANDLE:  
    case WM_QUERYCONVERTPOS:
      /* These messages can be handled by any of the listboxes, so we use the first */
      return WinSendMsg(InstData->ColList->RightCol->BoxHwnd, msg, mp1, mp2);

    case LM_SELECTITEM: {
      /* Normally this message can be handled by any one listbox (e.g. first), but   */
      /* in the case of mp1=LIT_NONE, PM does not send an LN_SELECT message so other */
      /* columns don't automatically get updated.  In that case we must propagate    */
      /* this message to every listbox.                                              */
      MCLBCOLDATA *CurrCol;
      MRESULT     Rc;
      for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol)
        Rc = WinSendMsg(CurrCol->BoxHwnd, msg, mp1, mp2);
      return Rc;
      }

    case LM_QUERYITEMTEXTLENGTH: {
      /* This message is interpreted to mean the length of all the columns in  */
      /* a given row, including tab separator character.                       */
      MCLBCOLDATA *CurrCol;
      SHORT       Len, Sum;

      Sum = 0;
      for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {
        Len = (SHORT)WinSendMsg(CurrCol->BoxHwnd, LM_QUERYITEMTEXTLENGTH, mp1, mp2);
        if (Len == LIT_ERROR)
          return MRFROMSHORT(LIT_ERROR);
        Sum = Sum + Len + 1;  // Include a tab separator in count
      }
      return MRFROMSHORT(Sum-1);  // Don't count last tab or any trailing null
      }

    case LM_QUERYITEMTEXT: {
      /* Return all the columns text separated with tab chars */
      MCLBCOLDATA *CurrCol;
      SHORT       Len, Max;
      char        *Buff;

      Max = SHORT2FROMMP(mp1);
      Buff= (char *)mp2;

      for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {
        Len = (SHORT)WinSendMsg(CurrCol->BoxHwnd, LM_QUERYITEMTEXT, MPFROM2SHORT(SHORT1FROMMP(mp1),Max), MPFROMP(Buff));
        Buff = Buff + Len;
        Max  = Max - Len;
        if ((Max > 0) && (CurrCol->RightCol != NULL)) {  // Add delimiter if room for it & more to come
          *Buff = InstData->TabChar;
          Buff++;
          Max--;
        }
      }

      return MRFROMSHORT(SHORT2FROMMP(mp1)-Max);  // Len is original minus what is left
      }

    case LM_SEARCHSTRING:
      //... need to fix this to search full MCLB-wide column.  would also be nice
      //... to have MCLB-specific message to search a single specified column.  for
      //... now we just cheat and search the first one.
      return WinSendMsg(InstData->ColList->RightCol->BoxHwnd, msg, mp1, mp2);

    /*-------------------------------------------------------------------------*/
    /* Process MCLB-specific messages                                          */
    /*-------------------------------------------------------------------------*/

    case MCLB_SETTITLES: {
      char *CurrTtl;
      MCLBCOLDATA *CurrCol;
      int  i;

      /* mp1 is pointer to new title string, delimited by tab character */
      /* in column (not display) order.                                 */

      CurrTtl = malloc(strlen((char *)mp1)+1);  // Space to extract titles

      for (CurrCol=InstData->ColList->Next, i=0; CurrCol!=NULL; CurrCol=CurrCol->Next, i++) {
        MCLBSubStr((char *)mp1, CurrTtl, i+1, InstData->TabChar);
        free(CurrCol->Title);              // free old title
        CurrCol->Title = strdup(CurrTtl);  // set new title
      }
      free(CurrTtl);
      return 0;
      }

    case MCLB_SETTITLEFONT: 
      /* mp1 is pointer to font name, or NULL to use default font */
      if ((char *)mp1 == NULL)
        return (MRESULT)WinRemovePresParam(InstData->Frame, PP_FONTNAMESIZE);
      else
        return (MRESULT)WinSetPresParam(InstData->Frame, PP_FONTNAMESIZE, strlen((char *)mp1)+1, (char *)mp1);

    case MCLB_SETTITLECOLORS:
      /* mp1 is forground color, mp2 is background.  If same, use default colors */
      if (mp1 == mp2) {
        WinRemovePresParam(InstData->Frame, PP_FOREGROUNDCOLOR);
        return (MRESULT)WinRemovePresParam(InstData->Frame, PP_BACKGROUNDCOLOR);
      }
      WinSetPresParam(InstData->Frame, PP_FOREGROUNDCOLOR, sizeof(LONG), &mp1);
      return (MRESULT)WinSetPresParam(InstData->Frame, PP_BACKGROUNDCOLOR, sizeof(LONG), &mp2);

    case MCLB_SETLISTFONT:
      /* mp1 is pointer to font name, or NULL to use default font. */
      /* Just set the font on first list, it will set all others.  */
      if ((char *)mp1 == NULL)
        return (MRESULT)WinRemovePresParam(InstData->ColList->RightCol->BoxHwnd, PP_FONTNAMESIZE);
      else
        return (MRESULT)WinSetPresParam(InstData->ColList->RightCol->BoxHwnd, PP_FONTNAMESIZE, strlen((char *)mp1)+1, (char *)mp1);

    case MCLB_SETLISTCOLORS: {
      int i;
      MCLBCOLDATA *CurrCol;
      BOOL Rc;

      /* mp1 is forground color, mp2 is background.  If same, use default colors */
      /* If multi-color style we need to do this on each listbox.                */
      if (InstData->Style & MCLBS_MULTICOLOR)
        i = InstData->Cols;
      else
        i = 1;

      CurrCol = InstData->ColList->RightCol;
      for (i=1; i<=InstData->Cols; i++) {
        if (mp1 == mp2) {
          WinRemovePresParam(CurrCol->BoxHwnd, PP_FOREGROUNDCOLOR);
          Rc=WinRemovePresParam(CurrCol->BoxHwnd, PP_BACKGROUNDCOLOR);
        }
        else {
          WinSetPresParam(CurrCol->BoxHwnd, PP_FOREGROUNDCOLOR, sizeof(LONG), &mp1);
          Rc=WinSetPresParam(CurrCol->BoxHwnd, PP_BACKGROUNDCOLOR, sizeof(LONG), &mp2);
        }
        CurrCol = CurrCol->RightCol;
      }
      return (MRESULT)Rc;
      }

    case MCLB_QUERYCTLCOL:
      /* This is assumed to be sent during the processing of a WM_CONTROL */
      /* message we have sent to the owner.  The owner can SEND us this   */
      /* message to discover which column originated the WM_CONTROL msg.  */
      return MRFROMSHORT(InstData->ControlCol);

    case MCLB_QUERYSTYLE:
      /* Return (only) the MCLB style flags */
      return MRFROMLONG(InstData->Style & MCLBS_MASK);

    case MCLB_QUERYFULLSIZE:
      /* Return width (pixels) available for column data */
      return MRFROMLONG(InstData->UsableCx);

    case MCLB_SETCOLSIZES: {
      /* Set width of each column.  mp1 is ptr to array of LONG.  We assume      */
      /* caller supplies an array with correct number of entries.  After setting */
      /* new column sizes, we call our resize algorithm to update the display    */
      /* and make sure columns fit into available display area.                  */
      MCLBCOLDATA *CurrCol;
      int         i;
      SWP         Pos;

      for (i=0, CurrCol = InstData->ColList->Next; CurrCol!=NULL; i++, CurrCol=CurrCol->Next)
        CurrCol->CurrSize = ((LONG *)mp1)[i];

      WinQueryWindowPos(hwnd, &Pos);
      MCLBSizeChildren(InstData, Pos.cx, Pos.cy);
      return 0;
      }

    case MCLB_QUERYCOLSIZES: {
      /* mp1 is a pointer to an array of LONGs.  We assume there are at least as */
      /* many elements as columns.                                               */
      LONG     *SizeList;
      MCLBCOLDATA *CurrCol;
      int      i;

      SizeList = (LONG *)PVOIDFROMMP(mp1);
      for (i=0, CurrCol = InstData->ColList->Next; CurrCol!=NULL; i++, CurrCol=CurrCol->Next) {
        /* Save this columns width */
        SizeList[i] = CurrCol->CurrSize;
      }

      return 0;
      }

    case MCLB_QUERYINFO: {
      /* mp1 is a pointer to an MCLBINFO structure.  We will fill it in with the */
      /* current MCLB values.  The InitSizes array will be set to the current    */
      /* column sizes, in pixels.  Note sender of this message must free the     */
      /* title string and array of LONG size values.                             */
      MCLBINFO *Info;
      MCLBCOLDATA *CurrCol;
      char     *Title;
      int      i;

      Info = (MCLBINFO *)mp1;

      memset(Info, 0x00, sizeof(MCLBINFO));
      Info->Cols = InstData->Cols;
      Info->TabChar = InstData->TabChar;
      Info->InitSizes = malloc(sizeof(LONG) * InstData->Cols);

      Title = strdup("");
      for (i=0, CurrCol = InstData->ColList->Next; CurrCol!=NULL; i++, CurrCol=CurrCol->Next) {
        /* Append this columns title to title string */
        Title = realloc(Title, strlen(Title)+strlen(CurrCol->Title)+2);  // Get memory for this column title
        strcat(Title, CurrCol->Title);
        strcat(Title, &(InstData->TabChar));
        /* Save this columns width */
        Info->InitSizes[i] = CurrCol->CurrSize;
      }
      Title[strlen(Title)-1] = '\0';  // trim final separator
      Info->Titles = Title;
 
      /* Get presentation parameters which are kept current in the instance data */
      strcpy(Info->TitleFont, InstData->TitleFont);
      strcpy(Info->ListFont, InstData->ListFont);
      Info->TitleFColor = InstData->TitleFColor;
      Info->TitleBColor = InstData->TitleBColor;
      Info->ListFColor  = InstData->ListFColor;  // undefined for MCLBS_MULTICOLOR style
      Info->ListBColor  = InstData->ListBColor;

      return 0;
      }

    /*-------------------------------------------------------------------------*/
    /* Process focus-change related messages                                   */
    /*-------------------------------------------------------------------------*/

    case WM_FOCUSCHANGE:
      /* Us or a child is getting/losing the focus.  Keep track of any children with */
      /* focus so that we can process Alt/Ctl keystrokes properly.  We cannot        */
      /* change focus during this message, so we post a msg to do it later.          */
  
      if ((BOOL)SHORT1FROMMP(mp2)) {               // MCLB is getting the focus
        if (InstData->FocusChild == NULLHANDLE)    // first column is default
          InstData->FocusChild = InstData->ColList->RightCol->BoxHwnd;
        WinPostMsg(hwnd, MSG_FOCUSCHILD, MPVOID, MPVOID);
      }
      break;    // proceed with normal processing
  
    case MSG_FOCUSCHILD:
      /* Message posted from WM_FOCUSCHANGE.  If we still have the focus, set the focus */
      /* to the last child that had it (we never want the focus on the MCLB window      */
      /* itself).                                                                       */
  
      if (WinQueryFocus(HWND_DESKTOP) == hwnd)
        WinSetFocus(HWND_DESKTOP, InstData->FocusChild);
      return 0;


  } // switch on msg

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************/
MRESULT EXPENTRY MCLBFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
/*****************************************************************************/
/* This window procedure is for the frame/title window of the MCLB control.  */
/* This window is sized by the MCLBSizeChildren() function, and it is always */
/* the full size of the control window.  Note this is not a "frame"          */
/* window in the PM-sense, it just provides a visual frame around the MCLB.  */
/*****************************************************************************/
{
MCLBINSTDATA *InstData;

  InstData = WinQueryWindowPtr(hwnd, 0L);

  switch (msg) {
    case WM_CREATE:
      InstData = (MCLBINSTDATA *)mp1;
      WinSetWindowPtr(hwnd, 0L, InstData);
      return (MRESULT)FALSE;

    case WM_BUTTON1DOWN:
      /* Set focus to MCLB window */
      WinSetFocus(HWND_DESKTOP, InstData->MCLBHwnd);
      return (MRESULT)TRUE;

    case WM_PRESPARAMCHANGED:
      if (!InstData->Initializing)   // Skip this when MCLB is being created
      switch (LONGFROMMP(mp1)) {
        case PP_FONTNAMESIZE: {
          SWP Pos;
          if (WinQueryPresParam(hwnd, PP_FONTNAMESIZE, 0L, NULL, MAX_FONTLEN, &InstData->TitleFont, QPF_NOINHERIT) == 0)
            (InstData->TitleFont)[0] = '\0';
          WinQueryWindowPos(InstData->MCLBHwnd, &Pos);
          MCLBSizeChildren(InstData, Pos.cx, Pos.cy);
          WinInvalidateRect(hwnd, NULL, FALSE);
          // Tell owner about the pres parm change
          WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_PPCHANGED),
                                                MPFROM2SHORT(0, MCLBPP_FONT));
          }
          break;
        case PP_BACKGROUNDCOLOR:
          /* Save new color and redraw */
          WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, 0L, NULL, sizeof(LONG), &InstData->TitleBColor, 0);
          WinInvalidateRect(hwnd, NULL, FALSE);
          // Tell owner about the pres parm change
          WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_PPCHANGED),
                                                  MPFROM2SHORT(0, MCLBPP_BACKCOLOR));
          break;
        case PP_FOREGROUNDCOLOR:
          /* Save new color and redraw */
          WinQueryPresParam(hwnd, PP_FOREGROUNDCOLOR, 0L, NULL, sizeof(LONG), &InstData->TitleFColor, 0);
          WinInvalidateRect(hwnd, NULL, FALSE);
          // Tell owner about the pres parm change
          WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_PPCHANGED),
                                                  MPFROM2SHORT(0, MCLBPP_FORECOLOR));
          break;
      }
      return 0;

    case WM_PAINT: {
      RECTL   Rect;
      POINTL  Point;
      HPS     Hps;
      SWP     Pos;
      SWP     LBPos;
      LONG    Top, Bottom;
      LONG    TitleColor;
      MCLBCOLDATA *CurrCol;

      Hps = WinBeginPaint(hwnd, NULLHANDLE, &Rect);
      WinQueryWindowPos(hwnd, &Pos);
      WinQueryWindowPos(InstData->ColList->Next->BoxHwnd, &LBPos);
      Top    = Pos.cy - 1;
 //   Bottom = InstData->ListBoxCy;
      Bottom = LBPos.cy;

      /* Fill title area with title background color */
      Rect.xLeft   = 0;
      Rect.xRight  = Pos.cx-1;
      Rect.yBottom = Bottom;
      Rect.yTop    = Top;
      WinFillRect(Hps, &Rect, MCLBColorIndex(Hps, InstData->TitleBColor));

      /* Match border color of listboxes to draw title border */
      GpiSetColor(Hps, SYSCLR_WINDOWFRAME);
      Point.x = 0;
      Point.y = Bottom;
      GpiMove(Hps, &Point);
      Point.y = Top;
      GpiLine(Hps, &Point);
      Point.x = Pos.cx - 1;
      GpiLine(Hps, &Point);
      Point.y = Bottom;
      GpiLine(Hps, &Point);

      /* Draw each title string centered over column listbox */

      TitleColor = MCLBColorIndex(Hps, InstData->TitleFColor);
      for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {
        WinQueryWindowPos(CurrCol->BoxHwnd, &Pos);
        Rect.yBottom = Bottom;
        Rect.yTop    = Top;
        Rect.xLeft = Pos.x + TITLEMARGIN_LEFT;
        Rect.xRight= Pos.x + Pos.cx
                           - WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL)
                           - 1
                           - TITLEMARGIN_RIGHT;
        GpiSetColor(Hps, TitleColor);
        WinDrawText(Hps, -1L, CurrCol->Title, &Rect, 0L, 0L, DT_CENTER|DT_VCENTER|DT_TEXTATTRS);

        /* For non-resizable MCLB, we extend the left border of the          */
        /* listbox through the title area since there is no separator window */
        if (InstData->Style & MCLBS_NOCOLRESIZE) {
          GpiSetColor(Hps, SYSCLR_WINDOWFRAME);
          Point.x = Pos.x;
          Point.y = Bottom -1;
          GpiMove(Hps, &Point);
          Point.y = Top;
          GpiLine(Hps, &Point);
        }
      }
 
      WinEndPaint(Hps);
      return 0;
      }

  } // switch on msg

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************/
MRESULT EXPENTRY MCLBSepProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
/*****************************************************************************/
/* Main window procedure for the separator window class.  This window        */
/* handles the resizing process.  Separator windows are not created when the */
/* MCLBS_NOCOLRESIZE style is specified.                                     */
/*****************************************************************************/
{
MCLBCOLDATA *ColData;

  ColData = WinQueryWindowPtr(hwnd, 0L);

  switch (msg) {
    case WM_CREATE:
      WinSetWindowPtr(hwnd, 0L, (MCLBCOLDATA *)mp1);
      break;

    case WM_MOUSEMOVE:
      /* When the mouse moves on the separator, change it to a dbl-arrow */
      WinSetPointer(HWND_DESKTOP,
                    WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE));
      return (MRESULT)TRUE;

    case WM_BUTTON1DOWN:
      /* Display and process a tracking rectangle to resize the columns */
      MCLBTracking(hwnd, ColData, 0);
      return (MRESULT)TRUE;
 
    case WM_PAINT: {
      RECTL   Rect;
      POINTL  Point;
      HPS     Hps;
      SWP     Pos;

      Hps = WinBeginPaint(hwnd, NULLHANDLE, &Rect);
      WinQueryWindowPos(hwnd, &Pos);
      /* Draw edge connecting adjacent listboxes at bottom of window */
      Point.x = Pos.cx-1;
      Point.y = 0;
      GpiSetColor(Hps, CLR_WHITE);
      GpiLine(Hps, &Point);

      Rect.xLeft   = 0;
      Rect.yBottom = 1;
      Rect.xRight  = Pos.cx;  // WinDrawBorder() stays inside the rectangle
      Rect.yTop    = Pos.cy;
      WinDrawBorder(Hps, &Rect, 1L, 1L,
                    SYSCLR_WINDOWFRAME, SYSCLR_WINDOW,
                    DB_PATCOPY|DB_INTERIOR);
      WinEndPaint(Hps);
      return 0;
    }

  } // switch on msg

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*****************************************************************************/
MRESULT EXPENTRY MCLBListBoxSubclassProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
/*****************************************************************************/
/* This is a subclass procedure for the MCLB listbox controls.  We just      */
/* watch for a few key messages here.                                        */
/*****************************************************************************/
{
MCLBCOLDATA *ColData;    // Data for this column
MCLBCOLDATA *CurrCol;    // Linked list of columns
MCLBINSTDATA *InstData;  // MCLB instance data

#define KEYDOWN(key) (WinGetKeyState(HWND_DESKTOP,key) & 0x8000)

  ColData = WinQueryWindowPtr(hwnd, 0L);
  InstData = ColData->InstData;

  switch (msg) {
      case WM_BUTTON1DOWN: {
        SHORT n, i, NewCursor;

        if (!(InstData->Style & MCLBS_CUASELECT))  // Do nothing special if not CUA style
          break;

        if (KEYDOWN(VK_SHIFT)| KEYDOWN(VK_CTRL))   // Let listbox handle CTRL and SHIFT selections
          break;
 
        /* Now this is a left-button selection of a listbox item.  For CUA-like    */
        /* selection we must de-select all other items in the listbox (normal      */
        /* listbox does this only if the item clicked on is not already selected). */
 
        NewCursor = MCLBLocateListboxItem( hwnd, SHORT2FROMMP(mp1) );

        n = (SHORT)WinSendMsg( hwnd, LM_QUERYITEMCOUNT, 0L, 0L );
        for (i=0; i<n; i++ ) 
          if (i != NewCursor)
            WinSendMsg(hwnd, LM_SELECTITEM, MPFROMSHORT(i), MPFROMSHORT(FALSE));

        break;
        }

    case WM_FOCUSCHANGE:
      /* Record the fact the we are the child that has focus */
      InstData->FocusChild = hwnd;
      break;

    case WM_BEGINSELECT:
    case WM_BEGINDRAG:
    case WM_ENDDRAG:
    case WM_ENDSELECT:
    case WM_CONTEXTMENU:
      /* Lisbox proc absorbes this but does not use it.  Send it to */
      /* our owner like the PM docs say it should be.  Otherwise    */
      /* cannot use direct-manipulation listbox (DMLB) subclass.    */
      return WinSendMsg(InstData->MCLBHwnd, msg, mp1, mp2);

    case WM_PRESPARAMCHANGED:
      if (!InstData->Initializing)   // Skip this when MCLB is being created
      switch (LONGFROMMP(mp1)) {
        case PP_FONTNAMESIZE: {
          SWP Pos;
          /* Must propagate font changes to all other listbox controls, but */
          /* not if this was caused by some other listboxs font change.     */
          if (InstData->PPChanging) // First listbox to change does everything
            break;

          /* Get new font name */
          if (WinQueryPresParam(hwnd, PP_FONTNAMESIZE, 0L, NULL, MAX_FONTLEN, &InstData->ListFont, QPF_NOINHERIT) == 0)
            (InstData->ListFont)[0] = '\0';

          /* Tell all other listboxes */
          InstData->PPChanging = TRUE;
          for (CurrCol=InstData->ColList->Next; CurrCol!=NULL; CurrCol=CurrCol->Next)
            WinSetPresParam(CurrCol->BoxHwnd, PP_FONTNAMESIZE, strlen(InstData->ListFont)+1, InstData->ListFont);

          InstData->PPChanging = FALSE;
          // Recaluclate sizes so listboxs can adjust to prevent clipping if
          // they are not LS_NOADJUSTPOS style.
          WinQueryWindowPos(InstData->MCLBHwnd, &Pos);
          MCLBSizeChildren(InstData, Pos.cx, Pos.cy);
          // Tell owner about the pres parm change
          WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_PPCHANGED),
                                                  MPFROM2SHORT(ColData->ColNum, MCLBPP_FONT));
          }
          break;
        case PP_BACKGROUNDCOLOR:
          /* Propagate colors to other listboxes if not multi-color style */
          if (!(InstData->PPChanging)) { // We are fist to get this, tell others if necessary
            InstData->PPChanging = TRUE;
            WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, 0L, NULL, sizeof(LONG), &InstData->ListBColor, 0);
            if (!(InstData->Style & MCLBS_MULTICOLOR)) {
              for (CurrCol=InstData->ColList->Next; CurrCol!=NULL; CurrCol=CurrCol->Next)
                WinSetPresParam(CurrCol->BoxHwnd, PP_BACKGROUNDCOLOR, sizeof(LONG), &InstData->ListBColor);
            }
            InstData->PPChanging = FALSE;
            // Tell owner about the pres parm change
            WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_PPCHANGED),
                                                    MPFROM2SHORT(ColData->ColNum, MCLBPP_BACKCOLOR));
          }
          break;
        case PP_FOREGROUNDCOLOR:
          /* Propagate colors to other listboxes if not multi-color style set */
          if (!(InstData->PPChanging)) { // We are fist to get this, tell others if necessary
            InstData->PPChanging = TRUE;
            WinQueryPresParam(hwnd, PP_FOREGROUNDCOLOR, 0L, NULL, sizeof(LONG), &InstData->ListFColor, 0);
            if (!(InstData->Style & MCLBS_MULTICOLOR)) {
              for (CurrCol=InstData->ColList->Next; CurrCol!=NULL; CurrCol=CurrCol->Next)
                WinSetPresParam(CurrCol->BoxHwnd, PP_FOREGROUNDCOLOR, sizeof(LONG), &InstData->ListFColor);
            }
            InstData->PPChanging = FALSE;
            // Tell owner about the pres parm change
            WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_PPCHANGED),
                                                    MPFROM2SHORT(ColData->ColNum, MCLBPP_FORECOLOR));
          }
          break;
      }
      break;  // Pass on all pres param changes to listbox proc

    case WM_CHAR: {
      /* We watch for CTRL/ALT arrow keys so the user can move focus between      */
      /* columns and resize columns with the keyboard (no mouse).                 */
      USHORT KeyStatus;
      USHORT VirtKey;
      MCLBCOLDATA *ColTemp;
  
      KeyStatus = SHORT1FROMMP(mp1);
      VirtKey   = SHORT2FROMMP(mp2);
  
      if ((KeyStatus & (KC_VIRTUALKEY|KC_ALT|KC_KEYUP)) ==  // Virtual key, ALT pressed, downstroke
                       (KC_VIRTUALKEY|KC_ALT)) {
        switch (VirtKey) {
          case VK_LEFT:  /*----- ALT-Left keystroke -----*/
            /* Move focus 1 column left if possible */
            for (ColTemp=InstData->ColList->RightCol; ColTemp->RightCol!=NULL; ColTemp=ColTemp->RightCol) {
              if (ColTemp->RightCol->BoxHwnd == hwnd) {
                WinSetFocus(HWND_DESKTOP, ColTemp->BoxHwnd);
                break;
              }
            }   
            return (MRESULT)TRUE;
  
          case VK_RIGHT: /*----- ALT-Right keystroke -----*/
            /* Move focus 1 column right if possible */
            if (ColData->RightCol != NULL)
              WinSetFocus(HWND_DESKTOP, ColData->RightCol->BoxHwnd);
            return (MRESULT)TRUE;
        } // switch on virtual key
        break; // some other ALT key -- pass it on
      } // if ALT key pressed
  
      if ((KeyStatus & (KC_VIRTUALKEY|KC_CTRL|KC_KEYUP)) ==  // Virtual key, CTRL pressed, downstroke
                       (KC_VIRTUALKEY|KC_CTRL)) {
        switch (VirtKey) {
          case VK_LEFT:  /*----- CTRL-Left keystroke -----*/
          case VK_RIGHT: /*----- CTRL-Right keystroke -----*/
            /* Start tracking separator assoicated with this column */
              if (ColData->SepHwnd != NULLHANDLE)                   // It has a separator
                MCLBTracking(ColData->SepHwnd, ColData, TF_SETPOINTERPOS);
            return (MRESULT)TRUE;                                   // All done
        } // switch on virtual key
        break; // some other CTRL key -- pass it on
      } // if CTRL key pressed
      break;
  
    } // end WM_CHAR processing
  }

  /* Call original listbox window proc */
  return (ColData->ListBoxProc)(hwnd, msg, mp1, mp2);
}
 

/*****************************************************************************/
void _Optlink MCLBSizeChildren(MCLBINSTDATA *InstData, ULONG Cx, ULONG Cy)
/*****************************************************************************/
/* Give an outer window size and position, calculate the size and position   */
/* of all the MCLB child windows.                                            */
/*****************************************************************************/
{
  LONG  SepCx;         // Width of separator windows
  LONG  VScrollCx;     // Width of vert scroll
  LONG  BoxCx;         // Width of a particular listbox
  HPS   Hps;           // Pres space for textbox measurements
  RECTL Rect;
  MCLBCOLDATA *CurrCol;
  LONG SumX;           // Sum of X sizes or delta-X size
  LONG DifX;           // Sum of X sizes or delta-X size
//LONG RemX;           // Remainder of equal distribution
  BOOL WasVisible;     // Window was visible during this operation
  USHORT Magic1,Magic2;// Magic adjustments for self-adjusting listbox style
  LONG HeadingCy;      // Minimum height needed for headings

  /* Set size of frame to entire window (always) */
  WinSetWindowPos(InstData->Frame, HWND_BOTTOM, 0, 0, Cx, Cy, SWP_MOVE|SWP_SIZE|SWP_ZORDER);

  if (InstData->Style & MCLBS_NOCOLRESIZE)
    SepCx = 1;         // Non-sizing separators are just 1 pixel wide
  else
    SepCx = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);

  VScrollCx  = WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);

  /* Use WinDrawText() to see how big (vertically) the headings will be */
  /* by using 1.5X the font size.  Note headings may be drawn   larger  */
  /* if LS_NOADJUSTPOS style is not used (because listboxs may adjust   */
  /* themselves to a smaller size and we make up for it in the heading).*/

  Hps = WinGetPS(InstData->Frame);     // Use frame window fonts for this
  memset(&Rect, 0x00, sizeof(RECTL));
  Rect.yTop   = MAX_INT;
  Rect.xRight = MAX_INT;
  WinDrawText(Hps, -1, " ", &Rect, 0L, 0L, DT_QUERYEXTENT|DT_LEFT|DT_BOTTOM);
  HeadingCy = Rect.yTop - Rect.yBottom;
  HeadingCy += HeadingCy / 2;
  WinReleasePS(Hps);

  /* Listboxes are window size minus headings minus borders */
  InstData->ListBoxCy = Cy - HeadingCy - 1;

  /* Usable listbox area is width of windows minus 1 scrollbar, separators and left border */
  InstData->UsableCx = Cx - VScrollCx - (SepCx * ((InstData->Cols)-1)) - 1;

  /* If current column widths do not fit exactly in available space, */
  /* use style-specific algorithm to resize them.                    */

  for (SumX=0, CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol)
    SumX = SumX + CurrCol->CurrSize;
  if (SumX != InstData->UsableCx) {
    switch (InstData->Style & MCLBS_SIZEMETHOD_MASK) {
      case MCLBS_SIZEMETHOD_CUSTOM: {
        //---------------------------------------------------------------------------------------
        LONG *ResizeData;
        int i;
    
        /* Build array with new size and each columns current size */
        ResizeData = malloc((InstData->Cols + 1)*sizeof(LONG));
        ResizeData[0] = InstData->UsableCx;
        for (CurrCol = InstData->ColList->RightCol, i=0; CurrCol!=NULL; CurrCol=CurrCol->RightCol, i++)
          ResizeData[i] = CurrCol->CurrSize;
    
        /* Let owner setup new column sizes.  If owner chooses not to, use */
        /* default proportional method.                                    */
        if ((BOOL)WinSendMsg(InstData->Owner, WM_CONTROL, MPFROM2SHORT(InstData->Id, MCLBN_CUSTOMSIZE), MPFROMP(ResizeData))) {
          for (CurrCol = InstData->ColList->RightCol, i=0; CurrCol!=NULL; CurrCol=CurrCol->RightCol, i++)
            CurrCol->CurrSize = ResizeData[i];
          free(ResizeData);
          break;
        }
        free(ResizeData);
        // NOTE: fall through next case for default behaviour
        }
      case MCLBS_SIZEMETHOD_PROP: {
        //---------------------------------------------------------------------------------------
        LONG Percent;
        Percent = (InstData->UsableCx*1000L) / SumX;
        DifX = 0;
        for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {
          CurrCol->CurrSize = (CurrCol->CurrSize * Percent) / 1000;
      
          /* If column would exceed usable width, limit it to usable width.  */
          /* This can occure due to rounding errors in above calculations    */
          if (CurrCol->CurrSize + DifX > InstData->UsableCx)
            CurrCol->CurrSize = InstData->UsableCx - DifX;
      
          DifX = DifX + CurrCol->CurrSize;  // Track width used so far
        }
        break;
        }
      case MCLBS_SIZEMETHOD_LEFT: {
        //---------------------------------------------------------------------------------------
        DifX = InstData->UsableCx - SumX;      // How much change was there?
        /* If size increased, just widen 1st column by that much.  If size */
        /* decreased, shrink columns left to right down to zero.           */
        if (DifX >= 0)
          InstData->ColList->RightCol->CurrSize += DifX;
        else {
          for (CurrCol = InstData->ColList->RightCol; ((CurrCol!=NULL) && (DifX < 0L)); CurrCol=CurrCol->RightCol) {
            SumX = min(CurrCol->CurrSize, labs(DifX));  // How much can we take from this col
            CurrCol->CurrSize = CurrCol->CurrSize - SumX;
            DifX = DifX + SumX;                         // Move diff toward zero
          }
        }
        break;
        }
    //case MCLBS_SIZEMETHOD_EQUAL:
      ////---------------------------------------------------------------------------------------
      // This method works but is not all that useful.  The proportional method
      // generally gives better results and is less code.
      ////---------------------------------------------------------------------------------------
      //SumX = (InstData->UsableCx - SumX) / (LONG)InstData->Cols;  // per-column delta
      //RemX = (InstData->UsableCx - SumX) % (LONG)InstData->Cols;  // remainder
      //if (SumX < 0)
      //  RemX = -1L * labs(RemX);  // force neg remainder
      //else
      //  RemX = labs(RemX);        // force pos remainder
      //DifX = 0;
      //for (CurrCol = InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {
      //  CurrCol->CurrSize = max(0L, CurrCol->CurrSize + SumX);
      //  if (RemX < 0) {
      //    CurrCol->CurrSize--;
      //    RemX++;
      //  }
      //  else if (RemX > 0) {
      //    CurrCol->CurrSize++;
      //    RemX--;
      //  }
      //  /* If column would exceed usable width, limit it to usable width.  */
      //  /* This can occure when some columns resize to <0 and we can only make them 0 */
      //  if (CurrCol->CurrSize + DifX > InstData->UsableCx)
      //    CurrCol->CurrSize = InstData->UsableCx - DifX;
      //
      //  DifX = DifX + CurrCol->CurrSize;  // Track width used so far
      //}
    } // switch on resize method
  }  // if usable size changed
        
  /* If updates would show on display, disable them until all done.  This */
  /* allows owner to disable the MCLB while several resize or pres param  */
  /* changes are done.                                                    */
  if (WinIsWindowVisible(InstData->MCLBHwnd)) {
    WinEnableWindowUpdate(InstData->MCLBHwnd, FALSE);
    WasVisible = TRUE;
  }
  else
    WasVisible = FALSE;

  /* Walk list of columns in visual order and set each size/position */
  SumX = 1;                     // Init running count
  if (InstData->Style & LS_NOADJUSTPOS) {
    Magic1 = 0;           // No magic adjustment reqd for exact-size listboxs
    Magic2 = 0;                 
  }
  else {
    Magic1 = 1;           // Ignore magic numbers... no logic to them at all, 
    Magic2 = 4;           //   just emperically derived values.               
  }
  for (CurrCol=InstData->ColList->RightCol; CurrCol!=NULL; CurrCol=CurrCol->RightCol) {

    /* Col sizes should fit exactly in available space, but if not */
    /* force-fit the last column.                                  */
    if (CurrCol->RightCol == NULL)
      CurrCol->CurrSize = (Cx-VScrollCx) - SumX;

    BoxCx = CurrCol->CurrSize;

    WinSetWindowPos(CurrCol->BoxHwnd,
                    HWND_TOP,
                    SumX-1+Magic1,             // Next pixel, minus border
                    Magic1,                    // Always at bottom
                    BoxCx+VScrollCx+1-Magic2,  // Width incl hidden scrollbar and border
                    InstData->ListBoxCy,       // All boxes same height
                    SWP_MOVE|SWP_SIZE);
     
    SumX = SumX + BoxCx;

    /* Put separator just to right of box, extending up through title area */
    /* (no sep window for last column or for NORESIZE style).              */

    if ((CurrCol->SepHwnd != NULLHANDLE) && (!(InstData->Style & MCLBS_NOCOLRESIZE))) {
      WinSetWindowPos(CurrCol->SepHwnd,
                      HWND_TOP,
                      SumX,               // Right of box
                      0,                  // Bottom of window
                      SepCx,              // Width
                      Cy,                 // Full window height
                      SWP_MOVE|SWP_SIZE|SWP_ZORDER);
    
      SumX = SumX + SepCx;                // Next open pixel
    }
    if (InstData->Style & MCLBS_NOCOLRESIZE) // Skip one pixel for noresize style
      SumX++;                                //   so left border will show

  } // while more columns

  if (WasVisible)  // reshow window if we disabled it
    WinEnableWindowUpdate(InstData->MCLBHwnd, TRUE);

}
 
/*****************************************************************************/
void _Optlink MCLBCreateColumn(HWND MCLBHwnd, ULONG Style, MCLBCOLDATA *ColData)
/*****************************************************************************/
/* Create one column of the MCLB.  Some of the ColData structure has already */
/* been filled in.                                                           */
/*****************************************************************************/
{
//HENUM HEnum;    /* Window enumeration handle  */
//HWND  Next;     /* Next window in enumeration */
LONG  LBStyle;  /* Listbox styles             */

  /* Enforce some listbox styles, remove our own from the bit flags */
  LBStyle = Style & ~MCLBS_MASK;   // Remove MCLB-specific style bits
  LBStyle = LBStyle |          // Add required styles
            WS_CLIPSIBLINGS |  // Clip other MCLB children
            WS_VISIBLE ;       // Make it visible
      //    LS_NOADJUSTPOS;    // Make it the exact specified size

  /* Create listbox control for this column */
  ColData->BoxHwnd = WinCreateWindow(
                        MCLBHwnd,          // MCLB window is parent
                        WC_LISTBOX,        // Std PM listbox class
                        "",                // No window text
                        LBStyle,           // Calculated style flags
                        0,0,0,0,           // Size/pos will be set later
                        MCLBHwnd,          // MCLB window is owner & gets all messages
                        HWND_TOP,          // Put on top of other children
                        ColData->ColNum,   // 1-based column number
                        NULL, NULL);       // No ctrl data or pres params

  if (ColData->BoxHwnd == NULLHANDLE)      // Quit if failure
    return;

  /* For this column, subclass the listbox, hide the vertical scrollbar, */
  /* and create the separator window to the right.  If this is the last  */
  /* column we don't hide or create a separator.                         */

  WinSetWindowPtr(ColData->BoxHwnd, 0L, ColData); // Set window ptr to column data
  ColData->ListBoxProc = WinSubclassWindow(ColData->BoxHwnd, (PFNWP)MCLBListBoxSubclassProc);

  if ((ColData->InstData->Cols != ColData->ColNum) &&      /* For non-last columns and              */
     (!(ColData->InstData->Style & MCLBS_NOCOLRESIZE))) {  /* resizable style controls only.        */

    // --- No need to hide scroll bars, all except last is covered by adjacent column
    //HEnum = WinBeginEnumWindows(ColData->BoxHwnd);         /* Enumerate the children of the listbox */
    //while ((Next=WinGetNextWindow(HEnum))!=NULLHANDLE)     /* Get the handle of the next window     */
    //  if (WinQueryWindowULong(Next,QWL_STYLE) & SBS_VERT)  /* If it is a vertical scrollbar         */
    //    WinShowWindow(Next, FALSE);                        /*     then hide it                      */
    //WinEndEnumWindows(HEnum);                              /* End the enumeration                   */

    /* Create separator window for column separator to right of this column */
 
    ColData->SepHwnd = WinCreateWindow(
                          MCLBHwnd,          // MCLB window is parent
                          MCLB_CLASS_SEP,    // Separator class name
                          "",                // No window text
                          WS_CLIPSIBLINGS |  // Clip other MCLB children
                          WS_VISIBLE,        // Make it visible
                          0,0,0,0,           // Size/pos will be set later
                          MCLBHwnd,          // MCLB window is owner & gets all messages
                          HWND_TOP,          // Put on top of other children
                          0,                 // ID is not of interest
                          ColData,           // Pass column data in mp1 of WM_CREATE
                          NULL);             // No pres params

  }  // if not last column

}

/*****************************************************************************/
LONG _Optlink MCLBColorIndex(HPS Hps, LONG RGBColor)
/*****************************************************************************/
/* Create an entry in the color table for the given RGB value.  The new      */
/* index is returned.  This allows a given RGB value to be selected by index.*/
/*****************************************************************************/
{
LONG ClrTableInfo[4];        // Color table info

  /* Get number of colors in current table */
  GpiQueryColorData(Hps, 4L, ClrTableInfo);
  /* Create new index for this RGB color */
  GpiCreateLogColorTable(Hps, 0L, LCOLF_CONSECRGB,
                         ClrTableInfo[QCD_LCT_HIINDEX] + 1,
                         1L,
                         &RGBColor);

  /* Return index of new entry */
  return ClrTableInfo[QCD_LCT_HIINDEX] + 1;
}

/*****************************************************************************/
void _Optlink MCLBTracking(HWND hwnd, MCLBCOLDATA *LColData, USHORT Option)
/*****************************************************************************/
/* Display and process a tracking rectangle for the given separator window.  */
/* The Option is used on the WinTrackRect() call.  The column data passed is */
/* for the column to the left of the separator.                              */
/*****************************************************************************/
{
TRACKINFO Track;                // PM tracking API data
MCLBCOLDATA *RColData;          // Column data for right of separator
MCLBINSTDATA *InstData;         // Ptr to overall control instance data
SWP       swpBoxL;              // Column left of separator
SWP       swpBoxR;              // Column right of separator
SWP       swpSep;               // Separator window
LONG      diff, vScroll;
LONG      temp;

  RColData = LColData->RightCol; // There is always a col to right of separator
  InstData = LColData->InstData; // Get ptr to control instance data area

  /* Get sizes of adjacent columns */
  WinQueryWindowPos(LColData->BoxHwnd, &swpBoxL);
  WinQueryWindowPos(RColData->BoxHwnd, &swpBoxR);
  WinQueryWindowPos(hwnd, &swpSep);

  /* Adjust sizes by vertical scroll bar width */
  vScroll = (USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);
  swpBoxL.cx -= vScroll;
  swpBoxR.cx -= vScroll;

  /* Setup data for tracking API */
  Track.cxBorder = Track.cyBorder = swpSep.cx/2;  // Track width = 1/2 separator window

  Track.rclTrack.xLeft = swpSep.x;                // Initial tracking rectangle
  Track.rclTrack.xRight = swpSep.x + swpSep.cx;
  Track.rclTrack.yBottom = swpSep.y;
  Track.rclTrack.yTop = swpSep.y + swpSep.cy;

  /* Left bound of track is left edge of left column */
  temp = swpBoxL.x;
  Track.rclBoundary.xLeft = temp;

  /* Right bound is right edge of right column */
  temp = swpBoxR.x + swpBoxR.cx;
  Track.rclBoundary.xRight = temp;

  /* Top and bottom bounds are fixed */
  Track.rclBoundary.yBottom = swpSep.y;
  Track.rclBoundary.yTop = swpSep.y + swpSep.cy;

  /* Size of tracking rectangle is size of separator window */
  Track.ptlMaxTrackSize.x = Track.ptlMinTrackSize.x = swpSep.cx;
  Track.ptlMaxTrackSize.y = Track.ptlMinTrackSize.y = swpSep.cy;

  /* Keyboard increments */
  Track.cxGrid = Track.cxKeyboard = 2;

  /* Keep rectangle in the window and move all of it */
  Track.fs = TF_MOVE | TF_ALLINBOUNDARY | Option;

  /* Set pointer to dbl-arrow if we are using the keyboard */
  if (Option == TF_SETPOINTERPOS)
    WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE));

  /* Finally, let PM display and process the rectangle until user lets go   */
  /* using the control window as the tracking surface.                      */

  if (WinTrackRect(InstData->MCLBHwnd, NULLHANDLE, &Track)) {
     SWP Pos;

     diff = (USHORT)Track.rclTrack.xLeft - swpSep.x;

     /* Calculate new column widths */

     LColData->CurrSize = swpBoxL.cx + diff;
     RColData->CurrSize = swpBoxR.cx - diff;
     WinQueryWindowPos(InstData->MCLBHwnd, &Pos);
     MCLBSizeChildren(InstData, Pos.cx, Pos.cy);

     /* Notify MCLB owner that column widths changed */
     WinSendMsg(InstData->Owner, WM_CONTROL,
                MPFROM2SHORT(InstData->Id, MCLBN_COLSIZED), 
                MPFROM2SHORT(LColData->ColNum, 0));

  } // if tracking sucessfull
}

/*****************************************************************************/
void _Optlink MCLBSelectAllColumns(MCLBINSTDATA *InstData, HWND SourceHwnd, SHORT Index, BOOL Select)
/*****************************************************************************/
/* Select the Index item in all columns of the MCLB except for the Source    */
/* column.  Select controls select/deselect option.                          */
/*****************************************************************************/
{
MCLBCOLDATA *CurrCol;

  /* Walk list of columns setting selection on all but source */
  CurrCol = InstData->ColList->RightCol; // First col in list
  while (CurrCol != NULL) {
    if (CurrCol->BoxHwnd != SourceHwnd)
      WinSendMsg(CurrCol->BoxHwnd, LM_SELECTITEM,
                                   MPFROMSHORT(Index), MPFROMSHORT(Select));
    CurrCol = CurrCol->RightCol;
  }
}


/*****************************************************************************/
void _Optlink MCLBSubStr(char *Source, char *Target, int WordNum, char Delim)
/*****************************************************************************/
/* Extract the WordNum'th delimited string from Source and copy to Target.   */
/* Word number is 1-based.  Source is not altered.                           */
/*****************************************************************************/
{
int i;

  for (i=1; (i<WordNum); i++) {  // Skip StrNum-1 delimiters
    while ((*Source!=Delim) && (*Source!='\0'))
      Source++;
    if (*Source!='\0')  // Skip delimiter, if one
      Source++;
  }

  while ((*Source!=Delim) && (*Source!='\0')) { // Copy up to delim or end
    *Target = *Source;
    Target++;
    Source++;
  }

  *Target = '\0';  // Append trailing null
}


/*----------------------------------------------------------------------*/
SHORT _Optlink MCLBLocateListboxItem( HWND hwnd, SHORT Y)
/*----------------------------------------------------------------------*/
/* Parameters:                                                          */
/*   hwnd         Listbox of interest                                   */
/*   Y            Y position of the pointer                             */
/*----------------------------------------------------------------------*/
/* Given a Y window coordinate, this function will calculate the item   */
/* number of the listbox item at that position.  This is a short version*/
/* of DMLBLocateListboxItem().                                          */
/*----------------------------------------------------------------------*/
{
   RECTL   Rect;
   POINTL  Points[2];
   HPS     hps;
   LONG    VertSize;
   SHORT   ItemNum;

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

   /* Return item under pointer */
   return ItemNum;
}
  

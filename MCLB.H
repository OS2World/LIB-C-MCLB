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
/* External MCLB definitions */

#define MAX_FONTLEN   FACESIZE+4        /* System fontface size plus room for size spec.*/

/* Structure used to create MCLB windows */

typedef struct _MCLBINFO { 
  ULONG  Size;                          /* Length of this structure                   */
  char   *Titles;                       /* Title strings (TabChar separated)          */
  char   TitleFont[MAX_FONTLEN];        /* Title font (null to inherit from owner)    */
  char   ListFont[MAX_FONTLEN];         /* List  font (null to inherit from owner)    */
  ULONG  TitleBColor;                   /* Title background color                     */
  ULONG  TitleFColor;                   /* Title foreground color                     */
  ULONG  ListBColor;                    /* List  background color                     */
  ULONG  ListFColor;                    /* List  foreground color                     */
  LONG   *InitSizes;                    /* Ptr to array of initial sizes              */
  char   _Reserved[64];                 /* Reserved for future use                    */
  USHORT Cols;                          /* Number of columns                          */
  char   TabChar;                       /* Data column separator character            */
  char   _Padd;                         /* Padd for separator character (zero)        */
} MCLBINFO;

/* Prototype for MCLB create function */

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
                MCLBINFO  *Info);   // MCLB create structure

/*********************************************************************/
/* MCLB style flags must not collide with any PM style bits          */
/* which as of V2.1 are:                                             */
/*                                                                   */
/* Std window style  WS_*   XXXX XXXX XX-- ----  ---- ---- ---- ---- */
/* Dlg manager style WS_*   ---- ---- ---- -XXX  ---- ---- ---- ---- */
/* Listbox styles    LS_*   ---- ---- ---- ----  ---- ---- ---X XXXX */
/*                                                                   */
/* ...so we use:  MCLBS_*   ---- ---- ---- ----  XXXX X--- ---- ---- */
/*                                                                   */
/*********************************************************************/

#define MCLBS_NOCOLRESIZE        0x00000800L // No resizing of columns allowed (e.g. no splitbars)
#define MCLBS_SIZEMETHOD_PROP    0x00000000L // Resize columns proportially (default) when control size chngs
//#define MCLBS_SIZEMETHOD_EQUAL 0x00001000L // Resize columns equally (PROP is preferred)
#define MCLBS_SIZEMETHOD_LEFT    0x00002000L // Resize left column
#define MCLBS_SIZEMETHOD_CUSTOM  0x00003000L // Application supplies resize algorithm
#define MCLBS_SIZEMETHOD_MASK    0x00003000L // Mask to extract method bits
#define MCLBS_MULTICOLOR         0x00004000L // Allow columns to have different colors
#define MCLBS_CUASELECT          0x00008000L // Perform CUA-style extended selection (like container)
#define MCLBS_MASK               0x0000F800L // Mask to extract all MCLB style bits

/*********************************************************************/
/* MCLB notification messages (must not collide with LN_* PM         */
/* values which as of V2.1 were from 1 to 5).  These values          */
/* are in short 2 of mp1 of WM_CONTROL messages.                     */
/*********************************************************************/

#define MCLBN_COLSIZED          400 // Column sizes were modified
#define MCLBN_CUSTOMSIZE        401 // Control size changed, set new col sizes (MCLBS_SIZEMETHOD_CUSTOM style only)
#define MCLBN_PPCHANGED         402 // Control pres parm changed:
                                    //    mp2 = (short) Column number or zero for title
                                    //          (short) MCLBPP_FONT/FORECOLOR/BACKCOLOR

#define MCLBPP_FONT               1 // Indicates font change in MCLBN_PPCHANGED control message
#define MCLBPP_FORECOLOR          2 // Indicates foreground color change in MCLBN_PPCHANGED control message
#define MCLBPP_BACKCOLOR          3 // Indicates background color change in MCLBN_PPCHANGED control message

/*********************************************************************/
/* MCLB messages -- these are messages which can be sent to the MCLB */
/* control to perform MCLB-specific functions.  This is in addition  */
/* to all the normal listbox (LS_*) messages.                        */
/*********************************************************************/

#define MCLB_SETTITLES      (WM_USER+100)  // Set new column titles (mp1=delimited string)
#define MCLB_SETTITLEFONT   (WM_USER+101)  // Set title font (mp1=title font).  NULL to use default font.
#define MCLB_SETLISTFONT    (WM_USER+102)  // Set list font (mp1=title font).  NULL to use default font.
#define MCLB_SETTITLECOLORS (WM_USER+103)  // Set title colors (mp1=foreground, mp2=background), RGB values, if mp1=mp1 use defaults
#define MCLB_SETLISTCOLORS  (WM_USER+104)  // Set list colors (mp1=fore, mp2=back), RGB values, if mp1=mp2 use defaults
#define MCLB_SETCOLSIZES    (WM_USER+105)  // Set column sizes (mp1=ptr to array of LONG).  Sizes should addup to MCLB_QUERYFULLSIZE result.
#define MCLB_QUERYCOLSIZES  (WM_USER+106)  // Query current column sizes (mp1=ptr to array of LONG).  Returned values are pixels.
#define MCLB_QUERYINFO      (WM_USER+107)  // Query MCLBINFO structure.  App must free title string & array of sizes.
#define MCLB_QUERYSTYLE     (WM_USER+108)  // Query MCLB style flags.  Only MCLB-specific bits are returned (e.g. not WS_VISIBLE, etc).
#define MCLB_QUERYFULLSIZE  (WM_USER+109)  // Query width of sum of columns (e.g. width available for columns)
#define MCLB_QUERYCTLCOL    (WM_USER+110)  // Query source of current WM_CONTROL message (column number) -- must be SENT, only valid during WM_CONTROL in owner

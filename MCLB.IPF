.***************************************************************************
.* MCLB toolkit online reference.
.*
.* NOTE: The IPFC pre-processor utility (IPFCPREP) is required to
.* process this document.
.***************************************************************************
.***************************************************************************
.***************************************************************************
.*                        DISCLAIMER OF WARRANTIES.                        *
.***************************************************************************
.***************************************************************************
.*                                                                         *
.*  Copyright (C) 1995 IBM Corporation                                     *
.*                                                                         *
.*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is        *
.*      sample code created by IBM Corporation. This sample code is not    *
.*      part of any standard or IBM product and is provided to you solely  *
.*      for  the purpose of assisting you in the development of your       *
.*      applications.  The code is provided "AS IS", without               *
.*      warranty of any kind.  IBM shall not be liable for any damages     *
.*      arising out of your use of the sample code, even if they have been *
.*      advised of the possibility of such damages.                        *
.***************************************************************************
:userdoc.
.********* Useful IPFC macros *********
.dm MCLBINFO on
:link reftype=hd res=101.MCLBINFO:elink.
.dm off
.dm WM_CONTROL on
:link reftype=hd res=500.WM_CONTROL:elink.
.dm off
.dm WM_MEASUREITEM on
:link reftype=hd res=501.WM_MEASUREITEM:elink.
.dm off
.dm WM_DRAWITEM on
:link reftype=hd res=502.WM_DRAWITEM:elink.
.dm off
.dm MCLBCreateWindow on
:link reftype=hd res=1000.MCLBCreateWindow:elink.
.dm off
.dm MCLBN_COLSIZED on
:link reftype=hd res=900.MCLBN_COLSIZED:elink.
.dm off
.dm MCLBN_CUSTOMSIZE on
:link reftype=hd res=901.MCLBN_CUSTOMSIZE:elink. 
.dm off
.dm MCLBN_PPCHANGED on
:link reftype=hd res=902.MCLBN_PPCHANGED:elink. 
.dm off
.dm  MCLB_SETTITLES on
:link reftype=hd res=910.MCLB_SETTITLES:elink. 
.dm off
.dm  MCLB_SETTITLEFONT on
:link reftype=hd res=911.MCLB_SETTITLEFONT:elink. 
.dm off
.dm  MCLB_SETLISTFONT on
:link reftype=hd res=912.MCLB_SETLISTFONT:elink. 
.dm off
.dm  MCLB_SETTITLECOLORS on
:link reftype=hd res=913.MCLB_SETTITLECOLORS:elink. 
.dm off
.dm  MCLB_SETLISTCOLORS on
:link reftype=hd res=914.MCLB_SETLISTCOLORS:elink. 
.dm off
.dm  MCLB_SETCOLSIZES on
:link reftype=hd res=915.MCLB_SETCOLSIZES:elink. 
.dm off
.dm  MCLB_QUERYCOLSIZES on
:link reftype=hd res=916.MCLB_QUERYCOLSIZES:elink. 
.dm off
.dm  MCLB_QUERYINFO on
:link reftype=hd res=917.MCLB_QUERYINFO:elink. 
.dm off
.dm  MCLB_QUERYSTYLE on
:link reftype=hd res=918.MCLB_QUERYSTYLE:elink. 
.dm off
.dm  MCLB_QUERYFULLSIZE on
:link reftype=hd res=919.MCLB_QUERYFULLSIZE:elink. 
.dm off
.dm  MCLB_QUERYCTLCOL on
:link reftype=hd res=920.MCLB_QUERYCTLCOL:elink. 
.dm off
.dm MCLBS_NOCOLRESIZE on
:link reftype=hd res=400.MCLBS_NOCOLRESIZE:elink.
.dm off
.dm MCLBS_SIZEMETHOD_PROP on
:link reftype=hd res=400.MCLBS_SIZEMETHOD_PROP:elink.
.dm off
.dm MCLBS_CUASELECT on
:link reftype=hd res=400.MCLBS_CUASELECT:elink.
.dm off
.dm MCLBS_SIZEMETHOD_LEFT on
:link reftype=hd res=400.MCLBS_SIZEMETHOD_LEFT:elink.
.dm off
.dm MCLBS_SIZEMETHOD_CUSTOM on
:link reftype=hd res=400.MCLBS_SIZEMETHOD_CUSTOM:elink.
.dm off
.***************************************
:title.Multi-Column ListBox (MCLB)
.***************************************
:h1. Introduction
This document is the programmer's reference for the MultiColumn ListBox (MCLB)
PM control.  For more information on programming with this
control, see :hp1.OS/2 Developer Magazine:ehp1., Nov/Dec 1995.
:p.
:artwork align=center name='pic1.bmp'.
:p.
:hp7.Acknowledgments:ehp7.
.br
This control was originally conceived at the IBM United Kingdom Warwick
Development group by Charles Cooper.  The control was rewritten
and enhanced by Mark McMillan of IBM, Research Triangle Park, USA.
:p.
:hp7.Description:ehp7.
.br
The :hp3.MultiColumn ListBox:ehp3. is a very useful PM custom control
designed to overcome some of the limitations of the standard
PM listbox and container controls.  It is simpler and in many cases
faster than a container, but provides multicolumn (tabular) support
for display of column-oriented data.
:p.
The MCLB supports individually sizable columns, optional horizontal
scroll bars at the base of each column, seperate font and color support
for the titles and column data, owner-drawn lists,
and a simple listbox-style programming
model.
:p.
The MCLB can be subclassed with the DMLB (Direct Manipulation ListBox)
function also supplied in this toolkit to provide drag/drop reordering
capability to the MCLB.
.*-------------------------------------------------------------------------
:h1 res=401.Creating an MCLB
To create an MCLB the application must create an :MCLBINFO. data structure
and initialize it.  At a minimum, the following fields must be supplied:
:ul compact.
:li.Size
:li.Cols
:li.Titles
:li.InitSizes
:li.TabChar
:eul.
.br
The application must then call :MCLBCreateWindow. to create the MCLB
control.
:p.
As supplied in the toolkit, the
MCLB does not support
direct creation from a dialog template.
However, it is very easy to get the same effect by defining a
static rectangle (or any other control) in the dialog template to
act as a placeholder.  During WM_INITDLG processing the application
can create the MCLB and size/position
it exactly on top of the placeholder control.
See the SAMP1 sample for code that does this.
.*-------------------------------------------------------------------------
:h1 res=400.Styles
The visual appearance of an MCLB can be
modified through the use of 'style' flags which are passed
to the :MCLBCreateWindow. API when the control is created.  The
style flags cannot be changed after the MCLB is created.
In addition
to the flags described below, any standard PM WS_* window style flags
can be used (e.g. WS_VISIBLE).  The standard PM listbox styles are
shown here, along with the extended styles unqiue to MCLB controls:
:ul.
:li.
:hp2.LS_MULTIPLESEL:ehp2.  Allow multiple items to be selected in the list
(same as normal PM listbox control).
:li.
:hp2.LS_EXTENDEDSEL:ehp2.  Allows extended selection methods to be used in the list
(same as normal PM listbox control).
:li.
:hp2.LS_HORZSCROLL:ehp2.  When this style is specified each column of the
MCLB will have a horizontal scroll bar at the bottom.  Note that for LS_OWNERDRAW
style, the application must properly process :WM_MEASUREITEM. messages in order
for the scroll bar to work correctly.  This is true even if the application
returns FALSE from :WM_DRAWITEM. and lets PM draw the text of an ownerdraw listbox.
:li.
:hp2.LS_OWNERDRAW:ehp2.  The listbox items are to be drawn by the application.
When this style is used the application must respond to the :WM_MEASUREITEM.
and :WM_DRAWITEM. messages.  This style applies to all columns of the MCLB (e.g.
the application will be responsible for drawing all items in all columns).
:li.
:hp2.LS_NOADJUSTPOS:ehp2.  When this style is specified the control is
sized exactly as specified and the last row may be partially visible.  When this
style is not specified, the columns will be vertically sized such that
the last row is not clipped.  The overall MCLB control size is never altered 
(the size of the title area will be adjusted to prevent clipping if this
style is not specified).
:li.
:hp2.MCLBS_CUASELECT:ehp2.  When this style is specified the
LS_MULTIPLESEL and LS_EXTENDEDSEL styles will also be used.  This
style causes the MCLB to deselect all items except one when the
left button is pressed.  This behaviour is consistent with the
container control.  Without this style, a left button press on
an already selected item will have no effect.
:li.
:hp2.MCLBS_NOCOLRESIZE:ehp2.  When this style is specified the user is not
allowed to change the relative column sizes.  The MCLB will be displayed
without splitbars between the columns (just thin vertical lines will be
drawn between the columns).  This style applies to all columns of the MCLB.
It is not possible to have some sizable and some fixed-width columns in the
same MCLB control.  The default is for the columns to be sizable and
movable splitbars to be displayed between each column.
:li.
:hp2.MCLBS_SIZEMETHOD_PROP:ehp2.  This is the default SIZEMETHOD style if none
is specified.  The SIZEMETHOD styles determine how the column sizes are adjusted
if the MCLB control is resized by the application.  Note that this is not
related to user-initiated column sizing, but only to what happens when the
entire MCLB size is changed.
:p.
This method will cause each column size to be adjusted in proportion to its
previous size.  For example, if a column was 20% of the width of the MCLB
before resizing, it will be adjusted to be 20% of the width after resizing.
Thus each column remains the same :hp2.relative:ehp2. size when the size
of the control is changed.
:li.
:hp2.MCLBS_SIZEMETHOD_LEFT:ehp2.  With this SIZEMETHOD style, any change in the
size of the MCLB control is reflected in the leftmost MCLB columns.  If the
MCLB size is increased, the leftmost column will increase by the same
amount.  If the size is decreased, column size reduction starts in the
leftmost column and continues left-to-right reducing each column to
zero width until the new overall control width is achieved.
:li.
:hp2.MCLBS_SIZEMETHOD_CUSTOM:ehp2.  With this SIZEMETHOD style, any change in the
size of the MCLB control must be handled by the application.  When the size
of the MCLB is changed the application will be sent a :WM_CONTROL. message
with a notification code of :MCLBN_CUSTOMSIZE..  Message parameter 2 will be
a pointer to array of n+1 LONG values where "n" is the number of columns.
The [0] element will contain the new
overall width (accounting for margins, dividers, etc).  The [1]-[n]
elements contain the current column widths in pixels.  The application must
set the new column widths in the [1]-[n] array elements.  The sum of the
[1]-[n] values should equal the overall width given in element [0].
:p.
This style can be used to implement specific column sizing rules as may
be required for particular applications.  For example, it might be used
to prevent a particular column of being reduced below a certain minimum
size.  There are some important notes to consider in using this style:
:ol.
:li.This style effects only the adjustment of column sizes that occures
when the MCLB control size is changed.  It does not effect user-controlled
column sizing.
:li.If the sum of the [1]-[n] elements is not equal to the value of the
[0] element, the last column will be used to make up the difference.
:li.The application can choose to use the default sizing method by
returning FALSE from the :MCLBN_CUSTOMSIZE. control message.
:eol.
:eul.
.*********************************************************************************************
:h1 res=101.MCLBINFO Data Structure
The MCLBINFO structure describes the characteristics of an MCLB control.
This structure is created and filled in by an application prior to
calling the :MCLBCreateWindow. API.  It is also returned to the
application with the current values when the application sends
a :MCLB_QUERYINFO. message.
:p.
:xmp.
typedef struct _MCLBINFO { 
  ULONG  Size;                    /* Length of this structure                   */
  char   *Titles;                 /* Title strings (TabChar separated)          */
  char   TitleFont[MAX_FONTLEN];  /* Title font (null to inherit from owner)    */
  char   ListFont[MAX_FONTLEN];   /* List  font (null to inherit from owner)    */
  ULONG  TitleBColor;             /* Title background color (RGB)               */
  ULONG  TitleFColor;             /* Title foreground color (RGB)               */
  ULONG  ListBColor;              /* List  background color (RGB)               */
  ULONG  ListFColor;              /* List  foreground color (RGB)               */
  LONG   *InitSizes;              /* Ptr to array of initial sizes              */
  char   _Reserved[64];           /* Reserved for future use                    */
  USHORT Cols;                    /* Number of columns                          */
  char   TabChar;                 /* Data column separator character            */
  char   _Padd;                   /* Padd for separator character (zero)        */
} MCLBINFO;
:exmp.
.*---------------------------------------------------------------------------
.* IPFCPREP macros used for message reference entries
.*---------------------------------------------------------------------------
.dm message on.
:h2 res=&res..&name.
.dm off
.*----------------
.dm params on
:parml.
.dm off
.*----------------
.dm param1 on
:pt.:hp6.Param1:ehp6.
.br
:parml.
.dm off
.*----------------
.dm param2 on
:eparml.
:pt.:hp6.Param2:ehp6.
.br
:parml.
.dm off
.*----------------
.dm item on
:pt.:hp2.&name.:ehp2. :hp1.(&type.):ehp1.
:pd.
.dm off
.*----------------
.dm returns on
:eparml.
:pt.:hp6.Returns:ehp6.&rbl.
.br
:parml.
.dm off
.*----------------
.dm eparams on
:eparml.
:eparml.
.dm off
.*----------------
.dm remarks on
:p.
:h3.:hp5.Remarks:ehp5.
.br
.dm off
.*----------------
.dm related on
:p.
:h3.:hp5.Related:ehp5.
.br
.dm off
.*----------------
.dm defproc on
:p.
:h3.:hp5.Default Processing:ehp5.
.br
.dm off
.*----------------
.dm emessage on
.dm off
.*----------------
.*-----------------------------------------------------------------------------------------------
:h1 res=500 toc=12.WM_CONTROL Message Reference
.*-----------------------------------------------------------------------------------------------
This section describes all the WM_CONTROL messages generated by a MCLB control and
sent to the control owner.  Note that most of the standard LN_* messages are
the same as for standard PM listbox controls.  Only LN_* messages that  differ from
the PM definitions are shown here.
:p.
For some WM_CONTROL messages it may be necessary for the application to know which
column of the MCLB generated the message.  The application may :hp1.send:ehp1. the
:MCLB_QUERYCTLCOL. message to get the number of the column which caused the current
WM_CONTROL message.  Note that :MCLB_QUERYCTLCOL. must :hp1.only:ehp1. be sent during
the processing of a WM_CONTROL message.
.*-----------------------------------------------------------------------------------------------
:message res=904 name='LN_*'.
The following WM_CONTROL listbox notification messages retain their normal PM meanings.  Note
that the control ID in SHORT 1 of mp1, and the window handle in mp2 are those of
the MCLB control itself.
:xmp.
   LN_ENTER
   LN_KILLFOCUS
   LN_SETFOCUS
   LN_SCROLL
   LN_SELECT
:exmp.
.*-----------------------------------------------------------------------------------------------
:message res=900 name='MCLBN_COLSIZED' .
This WM_CONTROL notification is sent by the MCLB to its owner when
the user has moved a column divider.
:params.
:param1.
:item name='MCLBID' type='USHORT'.
ID of the MCLB control
:item name='NotifyCode' type='USHORT'.
MCLBN_COLSIZED
:param2.
:item name='ColNum' type='USHORT'.
Number of the column to the left of the moved divider.  Note that this
column and the ColNum+1 column have both changed size.
:returns.
Nothing
:eparams.
:remarks.
This message occures only when the user moves a column divider in the MCLB.
It does not occure when the size of the MCLB control is changed.  The
:MCLB_QUERYCOLSIZES. message can be used to find the new column sizes.
:related.
:MCLBS_NOCOLRESIZE. style
.br
:MCLB_QUERYCOLSIZES. message
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=901 name='MCLBN_CUSTOMSIZE'.
This WM_CONTROL notification is sent by the MCLB to its owner when
the control window size has been changed and the control has the
:MCLBS_SIZEMETHOD_CUSTOM. style.
:params.
:param1.
:item name='MCLBID' type='USHORT'.
ID of the MCLB control
:item name='NotifyCode' type='USHORT'.
MCLBN_CUSTOMSIZE
:param2.
:item name='Sizes' type='LONG *'.
Pointer to array of LONG values.  The array has N+1 elements where
N is the number of columns.  The [0] element contains the new
width the columns must fit into.  The [1] to [N] elements contain
the current column sizes in pixels.
:returns.
:item name='Processed' type='BOOL'.
Return TRUE if the application has updated the [1] to [N] elements
of the array with the new column widths.  Return FALSE for the
MCLB control to proportionally resize the columns.
:eparams.
:remarks.
This message occures only if the :MCLBS_SIZEMETHOD_CUSTOM. style is used.
The application can choose to process this message and return TRUE, or
return FALSE and let the MCLB control perform default proportional
column sizing.
:p.
If the application returns TRUE, the [1] to [N] elements of the array
must be updated with the new adjusted column sizes.  The sum of the
[1] to [N] elements should be equal to the supplied [0] element value.
E.g. the new column sizes must add up to the new width allocated for
the columns.
:related.
:MCLBS_SIZEMETHOD_CUSTOM. style
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=902 name='MCLBN_PPCHANGED'.
This WM_CONTROL notification is sent by the MCLB to its owner when
any presentation parameter is altered.
:params.
:param1.
:item name='MCLBID' type='USHORT'.
ID of the MCLB control
:item name='NotifyCode' type='USHORT'.
MCLBN_PPCHANGED
:param2.
:item name='ColNum' type='USHORT'.
Number of the column that originated the presentation
parameter changes or zero if the title area was changed.
Note that all columns will have the same presentation parameters
(e.g. all columns will use the same font and colors).
:item name='PPType' type='USHORT'.
Indicates the type of presentation parameter that was changed:
:xmp.
  MCLBPP_FONT         Font changed
  MCLBPP_FORECOLOR    Foreground color changed
  MCLBPP_BACKCOLOR    Background color changed
:exmp.
:returns.
Nothing
:eparams.
:remarks.
The :MCLB_QUERYINFO. message can be used to determine the current
fonts and colors of the MCLB.
:related.
:MCLB_QUERYINFO.
:emessage.

.*-----------------------------------------------------------------------------------------------
:h1 toc=12.Owner-Draw Messages
.*-----------------------------------------------------------------------------------------------
This section describes messages the owner of an MCLB will receive
if the LS_OWNERDRAW style is used.
.*---------------------------------------------------------------------------
:message res=501 name='WM_MEASUREITEM'.
This message is sent to the owner of an owner-draw style MCLB to determine
the size of a listbox item.  This message will occure for each item of each
column of the listbox.  The measurements returned must be for just one
column, not the aggregate size of all columns.
:p.
This standard PM message is extended for an MCLB to include the column
number in SHORT 2 of mp1 (unused by PM).
:params.
:param1.
:item name='MCLBId' type='SHORT'.
ID of the MCLB control.
:item name='Col' type='USHORT'.
Column number of the item to be measured.
:param2.
:item name='ItemIndex' type='SHORT'.
Index of the item to be measured.
:returns.
:item name='Height' type='SHORT'.
Height of the item.
:item name='Width' type='SHORT'.
Width of the item.
:eparams.
:remarks.
The application should be prepared to receive this message when the
MCLB control is created (e.g. during :MCLBCreateWindow. processing).
:p.
Note that the vertical height
of all items in all columns must be the same, and this value must
be equal to or greater than the vertical font size for the listbox items.
:p.
The width must be the actual width of the item in the
column.  If the column
contains text the width returned should be the actual drawn
text width.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=502 name='WM_DRAWITEM'.
This message is sent to the owner of an owner-draw style MCLB when any
item in any column needs to be redrawn.  Note that this draw message
applies to just a single item in a single column, not to an entire
row.
:p.
This standard PM message is extended for an MCLB to include the column
number in SHORT 2 of mp1 (unused by PM).
:params.
:param1.
:item name='MCLBId' type='SHORT'.
ID of the MCLB control.
:item name='Col' type='USHORT'.
Column number of the item to be drawn.
:param2.
:item name='DrawInfo' type='OWNERITEM *'.
Pointer to an OWNERITEM data structure.
:returns.
:item name='Drawn' type='BOOL'.
Return TRUE if the application has drawn the item.  Return
FALSE for the listbox to draw the item text.
:eparams.
:remarks.
:related.
:emessage.
.*-----------------------------------------------------------------------------------------------
:h1 toc=12.MCLB Messages
.*-----------------------------------------------------------------------------------------------
This section describes all the messages an application can send to a MCLB control.
Unless noted otherwise here, all the PM listbox LM_* messages work as documented
for a listbox control.  Also unless stated otherwise, item text consists of
all the column data separated with the column separator character.
.*---------------------------------------------------------------------------
:message res=1100 name='LM_INSERTITEM'.
This standard listbox message is extended to include the column number
by which the inserted item is to be sorted.  SHORT 2 of mp1 is unused
by PM, but is defined for an MCLB as the column number by which sorting is
to be done.  If it is zero or greater than the number of columns,
LIT_ERORR will be returned.
:p.
It is recomended that if many items are to be inserted in the MBLB,
that WinEnableWindowUpdate() be used to improve performance.  Window
updating should be disabled before the first insert and enabled after
the last insert.
:params.
:param1.
:item name='InsertIndex' type='SHORT'.
Position in the list at which to insert, or LIT_END, LIT_SORTASCENDING, or
LIT_SORTDECENDING.
:item name='SortCol' type='USHORT'.
If InsertIndex is LIT_SORTASCENDING or LIT_SORTDECENDING then the
specified column number is used to determine the sorting sequence.
:param2.
:item name='Text' type='char *'.
Pointer to string to be inserted.  Column data is separated in the
string by the column separator charcter defined in :MCLBINFO. when the
MCLB was created.
:returns.
:item name='InsertedAt' type='SHORT'.
Index of newly inserted item or LIT_MEMERROR, or LIT_ERROR.
:eparams.
:remarks.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=1101 name='LM_INSERTMULTITEMS'.
:hp1.(Note: This is a new message in OS/2 Warp.):ehp1.
:p.
This standard listbox message is extended to include the column number
by which the inserted items are to be sorted.  The :hp2.reserved2:ehp2. field
of the LBOXINFO structure is used  to define
the column number by which sorting is
to be done.  If it is zero or greater than the number of columns,
LIT_ERORR will be returned.
:p.
Note that due to the implementation of the MCLB, there is little performance
difference between using this message to insert items and repeated use
of LM_INSERTITEM.
:params.
:param1.
:item name='Info' type='LBOXINFO *'.
Pointer to LBOXINFO structure (new in Warp 3.0 toolkit).
:param2.
:item name='Strings' type='char **'.
Pointer to array of string pointers.  Each string represents one
row of the MCLB using the column separator character to separate
the data into columns.
:returns.
:item name='InsertedAt' type='SHORT'.
Index of newly inserted item or LIT_MEMERROR, or LIT_ERROR.
:eparams.
:remarks.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=1102 name='LM_SEARCHSTRING'.
:hp1.This message is not fully implemented for an MCLB control.:ehp1.
The search is performed only on the first column.
:emessage.
.*---------------------------------------------------------------------------
:message res=910 name='MCLB_SETTITLES' .
This message is sent to the MCLB to set the title strings (column headers).
:params.
:param1.
:item name='Titles' type='* char'.
Pointer to a character string which contains the new column titles.  The
titles must be separated by the MCLB column separator character as defined
in the :MCLBINFO. structure when the MCLB was created.
:param2.
Reserved
:returns.
Nothing
:eparams.
:remarks.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=911 name='MCLB_SETTITLEFONT'.
This message is sent to the MCLB to set the font to be used for the titles.
:params.
:param1.
:item name='FontNameSize' type='* char'.
Pointer to a character string which contains the new font to be used.
The format of the string is the same as that used in the WinSetPresParam() call
such as "8.Helv".  This parameter may be NULL in which case the title
font is reset to the default font.
:param2.
Reserved
:returns.
Nothing
:eparams.
:remarks.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=912 name='MCLB_SETLISTFONT'.
This message is sent to the MCLB to set the font to be used for the list items.
:params.
:param1.
:item name='FontNameSize' type='* char'.
Pointer to a character string which contains the new font to be used.
The format of the string is the same as that used in the WinSetPresParam() call
such as "8.Helv".  This parameter may be NULL in which case the list
font is reset to the default font.
:param2.
Reserved
:returns.
Nothing
:eparams.
:remarks.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=913 name='MCLB_SETTITLECOLORS'.
This message is sent to the MCLB to set the colors to be used for the titles.
:params.
:param1.
:item name='Foreground' type='LONG'.
Contains the RGB value of the foreground color.
:param2.
:item name='Background' type='LONG'.
Contains the RGB value of the background color.
:returns.
Nothing
:eparams.
:remarks.
If the foreground and background colors specified are the same,
the title colors are reset to the default colors.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=914 name='MCLB_SETLISTCOLORS'.
This message is sent to the MCLB to set the colors to be used for the list items.
:params.
:param1.
:item name='Foreground' type='LONG'.
Contains the RGB value of the foreground color.
:param2.
:item name='Background' type='LONG'.
Contains the RGB value of the background color.
:returns.
Nothing
:eparams.
:remarks.
If the foreground and background colors specified are the same,
the list colors are reset to the default colors.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=915 name='MCLB_SETCOLSIZES'.
This message is sent to the MCLB to set the columns to specific sizes.
:params.
:param1.
:item name='NewSizes' type='LONG *'.
Pointer to array of LONG values.  Array must contain at least
N elements, where N is the number of columns in the MCLB.
:param2.
Reserved
:returns.
Nothing
:eparams.
:remarks.
The value of the [0] to [N-1] elements of the array must add up
to the current width of all the columns.  The current sum of the column
widths can be deterined with the :MCLB_QUERYFULLSIZE. message.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=916 name='MCLB_QUERYCOLSIZES'.
This message is sent to the MCLB to query the current size of the columns.
:params.
:param1.
:item name='Sizes' type='LONG *'.
Pointer to array of LONG values.  Array must contain at least
N elements, where N is the number of columns in the MCLB.
:param2.
Reserved
:returns.
Nothing
:eparams.
:remarks.
The value of the [0] to [N-1] elements of the array will
be set to the current column widths in pixels.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=917 name='MCLB_QUERYINFO'.
This message is sent to the MCLB to query the current MCLB configuration.
:params.
:param1.
:item name='Info' type='* MCLBINFO'.
Pointer to :MCLBINFO. structure.  Upon return the structured will
be filled with the current values. 
:param2.
Reserved
:returns.
Nothing
:eparams.
:remarks.
The value of the :hp1.InitSizes:ehp1. array will be set to the
current column sizes.
:p.
:hp1.NOTE&colon.:ehp1. The application must free the :hp1.InitSizes:ehp1.
array and the :hp1.Titles:ehp1. string.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=918 name='MCLB_QUERYSTYLE'.
This message is sent to the MCLB to query the MCLB styles flags.
:params.
:param1.
Reserved
:param2.
Reserved
:returns.
:item name='Styles' type='ULONG'.
Only the MCLB-specific style bits (MCLBS_* flags) will be returned.
:eparams.
:remarks.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=919 name='MCLB_QUERYFULLSIZE'.
This message is sent to the MCLB to query the sum of the column widths
:params.
:param1.
Reserved
:param2.
Reserved
:returns.
:item name='Width' type='LONG'.
The sum of the current column widths.
:eparams.
:remarks.
Note that this value is not the same as the width of the control window.
This value is the width of the control minus the width of all dividers,
vertical scroll bars, borders, etc.  It is the number of pixels actually
allocated for the columns of data.
:related.
:emessage.
.*---------------------------------------------------------------------------
:message res=920 name='MCLB_QUERYCTLCOL'.
This message is sent to the MCLB to query column which caused the current :WM_CONTROL. message.
:params.
:param1.
Reserved
:param2.
Reserved
:returns.
:item name='Col' type='USHORT'.
The number of the column which caused the current :WM_CONTROL. message.
:eparams.
:remarks.
This message must be :hp1.sent:ehp1. to the MCLB, not posted.  The results
of this message are only defined during :WM_CONTROL. message processing.
:p.
It can be useful at times to know which column of an MCLB has caused a
particular :WM_CONTROL. message to be sent to the owner.  The owner can
discover this by sending the MCLB this message.  This message must be
sent while the owner is processing the :WM_CONTROL. message of interest.
:related.
:emessage.
.*---------------------------------------------------------------------------
.* API Reference
.*---------------------------------------------------------------------------
:h1.Programming Interfaces (API)
This section describes each of the callable MCLB programming interfaces.
.*---------------------------------------------------------------------------
.* IPFCPREP macros used for message reference entries
.*---------------------------------------------------------------------------
.dm function on
:h2 res=&res..&name.
.dm off
.*----------------
.dm syntax on
:p.
:hp7.Syntax&colon.:ehp7.
.br
:lines.
:font facename='System Monospaced' size=12x10.
:hp4.&return. &name.(&params.):ehp4.
:font facename=default size=0x0.
:elines.
.dm off
.*----------------
.dm fparams on
:font facename='System Monospaced' size=12x10.
:table cols='15 10 10 50' rules=both frame=box.
:row.
:c.:hp1.Name:ehp1.
:c.:hp1.Type:ehp1.
:c.:hp1.In/Out:ehp1.
:c.:hp1.Description:ehp1.
.dm off
.*----------------
.dm fparam on
:row.
:c.&name.
:c.&type.
:c.&io.
:c.
.dm off
.*----------------
.dm freturns on
:etable.
:font facename=default size=0x0.
:hp1.Returns:ehp1.
:font facename='System Monospaced' size=12x10.
:table cols='15 10 10 50' rules=both frame=box.
.dm off
.*----------------
.dm efparams on
:etable.
:font facename=default size=0x0.
.dm off
.*----------------
.dm efunction on
.dm off
.*----------------
.*--------------------------------------------------------------------------
.*-------------------------------------------------------------------------
:function res=1000 name='MCLBCreateWindow' text='Create an MCLB Control'.
This function creates an MCLB and returns its window handle.
:syntax name='MCLBCreateWindow' params='Parent, Owner, Style, x, y, cx, cy, Behind, Id, MCLBInfo' return='MCLBHwnd'.
:fparams.
:fparam name='Parent' type='HWND' io='input'.
Handle of the parent window.
:fparam name='Owner' type='HWND' io='input'.
Handle of the owner window.  This window will recieve :WM_CONTROL. messages
from the MCLB.
:fparam name='Style' type='ULONG' io='input'.
Style flags.  This is a logical-OR combination of WS_* window styles,
LS_* listbox styles, and :link reftype=hd res=400.MCLBS_* MCLB styles:elink..
:fparam name='x' type='LONG' io='input'.
X position of the window relative to the Parent window.
:fparam name='y' type='LONG' io='input'.
Y position of the window relative to the Parent window.
:fparam name='cx' type='LONG' io='input'.
Width of the window.
:fparam name='cy' type='LONG' io='input'.
Height of the window.
:fparam name='Behind' type='HWND' io='input'.
Sibling window behind which this window is to be placed.  HWND_TOP or
HWND_BOTTOM can be used to place this window on top of, or behind all
other sibling windows.
:fparam name='Id' type='USHORT' io='input'.
Id of this window.  This Id is encoded in all :WM_CONTROL. messages from this MCLB.
:fparam name='Info' type='MCLBINFO *' io='input'.
Pointer to completed :MCLBINFO. data structure.
:freturns.
:fparam name='MCLBHwnd' type='HWND' io='return'.
Handle of the MCLB window, or NULLHANDLE if creation failed.
:efparams.
:remarks.
:related.
:MCLBINFO.
:euserdoc.

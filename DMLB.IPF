.***************************************************************************
.* DMLB toolkit online reference.
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
.dm DMLBInitialize on
:link reftype=hd res=400.DMLBInitialize:elink.
.dm off

.dm Resources on
:link reftype=hd res=700.DMLB Resources:elink.
.dm off

.dm LN_DMLB_QRYDROP on
:link reftype=hd res=906.LN_DMLB_QRYDROP:elink.
.dm off

.dm LN_DMLB_INSERT_COPY on
:link reftype=hd res=905.LN_DMLB_INSERT_COPY:elink.
.dm off

.dm LN_DMLB_INSERT_MOVE on
:link reftype=hd res=904.LN_DMLB_INSERT_MOVE:elink.
.dm off

.dm LN_DMLB_REORDERED on
:link reftype=hd res=903.LN_DMLB_REORDERED:elink.
.dm off

.dm LN_DMLB_DELETE_MOVE on
:link reftype=hd res=902.LN_DMLB_DELETE_MOVE:elink.
.dm off

.dm LN_DMLB_DELETE on
:link reftype=hd res=901.LN_DMLB_DELETE:elink.
.dm off

.dm LN_DMLB_CONTEXT on
:link reftype=hd res=900.LN_DMLBN_CONTEXT:elink.
.dm off
.***************************************
:title.Direct Manipulation ListBox (DMLB)
.***************************************
:h1. Introduction
This document is the programmer's reference for the Direct Manipulation ListBox (DMLB)
control.
For more information on programming with this
control, see :hp1.OS/2 Developer Magazine:ehp1., Nov/Dec 1995.
:p.
:artwork align=center name='pic2.bmp'.
:p.
:hp7.Acknowledgments:ehp7.
.br
This control was originally conceived at IBM Yorktown Research by
Alan Warren. The control was rewritten
and enhanced by Mark McMillan of IBM, Research Triangle Park, USA.
:p.
:hp7.Description:ehp7.
.br
The :hp3.Direct Manipulation ListBox:ehp3. is a very useful
enhancement for the standard PM listbox control.  It supplies the
capability to support drag/drop reordering of items in a listbox,
and drag/drop of items from one listbox to another.

.*---------------------------------------------------------------------------
:h1.Using the DMLB
To add direct-manipulation capability to a standard listbox (or an MCLB),
the listbox or MCLB should be created in the usual way.  After the listbox
control is created, the :DMLBInitialize. function is called to establish
the DMLB subclass.
:p.
The application must define the following resources in its .RC file:
:xmp.
#include "dmlb.h"
POINTER ID_DMLB_DRAGMOVE "lbdrag.ptr"
POINTER ID_DMLB_DRAGCOPY "lbdragcp.ptr"
POINTER ID_DMLB_DRAGNONE "nodrag.ptr"
POINTER ID_DMLB_DRGNORTH "drgnorth.ptr"
POINTER ID_DMLB_DRGSOUTH "drgsouth.ptr"
POINTER ID_DMLB_DRGDEL   "drgdel.ptr"
:exmp.
.br
If these resources reside an a DLL, the DLL module handle must be
supplied on the :DMLBInitialize. call.
:p.
Once the DMLB subclass is established, the application must, at a
minimum, respond to the :LN_DMLB_QRYDROP. message.  It may choose
to also process other DMLB notification messages.
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
.dm sample on
:p.
:h3.:hp5.Example:ehp5.
.br
:xmp.
:font facename='System Monospaced' size=12x10.
.dm off
.*----------------
.dm esample on
:font facename=default size=0x0.
:exmp.
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
This section describes all the WM_CONTROL messages generated by a DMLB subclassed control and
sent to the control owner.  Note that all of the standard LN_* messages are
generated from the listbox as usual.
.*-----------------------------------------------------------------------------------------------
:message res=900 name='LN_DMLB_CONTEXT'.
This WM_CONTROL notification is sent by the DMLB to its owner when
the user has requested a context menu on the listbox.
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_CONTEXT
:param2.
:item name='Index' type='SHORT'.
Index of the item for which the context menu was requested.  This value
will be LIT_NONE if the pointer was not over an item when the menu
was requested.
:returns.
Nothing
:eparams.
:remarks.
The owner can respond by poping up a context menu at the current pointer
coordinates or it can ignore this message if context menus are not
supported.
:sample.
case WM_INITDLG: // or WM_CREATE
  ContextHwnd = WinLoadMenu(...);
  ...

case WM_CONTROL:                                          
  switch SHORT1FROMMP(mp1) {                              
    case ID_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {

        case LN_DMLB_CONTEXT: {
          /* The user has requested a context menu.  Short 1 of mp2 is the */
          /* index of the listbox item under the pointer.                  */
          POINTL Point;
          SHORT  ContextIndex;

          ContextIndx = SHORT1FROMMP(mp2);

          /* Popup the context menu at the pointer position. */
          WinQueryMsgPos((HAB)0, &amp.Point);
          WinPopupMenu(HWND_DESKTOP, hwnd, ContextHwnd, Point.x, Point.y,
                                          0, PU_HCONSTRAIN| PU_VCONSTRAIN
                      PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 | PU_MOUSEBUTTON3
                                                        | PU_KEYBOARD );
          return (MRESULT)TRUE;
          }
      }                           
      break;                      
  }                               
  break; /* End of WM_CONTROL */          
:esample.
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=901 name='LN_DMLB_DELETE'.
This WM_CONTROL notification is sent to the owner of the :hp2.source:ehp2. listbox when
a listbox item is about to be deleted as a result of a drag/drop operation.
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_DELETE
:param2.
:item name='Target' type='HWND'.
The window handle of the target listbox on which the drop
has occured.
:returns.
Nothing
:eparams.
:remarks.
This message occures when a listbox item is dropped on a listbox
which returned DROPMODE_DELETE in response to a :LN_DMLB_QRYDROP.
message.  The listbox on which the drop occured is given in mp2.
The item to be deleted is the currently selected item of the
listbox.
:p.
Note that this message does :hp2.not:ehp2. occure when an item
is :hp1.moved:ehp1. from one listbox to another.  It occures
only if the item is to be deleted.
:p.
This message is sent :hp2.before:ehp2. the item is deleted giving
the owner a chance to free any dynamic resources that may be
associated with this item.
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=902 name='LN_DMLB_DELETE_MOVE'.
This WM_CONTROL notification is sent to the owner of the :hp2.source:ehp2. listbox when
a listbox item is about to be moved to another listbox as a result of a drag/drop operation.
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_DELETE_MOVE
:param2.
:item name='Target' type='HWND'.
The window handle of the target listbox on which the drop
has occured.
:returns.
Nothing
:eparams.
:remarks.
This message occures when a listbox item is dropped on a listbox
which returned DROPMODE_MOVE in response to a :LN_DMLB_QRYDROP.
message.  The listbox on which the drop occured is given in mp2.
The item to be moved is the currently selected item of the
listbox.
:p.
Note that this message does :hp2.not:ehp2. occure when an item
is :hp1.moved:ehp1. within the same listbox (see :LN_DMLB_REORDERED.).  It occures
only if the item is moved from this (source) listbox to another
(target) listbox.  When an item is moved from one listbox to
another, the item text and handle are both copied to the target
listbox and then the item is deleted from the source listbox.
:p.
This message is sent :hp2.before:ehp2. the item is deleted from the
source listbox.
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=903 name='LN_DMLB_REORDERED'.
This WM_CONTROL notification is sent by the DMLB to its owner when
a listbox item is moved within the same listbox (the listbox items are reordered).
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_REORDERED
:param2.
:item name='Target' type='HWND'.
The window handle of listbox
:returns.
Nothing
:eparams.
:remarks.
This message occures when a listbox item is dragged and dropped
on the same listbox.  The moved item is the currently
selected item.
:p.
When an item is moved the item text and item handle are copied, the
item is deleted, and then the item is reinserted at a new position.
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=904 name='LN_DMLB_INSERT_MOVE'.
This WM_CONTROL notification is sent to the owner of the :hp2.target:ehp2. listbox when
a listbox item inserted by being moved from another listbox.
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_INSERT_MOVE
:param2.
:item name='Target' type='HWND'.
The window handle of the source listbox
:returns.
Nothing
:eparams.
:remarks.
This message occures when a listbox item is dragged from another
listbox and dropped on this listbox.  This listbox must
responded with DROPMODE_MOVE to :LN_DMLB_QRYDROP. to receive
this message.
:p.
This message is sent after the insert has been completed.  The
inserted item is the currently selected item.  This message
occures before the item is deleted from the source listbox.
Both the item text and item handle are copied from the source
listbox.
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=905 name='LN_DMLB_INSERT_COPY'.
This WM_CONTROL notification is sent to the owner of the :hp2.target:ehp2. listbox
when a listbox item inserted by being copied from another listbox.
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_INSERT_COPY
:param2.
:item name='Target' type='HWND'.
The window handle of the source listbox
:returns.
Nothing
:eparams.
:remarks.
This message occures when a listbox item is dragged from another
listbox and dropped on this listbox.  This listbox must
responded with DROPMODE_COPY to :LN_DMLB_QRYDROP. to receive
this message.
:p.
This message is sent after the insert has been completed.  The
inserted item is the currently selected item.  
Both the item text and item handle are copied from the source
listbox.  The source listbox item is not deleted.
:emessage.
.*-----------------------------------------------------------------------------------------------
:message res=906 name='LN_DMLB_QRYDROP'.
This WM_CONTROL notification is sent to the owner of the :hp2.target:ehp2. listbox
when a listbox item is dragged over it.
:params.
:param1.
:item name='DMLBID' type='USHORT'.
ID of the DMLB listbox
:item name='NotifyCode' type='USHORT'.
LN_DMLB_QRYDROP
:param2.
:item name='Target' type='HWND'.
The window handle of the source listbox
:returns.
:item name='DropAllowed' type='BOOL'.
If FALSE is returned then no drop is allowed and the pointer will
be set to the "no drop" shape.  If TRUE is returned then a drop
is allowed and the pointer shape is set according to the
:hp1.DropType:ehp1. value.
:item name='DropType' type='SHORT'.
If :hp1.DropAllowed:ehp1. is FALSE then this value is ignored.  Otherwise
this value indicates what action will be taken if the user completes
the drop on this listbox.  The possible values are:
:ul.
:li.:hp2.DROPMODE_MOVE:ehp2. The item will be inserted into the current
listbox and deleted from the source listbox.  If the source and target
listboxs are same a :LN_DMLB_REORDERED. notification message will be sent.  If
they are different listboxes, :LN_DMLB_INSERT_MOVE. will be sent to the
target and :LN_DMLB_DELETE_MOVE. will be sent to the source.
:li.:hp2.DROPMODE_MOVE:ehp2. The item will be inserted into the current
listbox.  It will not be deleted from the source listbox.  If the source and target
listboxs are same the item will be duplicated in the listbox. A
:LN_DMLB_INSERT_COPY. notification message is sent to the target listbox.
No notification is sent to the source.
:li.:hp2.DROPMODE_DELETE:ehp2. The item will be deleted from the source
listbox.  It will not be inserted in the target listbox.  The source
listbox will receive a :LN_DMLB_DELETE. notification message.
:eul.
:eparams.
:remarks.
This message is sent to the target listbox while a drag is in progress
over the listbox.  The application must respond by indicating if
a drop is allowed or not, and if so, what type of drop should occure
(copy, move, or delete).  The pointer shape will be changed to reflect
the result of this message and if :hp1.NoDrop:ehp1. is TRUE, no
drop will be allowed.
:p.
Note that this message does occure when a listbox item is dragged
on its own listbox.
:sample.
  case WM_CONTROL:                                          
    switch SHORT1FROMMP(mp1) {                              
      case ID_LISTBOX:
        switch (SHORT2FROMMP(mp1)) {

          case LN_DMLB_QRYDROP:
            // If dropping in same listbox, move the item.  If drop is
            // from some other listbox, refuse it.

            if (HWNDFROMMP(mp2) == WinWindowFromID(hwnd, ID_LISTBOX))
              return MRFROM2SHORT(TRUE, DROPMODE_MOVE);

            return MRFROM2SHORT(FALSE, 0);

            ...
:esample.
:emessage.
.*---------------------------------------------------------------------------
:h1 res=700.DMLB Resources
.*---------------------------------------------------------------------------
The DMLB subclass requires access to several POINTER type resources
to use during drag/drop operations.  The application must have the
following statements in its resource (.RC) file:
:xmp.
  #include "dmlb.h"
  POINTER ID_DMLB_DRAGMOVE "lbdrag.ptr"
  POINTER ID_DMLB_DRAGCOPY "lbdragcp.ptr"
  POINTER ID_DMLB_DRAGNONE "nodrag.ptr"
  POINTER ID_DMLB_DRGNORTH "drgnorth.ptr"
  POINTER ID_DMLB_DRGSOUTH "drgsouth.ptr"
  POINTER ID_DMLB_DRGDEL   "drgdel.ptr"
:exmp.
.br
As supplied in the toolkit, these resources use POINTER IDs 9000 through 9005.
Application defined POINTER resources must not conflict with these ID values.
:p.
If these resources are defined in a DLL then the DLL module handle must be
supplied on the :DMLBInitialize. call.
.*---------------------------------------------------------------------------
.* IPFCPREP macros used for API reference entries
.*---------------------------------------------------------------------------
.dm function on
:h1 res=&res..&name.
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
:function res=400 name='DMLBInitialize' text='Establish and initialize DMLB subclass'.
This function establishes a DMLB subclass on a standard PM listbox control.
:syntax name='DMLBInitialize' params='LBHwnd, ResHMod' return='BOOL'.
:fparams.
:fparam name='LBHwnd' type='HWND' io='input'.
Handle of the listbox to be subclassed.
:fparam name='ResHMod' type='HMODULE' io='input'.
Handle of the module (DLL) the :Resources. are to be
retrieved from.  This may be NULLHANDLE if the sources are bound
to the EXE.
:freturns.
:fparam name='Sucess' type='BOOL' io='return'.
TRUE if initialization sucessful, FALSE if it failed.
:efparams.
:remarks.
This function is called to setup a DMLB subclassing of a standard PM listbox
control.  Note that the QWL_USER window words of the listbox window will be
used by the subclass for a pointer to the subclass instance data.
After this function sucessfully completes, the
application may use the first PVOID of the DMLB instance data.
:p.
The application must have the :Resources. (pointers) available
in a resource DLL or bound to the executable module.  If the resources
reside in a DLL the DLL module handle must be supplied on this call.
:efunction.
:euserdoc.

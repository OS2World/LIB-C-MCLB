#define ID_DMLB_DRAGMOVE     9000        /* POINTER resource IDs */
#define ID_DMLB_DRAGCOPY     9001
#define ID_DMLB_DRAGNONE     9002
#define ID_DMLB_DRGNORTH     9003
#define ID_DMLB_DRGSOUTH     9004
#define ID_DMLB_DRGDEL       9005

/* WM_CONTROL notification codes from a DMLB.  Delete-type notifications are  */
/* sent before the item is deleted.  Insert-type notifications are sent after */
/* the insert has been completed.  The REORDER notification is sent after the */
/* item has been moved in the list.                                           */

#define LN_DMLB_DELETE         999      /* Item is about to be deleted (current selection) */
#define LN_DMLB_DELETE_MOVE    998      /* Item is to be deleted for moving to another listbox */
#define LN_DMLB_REORDERED      997      /* An item was moved within the same listbox */
#define LN_DMLB_INSERT_MOVE    996      /* Item was inserted via drag-MOVE (current selection) */
#define LN_DMLB_INSERT_COPY    995      /* Item was inserted via drag-COPY (current selection) */
#define LN_DMLB_QRYDROP        991      /* A drop is about to occure, return Allow/NoAllow and mode flags */
#define LN_DMLB_CONTEXT        990      /* A context menu was requested on the listbox */
 
/* Codes for reponding to a WM_CONTROL LN_SLB_QRYDROP notification message */
/* sent to owner of target of drag/drop operation                          */

#define DROPMODE_COPY   1               /* Copy the item from the source to this listbox */
#define DROPMODE_MOVE   0               /* Move the item from the source to this listbox (default) */
#define DROPMODE_DELETE 2               /* Delete source item, do not insert in this listbox */

BOOL EXPENTRY DMLBInitialize(           /* Function to setup DM capabilites on a listbox control */
                HWND LBHwnd,            /* Handle of listbox or MCLB window */
                HMODULE ResHMod);       /* Module to load pointer resources, NULLHANDLE if bound to EXE */

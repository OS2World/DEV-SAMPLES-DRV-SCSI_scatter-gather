#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include "os2.h"
#include "dos.h"
#include "bseerr.h"
#include "devclass.h"
#include "dskinit.h"
#include "strat2.h"
#include "dhcalls.h"
#include "reqpkt.h"
#include "aspi.h"
#include "asgext.h"

/* !!!  = value !!! */

PFN          Device_Help          =  0L;
ULONG        postSema             =  0L;   // (32 bit HEV)
ULONG        physBuffer           =  0L;   
BYTE         postInProgress       =  0;

VOID   (FAR *ProtIDCEntry)(VOID)  =  0;
USHORT       ProtIDC_DS           =  0;

ULONG        LockHandle           =  0L;
ULONG        SgLockHandle[N_SGLH] = {0L};
ULONG        SgLockCounter        =  0L;
ULONG        Hook1                =  0L;

/* Start of Initialization Data */

UCHAR        BeginInitData = 0;

#define      MSG_REPLACEMENT_STRING  1178
MSGTABLE     InitMsg = { MSG_REPLACEMENT_STRING, 1, 0 };

UCHAR        VersionMsg[] =
  "Asg (ASPI Scatter/Gather router) 1.0.\r\n";

NPSZ         DDName="SCSIMGR$";
IDCTABLE     IDCTable={{0,0,0},0,0};

UCHAR        IdentMsg[]="asg: ";
UCHAR        DD_OK_Msg[]="SCSIMGR$ found\n";
UCHAR        DD_ERR_Msg[]="SCSIMGR$ not found\n";

UCHAR        StringBuffer[256] = { 0 };

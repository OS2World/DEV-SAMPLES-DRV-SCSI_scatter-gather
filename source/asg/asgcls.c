#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#define INCL_DOSERRORS
#include "os2.h"
#include "dos.h"
#include "bseerr.h"
#include "devclass.h"
#include "dskinit.h"

#include "strat2.h"
#include "dhcalls.h"

#include "reqpkt.h"

#include "asgext.h"

USHORT FAR DriverOpen( PRP_OPENCLOSE  pRPH )
/*==========================================*/
{
   return  STATUS_DONE; 
}

USHORT FAR DriverClose( PRP_OPENCLOSE  pRPH )
/*==========================================*/
{
   if(postSema)DevHelp_CloseEventSem(postSema); postSema=0L;
   if(LockHandle)DevHelp_UnLock(LockHandle);    LockHandle=0L;
   return  STATUS_DONE; 
}



#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include "os2.h"
#include "dos.h"
#include "devcmd.h"
#include "dskinit.h"

#include "strat2.h"
#include "dhcalls.h"

#include "reqpkt.h"
#include "asgext.h"


/* Strategy routine */

VOID NEAR AsgStr()
/*==============*/
{  PRPH     pRPH; 
   USHORT   Cmd;

   _asm {
           mov word ptr pRPH[0], bx    /*  pRPH is initialized to       */
           mov word ptr pRPH[2], es    /*  ES:BX passed from the kernel */
        }


   pRPH->Status = STATUS_DONE;
   Cmd = pRPH->Cmd;

   if( Cmd == CMDInit )
    { pRPH->Status =  DriverInit( (PRPINITIN) pRPH );
    }
   else
   if( Cmd == CMDOpen )
    { pRPH->Status = DriverOpen( (PRP_OPENCLOSE) pRPH );
    }
   else
   if( Cmd == CMDGenIOCTL )
    { pRPH->Status = DriverIOCtl( (PRP_GENIOCTL) pRPH );
    }
   else
   if( Cmd == CMDClose )
    { pRPH->Status = DriverClose( (PRP_OPENCLOSE) pRPH );
    }  
   else
   if( Cmd == CMDDeInstall )
    { pRPH->Status = STATUS_DONE; // DriverxxOpen( (PRP_GENIOCTL) pRPH );
    }
   else
   if( Cmd == CMDShutdown )
    { pRPH->Status = STATUS_DONE; // DriverxxOpen( (PRP_GENIOCTL) pRPH );
    }
   else
   {  pRPH->Status =  STATUS_DONE | STATUS_ERR_UNKCMD;
   }

   _asm {
           leave
           retf
        }


} /* ASPIRStr */



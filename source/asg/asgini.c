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

USHORT FAR DriverInit( PRPINITIN  pRPH )
/*====================================*/
{  PRPINITIN                     pRPI = (PRPINITIN)  pRPH;
   PRPINITOUT                    pRPO = (PRPINITOUT) pRPH;
   USHORT                        rc,i,Q_Flag=0,V_Flag=0; 
   PSZ                           p;

   Device_Help = pRPH->DevHlpEP; /* DevHelp_Beep(440,500); */

   pRPO->CodeEnd    =  (USHORT) &DriverInit;
   pRPO->DataEnd    =  (USHORT) &BeginInitData; 
   pRPO->rph.Status = STDON;

   GetInitParms((PUCHAR)StringBuffer,pRPI);
   p=StringBuffer;
   for (i=0; *p!=0 && i<40; i++, p++)
     { if (*p=='/') { p++;
                      switch (*p) { case 'Q': Q_Flag=1; break;
                                    case 'V': V_Flag=1; break;
                                    deafult:;
                                  }
                    }
     }

   if(!Q_Flag) { sprintf((PUCHAR)StringBuffer,(PUCHAR)"%s",(PSZ)VersionMsg);
                 puts(StringBuffer);
               } 

   rc=DevHelp_AttachDD(DDName,(NPBYTE)&IDCTable);

   if(V_Flag||rc)
               { sprintf((PUCHAR)StringBuffer,(PUCHAR)"%s%s",(PSZ)IdentMsg,
                                          rc?(PSZ)DD_ERR_Msg:(PSZ)DD_OK_Msg);
                 puts(StringBuffer);
               }

   DevHelp_AllocateCtxHook((NPFN)ctx_hand,&Hook1);

   if(!rc)
     { ProtIDCEntry=IDCTable.ProtIDCEntry;
       ProtIDC_DS=IDCTable.ProtIDC_DS;
     }

   return  STATUS_DONE; 
}



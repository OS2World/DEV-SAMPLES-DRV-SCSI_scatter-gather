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
#include "devcmd.h"
#include "asgext.h"


VOID NEAR GetInitParms ( PUCHAR pCmdString, PRPINITIN pRP)
/* example: pCmdString ... "c:\arspir.sys /a /b /c" */
{
   PSZ    pCmdStringStart,p;
   int    i;   

   pCmdStringStart = pRP->InitArgs;

   if (pRP->rph.Cmd == CMDInitBase)
      OFFSETOF(pCmdStringStart)=((PDDD_PARM_LIST)pRP->InitArgs)->cmd_line_args;

   /*
   ** Fold characters to upper case
   */
   for (i=0,p=pCmdString; *pCmdStringStart!=0 && i<40; i++, pCmdStringStart++)
   {
      if (*pCmdStringStart >= 'a' &&  *pCmdStringStart <= 'z')
         *p++ = *pCmdStringStart - ('a' - 'A');
      else
         *p++ = *pCmdStringStart;
   }
   *p=0;
}

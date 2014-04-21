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
#include "aspi.h"
#include "asgext.h"

VOID FAR ctx_hand(void)
{
  DevHelp_PostEventSem(postSema);
}


VOID asgPost(ULONG SRBPointer)
{ 
  DevHelp_ArmCtxHook(0L, Hook1); 
}

VOID FAR postEntry(USHORT,ULONG);


USHORT FAR DriverIOCtl( PRP_GENIOCTL pRPH )
/*=======================================*/
{ if(pRPH->Category != IOCTLCAT)return STATUS_DONE;
  
  switch (pRPH->Function)
     { case 0x02: { USHORT pSRB_Seg,pSRB_Off;
                    PVOID  pSem=(PVOID)&postSema;  
                    PASPI_SRB_EXECUTE_IO pSRB=(PASPI_SRB_EXECUTE_IO)
                                                     pRPH->ParmPacket;
                    ULONG  asgPostAddr;
                    USHORT codeseg=FP_SEG(asgPost);
                    asgPostAddr=(ULONG)(void FAR*)postEntry;
                    asgPostAddr=asgPostAddr+(codeseg<<16);  

                    pSRB->ppDataBuffer=physBuffer; 

                    *(PULONG)&(pSRB->PM_PostAddress)=asgPostAddr;
                    pSRB->PM_DataSeg=((PUSHORT)&pSem)[1];
                    DevHelp_VirtToPhys(pSRB,(PULONG)&(pSRB->ppSRB)); 

                    pSRB_Seg=((PUSHORT)&pSRB)[1];
                    pSRB_Off=((PUSHORT)&pSRB)[0];

                   _asm { push pSRB_Seg
                          push pSRB_Off
                          push ProtIDC_DS
                          call ProtIDCEntry
                          add sp,6
                        }  
                  }
                  break;
       case 0x03: { *(PULONG)&postSema=*((PULONG)pRPH->ParmPacket);
                    *((USHORT FAR*)pRPH->DataPacket)=
                                   DevHelp_OpenEventSem(postSema); 
                  }
                  break;
       case 0x04: { ULONG DATAPtr;
                    USHORT result,bufseg;
                   *(PULONG)&DATAPtr=(ULONG)(PULONG)pRPH->ParmPacket;
                   *(PUSHORT)&bufseg=FP_SEG(DATAPtr);
                    result=DevHelp_Lock((SEL)bufseg,1,0,&LockHandle);
                    DevHelp_VirtToPhys((BYTE FAR *)DATAPtr,
                                       (PULONG)&physBuffer);
                   *((USHORT FAR*)pRPH->DataPacket)=*((PUSHORT)&result);
                  } 
                  break;
       case 0x06: { PULONG d=(PULONG)(pRPH->DataPacket); 
                    ULONG DATAPtr,PhysA;
                    USHORT result,bufseg;
                   *(PULONG)&DATAPtr=(ULONG)(PULONG)pRPH->ParmPacket;
                   *(PUSHORT)&bufseg=FP_SEG(DATAPtr);
                    if(SgLockCounter<N_SGLH)
                     { result=DevHelp_Lock((SEL)bufseg,1,0,
                                       &(SgLockHandle[SgLockCounter]));
                       if(result)*d=0;
                       else 
                         { DevHelp_VirtToPhys((BYTE FAR *)DATAPtr,
                                                   (PULONG)&PhysA);
                           *d=PhysA;
                           SgLockCounter++; 
                         }
                     }
                    else *d=0; 
                  } 
                  break;
       case 0x07: { ULONG i;
                    for(i=0;i<SgLockCounter;i++)
                        { DevHelp_UnLock(SgLockHandle[i]);
                          SgLockHandle[i]=0L;
                        }
                    SgLockCounter=0;
                  }
                  break; 
       default:   break;
     }
  return  STATUS_DONE; 
}

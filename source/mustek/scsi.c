#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#define  INCL_DOSERRORS
#define  INCL_DOSFILEMGR
#define  INCL_DOS
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSSEMAPHORES
#define  INCL_DOSMEMMGR
#include <os2.h>

#include "srb.h"
#include "scsi.h"

#define DEBUG     0
#define DEBUG_SG  0
#define DEBUG_CMD 0


int scsi_debug_level      =  0;
int scsi_fd               = -1;
unsigned int scsi_pack_id =  0;
unsigned char scsi_sensebuffer[16];

int scsi_ha_num;            /* host adapter number                        */
int scsi_id;		    /* Scanner's SCSI ID #.                       */
HEV postSema = 0;	    /* Event Semaphore for posting SRB completion */
HFILE driver_handle = 0;    /* file handle for device driver              */
PVOID buffer = 0;	    /* Big data buffer.                           */ 

static int scsi_sg_actnum=SCSI_SG_DEFAULTNUM;

#ifdef USE_SG
struct  sg_list { unsigned char *p; int sze; };

#define SG_STEP        (SCSI_SG_BUFSIZE>0x8000?0x10000:0x8000)
#define SG_BASE        (0x10000)
#define BUF_ALLOC_SIZE (SG_BASE+SG_STEP*scsi_sg_actnum)
#define BUF_GLOB_SIZE  (SCSI_SG_SUMSIZE)
#define SG_ADDR(b,i)   (((unsigned char*)b)+SG_BASE+i*SG_STEP)  
#else
#define BUF_GLOB_SIZE  SCSI_BUFSIZE
#define BUF_ALLOC_SIZE SCSI_BUFSIZE
#endif

#define SCSI_ERR(string...) { fprintf (stderr, ##string); return(-1); }

#define SCSI_MSG(l,s...) { if(scsi_debug_level>=l){ fprintf(stderr, ##s); }}


/*------------------------------------------------------------------------*/
/* int block_sigint (void)                                                */
/* Disable handling of SIGINT.                                            */
/* Return: 0 if successful, -1 if there were errors.                      */
/*------------------------------------------------------------------------*/
int block_sigint (void) {
  sigset_t s;
  if (sigemptyset(&s) < 0) {
    perror("sigemptyset");
    return(-1);
  }
  if (sigaddset(&s, SIGINT) < 0) {
    perror("sigaddset");
    return(-1);
  }
  if (sigprocmask(SIG_BLOCK, &s, NULL) < 0) {
    perror("sigprocmask");
    return(-1);
  }
  return(0);
}


/*------------------------------------------------------------------------*/
/* int unblock_sigint (void)                                              */
/* Enable handling of SIGINT.                                             */
/* Return: 0 if successful, -1 if there were errors.                      */
/*------------------------------------------------------------------------*/
int unblock_sigint (void) {
  sigset_t s;
  if (sigemptyset(&s) < 0) {
    perror("sigemptyset");
    return(-1);
  }
  if (sigaddset(&s, SIGINT) < 0) {
    perror("sigaddset");
    return(-1);
  }
  if (sigprocmask(SIG_UNBLOCK, &s, NULL) < 0) {
    perror("sigprocmask");
    return(-1);
  }
  return(0);
}


/*------------------------------------------------------------------------*/
/* int scsi_open_device (char *scsi_device)                               */
/* Opens the device specified in scsi_device. If open fails, an error     */
/* message is printed to stderr, and the function returns with -1.  */
/* This happens if either the scsi_device was not set (NULL) or the       */
/* open() call failed, i.e. returned with a value < 0.                    */
/* If successful, returns the filedescriptor of the scsi device.          */
/*------------------------------------------------------------------------*/
int scsi_open_device (char *scsi_device) 
{
  ULONG rc;                  /* return value */
  ULONG ActionTaken;                 
  USHORT openSemaReturn;             
  USHORT lockSegmentReturn;          
  unsigned long cbreturn;
  unsigned long cbParam;

  if (!scsi_device)   /* Return if no SCSI device is specified */
  { fprintf(stderr, "scsi_open_device: no device specified\n");
    return(-1);
  }
  sscanf(scsi_device,"%i:%i",&scsi_ha_num,&scsi_id);
  if (scsi_id < 0 || scsi_id > 15) 
  {  fprintf(stderr, "scsi_open_device: bad SCSI ID %s\n", scsi_device);
     return (-1);
  } 
  /* Allocate data buffer. */
  rc = DosAllocMem(&buffer, BUF_ALLOC_SIZE, 
                            OBJ_TILE|PAG_READ|PAG_WRITE|PAG_COMMIT);
  if (rc) 
  {  fprintf(stderr, "scsi_open_device: can't allocate memory\n");
     return (-1);
  }
  /* Open driver */
  rc = DosOpen((PSZ) "ASGSCSI$",
               &driver_handle,
               &ActionTaken,
               0,
               0,
               FILE_OPEN,
               OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
               NULL);
  if (rc) 
  { fprintf(stderr, "scsi_open_device:  opening failed.\n");
    return (-1);
  }
  /* Create event semaphore */
  rc = DosCreateEventSem(NULL, &postSema, DC_SEM_SHARED, 0);
  if (rc) 
  { fprintf(stderr, "scsi_open_device:  couldn't create semaphore.\n");
    return (-1);
  }
  /* Pass semaphore handle to driver */
  rc = DosDevIOCtl(driver_handle, 0x92, 0x03,     
                   (void*) &postSema, sizeof(HEV),&cbParam,
                   (void*) &openSemaReturn, sizeof(USHORT), &cbreturn);
  if (rc || openSemaReturn) 
  {  fprintf(stderr, "scsi_open_device:  couldn't set semaphore.\n");
     return (-1);
  }
 /* Lock buffer. Pass buffer pointer to driver */
  rc = DosDevIOCtl(driver_handle, 0x92, 0x04, 
                   (void*) buffer, sizeof(PVOID),&cbParam, 
                   (void*) &lockSegmentReturn, sizeof(USHORT), &cbreturn);
  if (rc || lockSegmentReturn) 
  { fprintf(stderr, "scsi_open_device:  Can't lock buffer.\n");
    return (-1);
  }

/*
       DosDevIOCtl(driver_handle, 0x92, 0x08, NULL, 
                       0, &cbParam,
                       (void*)&rc, sizeof(void*), &cbreturn);
fprintf(stderr,"XXXXXX %i\n",rc);
// 89  * 0xffff !!!!!
// 193 * 0x8000
// 399 * 0x4000
*/
  return (0);
}

/*-----------------------------------*/
/* void scsi_close_device()          */
/* Close driver and free everything. */
/*-----------------------------------*/
void scsi_close_device()
 { int rc;
   unsigned long cbreturn,cbParam;
   if(postSema)      DosCloseEventSem(postSema);  postSema=0;
   if(driver_handle) DosClose(driver_handle);     driver_handle=0;
   if(buffer)        DosFreeMem(buffer);          buffer=0;
}


/*------------------------------------------------------------------------*/
/* int scsi_handle_cmd ()                                                 */
/* Assembles the command packet to be sent to the SCSI device from the    */
/* specified SCSI command and data, writes it, and reads the reply. If a  */
/* reply size and buffer is specified, the reply data is copied into that */
/* buffer.                                                                */
/* The sense data is copied to scsi_sensebuffer.                          */
/* Return: 0 if successful, -1 if errors occured.                         */
/*------------------------------------------------------------------------*/
int scsi_handle_cmd (unsigned char *scsi_cmd,   int scsi_cmd_len,
                     unsigned char *scsi_data,  int data_size,
                     unsigned char *scsi_reply, int reply_size) 
{ ULONG rc;                  // return value
  unsigned long cbreturn=0;
  unsigned long cbParam=0;
  SRB SRBlock;               // SCSI Request Block
  ULONG count=0;	     // For semaphore.
#ifdef USE_SG
  int sg_list_count=0;       // For scatter-gather
#endif
  if(DEBUG_CMD)
  { int i; 
    for(i=0;i<scsi_cmd_len;i++)fprintf(stderr,"%02x ",scsi_cmd[i]);
    for(i=0;i<data_size;i++)fprintf(stderr,"%02x ",scsi_data[i]);
    fprintf(stderr,"\n");
  }

  memset((char *) &SRBlock, 0, sizeof(SRBlock));

  if (scsi_cmd == NULL) 
    SCSI_ERR("scsi_handle_cmd: No command.\n");
  if ((scsi_cmd_len != 6) && (scsi_cmd_len != 10) && (scsi_cmd_len != 12))
    SCSI_ERR("scsi_handle_cmd: Illegal command length (%d).\n", scsi_cmd_len);
  if (data_size && (scsi_data == NULL)) 
    SCSI_ERR("scsi_handle_cmd: Input data expected.\n");
  if (reply_size && (scsi_reply == NULL)) 
    SCSI_ERR("scsi_handle_cmd: No space allocated for expected reply.\n");
  if (data_size&&reply_size)
    SCSI_ERR("scsi_handle_cmd: Can't do both input and output.\n");
  if (data_size>scsi_maxdatasize())
    SCSI_ERR("scsi_handle_cmd: Packet length exceeds buffer size\n   "
             "             (buffer size %ld bytes, packet length %d bytes).\n",
                                 scsi_maxdatasize(), data_size);
  if(reply_size>scsi_maxdatasize())
    SCSI_ERR("scsi_handle_cmd: Reply length %d exceeds buffer size %ld\n",
			         reply_size, scsi_maxdatasize());

#ifdef USE_SG
  if(data_size>SCSI_BUFSIZE||reply_size>SCSI_BUFSIZE)
  { int    i,n=data_size>reply_size?data_size:reply_size,lsz,npherr;
    struct sg_list *sgl=(struct sg_list *)buffer;
    for(i=0;i<scsi_sg_actnum;i++) 
               { int delta=n>SCSI_SG_BUFSIZE?SCSI_SG_BUFSIZE:n;
                 sgl[i].p=SG_ADDR(buffer,i);
                 sgl[i].sze=delta;
                 n-=delta;
                 if(!n)break;
               } 
    sg_list_count=i+1;
    if(DEBUG_SG)
      { fprintf(stderr,"sg_list_count:%i\n",sg_list_count);
        for(i=0;i<sg_list_count;i++)
            fprintf(stderr,"(%08x,%04x) ",sgl[i].p,sgl[i].sze);
        fprintf(stderr,"\nPhysAddr:\n");
      }
    for(i=0,npherr=0;i<sg_list_count;i++)
     { void *S=sgl[i].p;
       void *D;
       rc = DosDevIOCtl(driver_handle, 0x92, 0x06, (void*)S, 
                       sizeof(void*), &cbParam,
                       (void*)&D, sizeof(void*), &cbreturn);
       sgl[i].p=(unsigned char *)D;
       if(!sgl[i].p){ sgl[i].sze=0; npherr++; }
     } 
    if(DEBUG_SG)
     { for(i=0;i<sg_list_count;i++)
         fprintf(stderr,"(%08x,%04x) ",sgl[i].p,sgl[i].sze);
       fprintf(stderr,"\n");
       if(npherr)fprintf(stderr,"failed %i\n",npherr);
     } 
    if(npherr)
      { fprintf(stderr,"scatter/gather lock failure (%i from %i).\n",
                  npherr,sg_list_count);
        rc = DosDevIOCtl(driver_handle, 0x92, 0x07, NULL, 
                         0, &cbParam, NULL, 0, &cbreturn);
        scsi_sg_actnum=sg_list_count-npherr;
        if(scsi_sg_actnum<0)scsi_sg_actnum=0;
        fprintf(stderr,"reduction scatter/gather buffers to %.1f MB\n",
                        scsi_maxdatasize()/1024./1024.);
        return -1;
      } 
  }  
  
#endif	                                     
  SCSI_MSG(5, "scsi_handle_cmd: Sending command, opcode 0x%02x.\n",
           scsi_cmd[0]);
  SCSI_MSG(5, "scsi_handle_cmd: Pack length = %d, expected reply size = %d\n",
           data_size, reply_size);
  SRBlock.cmd=SRB_Command;                      // execute SCSI cmd
  SRBlock.ha_num=scsi_ha_num;                   // host adapter number
  SRBlock.flags=SRB_Post;                       // posting enabled
#ifdef USE_SG
  if(sg_list_count)
   { SRBlock.flags|=SRB_SG;                     // scatter/gather enabled
     SRBlock.sg_list_len=sg_list_count;
   } 
#endif
#define BUFFER (sg_list_count?SG_ADDR(buffer,0):buffer)

  if (data_size)
	{					// Writing?  Copy data in.
	SRBlock.flags |= SRB_Write;
	memcpy(BUFFER, scsi_data, data_size);
	}
  else if (reply_size)
	SRBlock.flags |= SRB_Read;
  else
	SRBlock.flags |= SRB_NoTransfer;
  SRBlock.u.cmd.target=scsi_id;                 // Target SCSI ID
  SRBlock.u.cmd.lun=0;                          // Target SCSI LUN
					        // # of bytes transferred
  SRBlock.u.cmd.data_len=data_size?data_size:
                                   reply_size;
  SRBlock.u.cmd.sense_len=16;                   // length of sense buffer
  SRBlock.u.cmd.data_ptr=NULL;                  // pointer to data buffer
  SRBlock.u.cmd.link_ptr=NULL;                  // pointer to next SRB
  SRBlock.u.cmd.cdb_len=scsi_cmd_len;           // SCSI command length
  memcpy(&SRBlock.u.cmd.cdb_st[0], scsi_cmd, scsi_cmd_len);
						// Do the command.
#if(DEBUG)
  fprintf(stderr,"do command...\n");
#endif
  rc = DosDevIOCtl(driver_handle, 0x92, 0x02, (void*) &SRBlock, 
                   sizeof(SRB), &cbParam,
                   (void*)&SRBlock, sizeof(SRB), &cbreturn);

  if (rc) 
  {
    perror("scsi_handle_cmd");
    return(-1);
  }
#if(DEBUG)
  fprintf(stderr,"wait for semaphore...\n");
#endif
  while((rc=DosWaitEventSem(postSema,SEM_IMMEDIATE_RETURN))
             ==ERROR_TIMEOUT)sleep(1);
  if(rc||DosResetEventSem(postSema, &count))
       SCSI_ERR("scsi_handle_cmd:  semaphore failure.\n");
#if(DEBUG)
  fprintf(stderr,"done\n");
#endif
				            // Clear sense buffer.
  memset(scsi_sensebuffer, 0, sizeof(scsi_sensebuffer));
					    // Get sense data if available.
  if ((SRBlock.status == SRB_Aborted || SRBlock.status == SRB_Error) &&
      SRBlock.u.cmd.target_status == SRB_CheckStatus)
        memcpy(scsi_sensebuffer,&SRBlock.u.cmd.cdb_st[scsi_cmd_len], 
  						sizeof(scsi_sensebuffer));
  if (SRBlock.status != SRB_Done ||
      SRBlock.u.cmd.ha_status != SRB_NoError ||
      SRBlock.u.cmd.target_status != SRB_NoStatus)
         SCSI_ERR("scsi_handle_cmd:  command 0x%02x failed.\n",scsi_cmd[0]);
#ifdef USE_SG
  if(reply_size)
   { if(sg_list_count)
       { int i,n=reply_size;
         unsigned char *d=scsi_reply;
         for(i=0;i<sg_list_count;i++)
           { int delta=n>SCSI_SG_BUFSIZE?SCSI_SG_BUFSIZE:n;
             unsigned char *s=SG_ADDR(buffer,i);
             memcpy(d,s,delta);
             n-=delta;
             d+=delta;  
             if(!n)break;
           }
       }
     else memcpy(scsi_reply, BUFFER, reply_size);
   }
  if(sg_list_count)
   { rc = DosDevIOCtl(driver_handle, 0x92, 0x07, NULL, 
                     0, &cbParam, NULL, 0, &cbreturn);
   } 
#else
  if(reply_size)memcpy(scsi_reply, BUFFER, reply_size);
#endif

 return(0);
}


int  scsi_maxdatasize()
/*-------------------*/
{
#ifndef USE_SG
  return SCSI_BUFSIZE;
#else
  return SCSI_SG_BUFSIZE*scsi_sg_actnum;
#endif
}

extern void scsi_sg_bufsize(int bytes)
/*----------------------------------*/
{ int n=bytes/SCSI_SG_BUFSIZE;
  if(n<0)n=0;
  scsi_sg_actnum=n;
}

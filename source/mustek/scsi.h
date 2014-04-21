#ifndef _SCSI_H
#define _SCSI_H

#define USE_SG

#define SCSI_HDRLEN sizeof(struct sg_header)

#define SCSI_BUFSIZE       (0x8000L)                          // 32kB
#define SCSI_SG_BUFSIZE    (0xffffL)                          // 64kB (-1B)
#define SCSI_SG_DEFAULTNUM (0x50L)                            // 80  
//#define SCSI_SG_SUMSIZE ( SCSI_SG_BUFSIZE*SCSI_SG_MAXNUM)   
// 64 4MB  A4 l 580 dpi A4 g 200 dpi A4 c 120 dpi
// 80 5MB                    225 dpi      130 dpi
                            

typedef unsigned char scsi_6byte_cmd[6];
typedef unsigned char scsi_10byte_cmd[10];
typedef unsigned char scsi_12byte_cmd[12];

extern int scsi_debug_level;
extern char *scsi_device;
extern int scsi_fd;
extern unsigned char scsi_cmd_buffer[];
extern unsigned int scsi_packet_id;
extern int scsi_memsize;
extern unsigned char scsi_sensebuffer[];

struct inquiry_data {
  char peripheral_qualifier;
  char peripheral_device_type;
  char RMB;
  char device_type_modifier;
  char ISO_version;
  char ECMA_version;
  char ANSI_version;
  char AENC;
  char TrmIOP;
  char response_data_format;
  char RelAdr;
  char WBus32;
  char WBus16;
  char Sync;
  char Linked;
  char CmdQue;
  char SftRe;
  char vendor[9];
  char model[17];
  char revision[5];
  char additional_inquiry;
};

extern int  scsi_open_device  ( char *dev );  /* ad:id */
extern void scsi_close_device (void);
extern int  scsi_handle_cmd   (unsigned char *scsi_cmd,   int scsi_cmd_len,
                               unsigned char *scsi_data,  int data_size,
                               unsigned char *scsi_reply, int reply_size);
extern int  scsi_maxdatasize  ();
extern void scsi_sg_bufsize   (int bytes);

#endif

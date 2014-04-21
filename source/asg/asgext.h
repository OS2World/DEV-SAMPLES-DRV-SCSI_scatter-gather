#define N_SGLH 1024

USHORT FAR  DriverOpen   ( PRP_OPENCLOSE  pRPH );
USHORT FAR  DriverClose  ( PRP_OPENCLOSE  pRPH );
USHORT FAR  DriverInit   ( PRPINITIN     pRPH );
USHORT FAR  DriverIOCtl  ( PRP_GENIOCTL  pRPH );
VOID   NEAR GetInitParms ( PUCHAR pCmdString, PRPINITIN pRP);
VOID   NEAR sprintf      ( PSZ s, PSZ fmt, ... );
VOID   NEAR puts         ( PSZ Buf );
VOID   FAR  ctx_hand     (void);

/*-------------------------------------------------------------------*/
/*      Static Data                                                  */
/*-------------------------------------------------------------------*/

extern PFN          Device_Help;

extern ULONG        postSema;   // (32 bit HEV)
extern ULONG        physBuffer; 
extern BYTE         postInProgress;

extern VOID   (FAR *ProtIDCEntry)(VOID);
extern USHORT       ProtIDC_DS;

extern ULONG        LockHandle;
extern ULONG        SgLockHandle[N_SGLH];
extern ULONG        SgLockCounter;
extern ULONG        Hook1;

/*-------------------------------------------------------------------*/
/*                                                                   */
/*-------------------------------------------------------------------*/


/*-------------------------------------------------------------------*/
/*      Initialization Data                                          */
/*-------------------------------------------------------------------*/

extern UCHAR        BeginInitData;
extern MSGTABLE     InitMsg;
extern UCHAR        VersionMsg[];
extern NPSZ         DDName;
extern IDCTABLE     IDCTable;
extern UCHAR        IdentMsg[];
extern UCHAR        DD_OK_Msg[];
extern UCHAR        DD_ERR_Msg[];

extern UCHAR        StringBuffer[];

#define IOCTLCAT  0x92

SCSI scatter/gather sample code 
-------------------------------

- Only Warp Server for e-Business tested ( PIII/450,256 MB, 
  ADAPTEC AHA-2910C, MUSTEK scanner MFS-12000SP firmware revision 1.02.),
- SCSI scatter/gather buffer 5MB ( only one ioctl command for A4 color 
  130 dpi or gray 220 dpi )      

bin\ASG.SYS  - simple ASPI router with scatter/gather support 
             - for testing add the following line to the CONFIG.SYS:
               DEVICE=YOUR_PATH\asg.sys
bin\scan.exe - sample program for MUSTEK scanner MFS-12000SP
               (firmware revision 1.02.)      
             - emx runtime requiered 
             - for help type scan -h and read source code

source\asg    ... source code for "ASG.SYS"
source\mustek ... sample "scan.exe" program source code
                  MUSTEK scanner MFS-12000SP  (firmware revision 1.02.)


used code from:
1) IBM OS/2 device driver [development] kit (DDK)
2) ASPI  Router  for  OS/2, Daniel Dorau (woodst@cs.tu-berlin.de)
   Version 1.01  June 1997
3) Mustek backend from sane - Scanner Access Now Easy.
   Copyright (C) 1996, 1997 David Mosberger-Tang and Andreas Czechanowski,
   1998 Andreas Bolsch for extension to ScanExpress models version 0.6,
   2000 Henning Meier-Geinitz


Wed Dec 13 20:10:08 CET 2000

Bohumir Horeni
horeni@login.cz 
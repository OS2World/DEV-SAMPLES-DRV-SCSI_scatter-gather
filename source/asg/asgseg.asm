                .386p
                include devhdr.inc

;/*-------------------------------------*/
;/* Assembler Helper to order segments  */
;/*-------------------------------------*/

DDHeader       segment word public use16 'DATA'

               public _AsgDDHeader

_AsgDDHeader   dd      -1
               dw      DEVLEV_3 + DEV_CHAR_DEV + DEV_SHARE + DEV_30
               dw      _AsgStr
               dw      0
               db      "ASGSCSI$"
               dq      0
               dd      DEV_IOCTL2 + DEV_INITCOMPLETE + DEV_16MB
               dw      0
DDHeader       ends

LIBDATA        segment dword public use16 'DATA'
LIBDATA        ends

_DATA          segment dword public use16 'DATA'
_DATA          ends

CONST          segment dword public use16 'CONST'
CONST          ends

_BSS           segment dword public use16 'BSS'
_BSS           ends

_TEXT          segment dword public use16 'CODE'

               extrn  _AsgStr : near

_TEXT          ends

Code            segment dword public use16 'CODE'
Code            ends

LIBCODE         segment dword public use16 'CODE'
LIBCODE         ends

DGROUP          group   CONST, _BSS, DDHeader, LIBDATA, _DATA
StaticGroup     group   Code, LIBCODE, _TEXT

        end

;*****************************************************************************
;*                                                                           *
;*  Assembler module for ASPI Router                                         *
;*                                                                           *
;*  This is the entry code for the post callback routine called              *
;*  from OS2ASPI.DMD.                                                        *
;*                                                                           *
;*****************************************************************************

        .386p

        extrn   _asgPost : near
        extrn   _postInProgress : near
        public  _postEntry
        PUBLIC  _cstart

_TEXT   segment word public use16 'CODE'

        assume        cs:_TEXT

_postEntry proc far
_cstart:
        mov     byte ptr _postInProgress, 1
        push    bp
        mov     bp, sp
        pusha
        mov     ds, [bp+6]
        push    dword ptr [bp+8]
        call    _asgPost
        add     sp, 4
        popa
        pop     bp
        mov     byte ptr _postInProgress, 1
        ret
_postEntry endp

_TEXT   ends

        end

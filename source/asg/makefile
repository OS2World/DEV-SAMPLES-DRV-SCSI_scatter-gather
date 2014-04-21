
#****************************************************************************
#  Dot directive definition area (usually just suffixes)
#****************************************************************************

.SUFFIXES:
.SUFFIXES: .com .flt .exe .obj .asm .inc .def .lnk .lrf .crf .ref
.SUFFIXES: .lst .sym .map .c .h .lib .cod .flt

#****************************************************************************
#  Environment Setup for the component(s).
#****************************************************************************

#
# Conditional Setup Area and User Defined Macros
#

#
# Compiler Location w/ includes, libs and tools
#

DDKBASE   = c:\usr\os2\ddk\base

INC       = $(DDKBASE)\inc
H         = $(DDKBASE)\h
LIB       = $(DDKBASE)\lib
TOOLSPATH = $(DDKBASE)\tools


#DOSINC    = ..\..\..\..\dos\dosinc
#INIT      = ..\..\..\..\dos\init
#TASK      = ..\..\..\..\dos\task
#INC       = ..\..\..\..\..\inc
#H         = ..\..\..\..\..\h
#LIB       = ..\..\..\..\..\lib
#TOOLSPATH = ..\..\..\..\..\tools
#DISKH     = ..\..\diskh
#DHLIB     = ..\..\devhelp
#ADDLIB    = ..\..\addcalls
#APMLIB    = ..\..\..\thinkpad\dockii\apmcalls
#DOCLIB    = ..\..\..\thinkpad\dockii\doccalls
#CDROM     = ..\os2cdrom
#RMLIB     = ..\..\..\resource\rmcalls
#RSM_H     = ..\..\..\resource\rsm_h

#
# Since the compiler/linker and other tools use environment
# variables ( INCLUDE, LIB, etc ) in order to get the location of files,
# the following line will check the environment for the LIFE of the
# makefile and will be specific to this set of instructions. All MAKEFILES
# are requested to use this format to insure that they are using the correct
# level of files and tools.
#

!if [set INCLUDE=.;$(H);$(INC)] || \
    [set LIB=$(LIB)] || [set PATH=$(TOOLSPATH)]
!endif


#
# Compiler/tools Macros
#

AS=masm
CC=cl
IMPLIB=implib
IPF=ipfc
LIBUTIL=lib
LINK=link
MAPSYM=mapsym
RC=rc

#
# Compiler and Linker Options
#

AFLAGS = -MX -T -Z $(ENV)
AINC   = -I. -I$(INC)
CFLAGS = /c /Zp /G2s /Answ /nologo $(ENV)
DFLAGS = /c /Fc /Zp /G2s /Answ /nologo $(ENV) -DDEBUG -DPRE_RELEASE
LFLAGS = /map /nod /noe /exepack /packd /a:16 /far
RFLAGS = -r

LIBS  =  $(LIB)\dhcalls.lib $(LIB)\libh.lib $(LIB)\os2386p.lib 
DEF02 =  asg.def

#****************************************************************************
# Set up Macros that will contain all the different dependencies for the
# executables and dlls etc. that are generated.
#****************************************************************************

#
# OBJ files
#
#
OBJ1 =  ASGSEG.OBJ ASGGEN.OBJ ASGDAT.OBJ
OBJ2 =  ASGIOCTL.OBJ ASGCLS.OBJ 
OBJ3 =  ASGPOST.OBJ
OBJ4 =  ASGINI.OBJ PRINTF.OBJ GETINIP.OBJ

#
#       LIST Files
#
LIST=   ASGSEG.LST ASGGEN.LST ASGDAT.LST \
        ASGINI.LST ASGCLS.LST ASGIOCTL.LST 

OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)

#****************************************************************************
#   Setup the inference rules for compiling and assembling source code to
#   obejct code.
#****************************************************************************


.asm.obj:
        $(AS) $(AFLAGS) $(AINC) $*.asm;

.asm.lst:
        $(AS) -l -n $(AFLAGS) $(AINC) $*.asm;

.c.obj:
        $(CC) $(CFLAGS) $(CINC) $*.c

.c.lst:
        $(CC) $(CFLAGS) /Fc $(CINC) $*.c
        copy $*.cod $*.lst
        del $*.cod


#****************************************************************************
#   Target Information
#****************************************************************************
#
# This is a very important step. The following small amount of code MUST
# NOT be removed from the program. The following directive will do
# dependency checking every time this component is built UNLESS the
# following is performed:
#                    A specific tag is used -- ie. all
#
# This allows the developer as well as the B & I group to perform incremental
# build with a degree of accuracy that has not been used before.
# There are some instances where certain types of INCLUDE files must be
# created first. This type of format will allow the developer to require
# that file to be created first. In order to achive that, all that has to
# be done is to make the DEPEND.MAK tag have your required target. Below is
# an example:
#
#    depend.mak:   { your file(s) } dephold
#
# Please DON'T remove the following line
#

!include      "$(H)\version.mak"

#
# Should be the default tag for all general processing
#

all:   ASG.SYS

list: $(LIST)

clean:
        if exist *.lnk  del *.lnk
        if exist *.obj  del *.obj
        if exist *.map  del *.map
        if exist *.old  del *.old
        if exist *.lst  del *.lst
        if exist *.lsd  del *.lsd
        if exist *.sym  del *.sym
        if exist *.sys  del *.sys
        if exist *.add  del *.add
        if exist *.flt  del *.flt
        if exist *.tff  del *.tff
        if exist *.def  del *.def
        if exist *.mak  del *.mak


#*****************************************************************************
#   Specific Description Block Information
#*****************************************************************************

# This section would only be for specific direction as to how to create
# unique elements that are necessary to the build process. This could
# be compiling or assembling, creation of DEF files and other unique
# files.
# If all compiler and assembly rules are the same, use an inference rule to
# perform the compilation.
#

ASG.SYS:  $(OBJS) $(LIBS) makefile
        rem Create DEF file <<$(DEF02)
LIBRARY ASG

DESCRIPTION "$(FILEVER)  Aspi Router"

PROTMODE

SEGMENTS
 DDHeader       CLASS 'DATA'
 LIBDATA        CLASS 'DATA'
 _DATA          CLASS 'DATA'
 CONST          CLASS 'CONST'
 _BSS           CLASS 'BSS'
 'Code'         CLASS 'CODE' IOPL
 LIBCODE        CLASS 'CODE' IOPL
 _TEXT          CLASS 'CODE' IOPL

<<keep
        $(LINK) $(LFLAGS) @<<$(@B).lnk
$(OBJ1) +
$(OBJ2) +
$(OBJ3) +
$(OBJ4)
$*.SYS
$*.map
$(LIBS)
$(DEF02)
<<keep
             $(MAPSYM) $*.map

#****************************************************************************

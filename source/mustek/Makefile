#
# Makefile scan
#

# Defined (compiler)

DEFINED = 

# Include Directories

INCLUDES = -I./ -I../include 

# RC Include Directories

RCINCLUDES = -i ./ -i ../include 

# Standard Library Directories

STDL = 

# Library Object Directory

LODIR =  #

# List Of Library Object Files

LOBJ = \


# Object Directory

ODIR = ./#

# List Of Object Files

OBJ = \
$(ODIR)eps.o $(ODIR)scan.o $(ODIR)scsi.o 

# Linked Libraries

LIBR = 

# Library Files

DLIB = 

# Target Def File

TDEF = 

# Target Rc File

TRC = 

# Target Res File

TRES = 

# Target Exe File

TEXE = scan.exe

# Tools

AR  = ar
AF  = rvs
CR  = rc
CRF = -r $(RCINCLUDES)
CC  = gcc
CF  = -c $(CFS) $(INCLUDES) $(DEFINED) -O3 
CXF = -s $(STDL)

# Target

all : $(TEXE)
.PHONY : all

# Exe Rule

$(TEXE) : $(OBJ) $(DLIB) $(TDEF) $(TRES)
	$(CC) $(CXF) -o $@ $(WX0) $(OBJ) $(LIBR) $(TDEF) $(TRES)

# Object Rules

$(ODIR)eps.o : ./eps.c \
               .//eps.h
	$(CC) $(CF) -o $@ $<

$(ODIR)scan.o : ./scan.c \
                .//scsi.h \
                .//eps.h
	$(CC) $(CF) -o $@ $<

$(ODIR)scsi.o : ./scsi.c \
                .//srb.h \
                .//scsi.h
	$(CC) $(CF) -o $@ $<

clean: 
	rm -f $(OBJ) $(LOBJ) $(TEXE)

# end
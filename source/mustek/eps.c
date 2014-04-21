#include<string.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>

#include "eps.h"

void write_eps_header(int w, int h, int dpi, char *modename)
/*========================================================*/
{ int llx=0,lly=0,urx,ury;
  time_t t=time(0);

  urx = llx+(w*72+dpi/2)/dpi;
  ury = lly+(h*72+dpi/2)/dpi;

  fprintf(stdout,
         "%%!PS-Adobe-3.0 EPSF-3.0\n"
         "%%%%BoundingBox: %i %i %i %i\n"
         "%%%%Title: %s\n" 
         "%%%%Creator: %s (%ix%i,%idpi,%s)\n"
         "%%%%CreationDate: %s"  // '\n' in ctime()
         "%%%%Pages: 1\n"
         "%%%%EndComments\n\n",
         llx,lly,urx,ury,
         "none",CREATOR,w,h,dpi,modename,ctime(&t));
  fprintf(stdout,
    "%%%%BeginProlog\n"
    "/B {bind def} bind def\n"
    "/cp {closepath} B /l {lineto} B /m {moveto} B\n"
    "/n {newpath} B /r {setrgbcolor} B /g {setgray} B\n"
    "/s {stroke} B /f {fill} B /t {moveto show} B\n"
    "/rt {gsave translate 1000 div rotate 0 0 moveto show grestore} B\n"
    "/SF {findfont exch scalefont setfont} B\n"
    "/w {setlinewidth} B /c {setlinecap} B /j {setlinejoin} B\n"
           );
  fprintf(stdout,
    "/redefine { findfont begin currentdict dup length dict begin {\n"
    "1 index /FID ne {def} {pop pop} ifelse } forall /FontName exch def\n"
    "dup length 0 ne { /Encoding Encoding 256 array copy def 0 exch {\n"
    "dup type /nametype eq { Encoding 2 index 2 index put pop 1 add\n"
    "}{ exch pop } ifelse } forall } if pop currentdict dup end end\n"
    "/FontName get exch definefont pop } B\n"
          );
  printf("%%%%EndProlog\n\n"
         "%%%%Page: 1 1\n");
  printf("72 %i div dup scale\n",dpi);
}


void write_eps_trailer()
/*====================*/
{ fprintf(stdout,"showpage\n"); 
  fprintf(stdout,"%%%%EOF\n"); 
}

#define TOH(x) ((x)<10?'0'+(x):'a'+(x)-10)

static
void write_eps_pixmap(int x, int y, int w, int h, char *dta, int nbp)
/*=================================================================*/
/* y +---+-----+  */
/*   |   | map |  */
/*   |   +-----+  */
/*   |   |        */
/* 0 +---+ x      */
/*                */
{ int i,k;
  unsigned char *buf=(unsigned char *)malloc(4*w*nbp),
                *udt=(unsigned char*)dta;
  for(i=0;i<h;i++,y--) 
   { int pixw=w,pixh=1;
     int n=w*pixh*nbp;
     unsigned char *p=buf;  

     for(k=0;k<n;k++) { unsigned c=*udt++;
                        if(k&&k%30==0)*p++='\n';
                        *p++=TOH(c>>4);
                        *p++=TOH(c&0xf); 
                      }  
     *p=0; 
     fprintf(stdout,"gsave %i %i %i %i translate scale "
                    "%i %i 8 [%i 0 0 -%i 0 %i] {<\n"
                    "%s\n>} %s grestore\n",
                pixw,pixh,x,y>0?y:0,
                pixw,pixh,pixw,pixh,pixh,
                buf,nbp>1?"false 3 colorimage":"image");
  }
  free(buf);
}

void write_eps_colormap(int x, int y, int bpr, int h, char *dta)
/*------------------------------------------------------------*/
{ write_eps_pixmap(x,y,bpr/3,h,dta,3);
}

void write_eps_graymap(int x, int y, int bpr, int h, char *dta)
/*-----------------------------------------------------------*/
{ write_eps_pixmap(x,y,bpr,h,dta,1);
}


void write_eps_bitmap(int x, int y, int bpr, int h, char *dta)
/*===========================================================*/
{ int i,k,w=8*bpr;
  unsigned char *buf=(unsigned char *)malloc(4*bpr),
                *udt=(unsigned char*)dta;
  for(i=0;i<h;i++,y--) 
   { int bitsw=w,bitsh=1;
     int n=bpr*bitsh;
     unsigned char *p=buf;  

     for(k=0;k<n;k++) { unsigned c=*udt++;
                        if(k&&k%30==0)*p++='\n';
                        *p++=TOH(c>>4);
                        *p++=TOH(c&0xf); 
                      }  
     *p=0; 
     fprintf(stdout,"gsave %i %i %i %i translate scale "
                    "%i %i true [%i 0 0 -%i 0 %i] {<\n"
                    "%s\n>} imagemask grestore\n",
                bitsw,bitsh,x,y>0?y:0,
                bitsw,bitsh,bitsw,bitsh,bitsh,
                buf);
  }
  free(buf);
}



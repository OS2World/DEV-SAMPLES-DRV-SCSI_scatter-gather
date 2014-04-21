/*=====================================================*/
/* MUSTEK scanner MFS-12000SP  firmware revision 1.02. */
/*=====================================================*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "scsi.h"
#include "eps.h"


int  inquiry     (void);
int  testready   (void); 
int  backtrack   (int on_off);
int  startscan   (int mode);
int  modeselect  (int mode, int dpi, int brightness, int contrast, int speed);
int  scanarea    (double tlx, double tly, double brx, double bry, int max_dpi);
int  getstatus   (int *bytes_per_line, int *lines);
int  readdata    (int res, int mode, int lines, int bpl, int peak_r, int d[3]);
int  ccddistance (int res, int mode, int max_dpi, int *peak_res, int d[3]);

void help        (void);

#define FASTEST 0
#define FASTER  1
#define NORMAL  2
#define SLOWER  3
#define SLOWEST 4

#define RGB  6
#define GRAY 5
#define LART 4

#define MM_PER_INCH     25.4

#define MAX_DPI         1200
#define MAX_W          ( 8.5 * MM_PER_INCH)  
#define MAX_H          (14.0 * MM_PER_INCH)

#define DEFAULT_DPI            50
#define DEFAULT_DEVICE       "0:6"
#define DEFAULT_X             0.0
#define DEFAULT_Y             0.0
#define DEFAULT_W           210.0
#define DEFAULT_H           297.0
#define DEFAULT_GAMMA         1.0
#define DEFAULT_BLACK_LIMIT   0.0  // relative  <limit ... black
#define DEFAULT_WHITE_LIMIT   1.0  // relative  >limit ... white
#define FMT_PNM         1
#define FMT_PS          2

#define END { scsi_close_device(); return -1; }

static int debug=0;
static int verbose=1;
static int format=FMT_PNM;
static double gamma=DEFAULT_GAMMA;
static double black_limit=DEFAULT_BLACK_LIMIT;
static double white_limit=DEFAULT_WHITE_LIMIT;

int main(int argc, char *argv[])
/*=============================*/
{ char   *device=DEFAULT_DEVICE;
  int     bytes_per_line,lines,opt,peak_res,dist[3],
          dpi_max    = MAX_DPI,
          mode       = GRAY,
          res        = DEFAULT_DPI,
          brightness = 0,  /* -100 ... +100    */
          contrast   = 0,  /* -100 ... +100    */
          back       = 1,  /* backtrack on_off */
          speed      = NORMAL;
   double x = DEFAULT_X,
          y = DEFAULT_Y,
          w = DEFAULT_W,
          h = DEFAULT_H;

   optind=0;
   while((opt=getopt(argc,argv,"cd:ghlr:x:y:W:H:DqG:pB:t:T:"))>=0)
   switch(opt)
    { case 'c': mode=RGB;   break;
      case 'l': mode=LART;  break;
      case 'g': mode=GRAY;  break;
      case 'r': { int r;
                  if(sscanf(optarg,"%i",&r)==1)res=r;
                } break;  
      case 'x': { int r;
                  if(sscanf(optarg,"%i",&r)==1)x=r;
                } break;  
      case 'y': { int r;
                  if(sscanf(optarg,"%i",&r)==1)y=r;
                } break;  
      case 'W': { int r;
                  if(sscanf(optarg,"%i",&r)==1)w=r;
                } break;  
      case 'H': { int r;
                  if(sscanf(optarg,"%i",&r)==1)h=r;
                } break;  
      case 'G': { double r;
                  if(sscanf(optarg,"%lf",&r)==1&&r>0)gamma=r;
                } break;  
      case 't': { double r;
                  if(sscanf(optarg,"%lf",&r)==1&&r>=0&&r<=1)black_limit=r;
                } break;  
      case 'T': { double r;
                  if(sscanf(optarg,"%lf",&r)==1&&r>=0&&r<=1)white_limit=r;
                } break;  
      case 'p': format=FMT_PS;         break; 
      case 'd': device=strdup(optarg); break;
      case 'B': { double r;
                  if(sscanf(optarg,"%lf",&r)==1)
                        scsi_sg_bufsize((int)(r*1024*1024));
                } break; 
      case 'D': debug++;               break;
      case 'q': verbose=0;             break;
      case 'h': help(); return 0;
      default:;
    } 

  if(x<0)x=0; if(x>MAX_W)x=MAX_W;
  if(y<0)y=0; if(y>MAX_H)y=MAX_H;
  if(x+w>MAX_W)w=MAX_W-x;
  if(y+h>MAX_H)h=MAX_H-y;

  if(scsi_open_device(device))
    { fprintf(stderr,"Unable to open SCSI device.\n"); return -1; }
  if(debug){ fprintf(stderr,"scsi_open ok.\n");  }

  if(inquiry())                                          END
  if(testready())                                        END
  if(scanarea(x,y,x+w,y+h,dpi_max))                      END
  if(backtrack(back))                                    END
  if(modeselect(mode,res,brightness,contrast,speed))     END
  if(startscan(mode))                                    END
  if(ccddistance(res,mode,dpi_max,&peak_res,dist))       END
  if(backtrack(back))                                    END
  if(getstatus(&bytes_per_line, &lines))                 END

  readdata(res,mode,lines,bytes_per_line,peak_res,dist);

  if(debug)fprintf(stderr,"scsi_close_device...\n");

  scsi_close_device();
  if(debug)fprintf(stderr,"end\n");

  return(0);
}

void help()
/*=======*/
{ printf("scan [opt] [ > file ]\n"
         "options: l  ... lineart\n" 
         "         c  ... color\n" 
         "         g  ... gray (default)\n" 
         "         r# ... resolution (dpi, default %i)\n" 
         "         d# ... SCSI device (ad:id, default %s)\n" 
         "         x# ... left offset (mm, default %.0f)\n" 
         "         y# ... top offset (mm, default %.0f)\n" 
         "         W# ... width (mm, default %.0f)\n" 
         "         H# ... height (mm, default %.0f)\n" 
         "         G# ... gamma (default %.2f)\n" 
         "         t# ... relative lower (black) treshold (default %.2f)\n" 
         "         T# ... relative upper (white) treshold (default %.2f)\n" 
         "         p  ... postscript output (default pnm)\n" 
         "         B# ... scatter/gather buffer size (MB, default %.1f)\n"  
         "         D  ... debug mode\n"
         "         q  ... quiet mode\n"
         "         h  ... help\n",
         DEFAULT_DPI,DEFAULT_DEVICE,
         DEFAULT_X,DEFAULT_Y,DEFAULT_W,DEFAULT_H,DEFAULT_GAMMA,
         DEFAULT_BLACK_LIMIT,DEFAULT_WHITE_LIMIT,
         scsi_maxdatasize()/1024./1024.); 
}


#define MUSTEK_SCSI_TEST_UNIT_READY	0x00
#define MUSTEK_SCSI_AREA_AND_WINDOWS	0x04
#define MUSTEK_SCSI_READ_SCANNED_DATA	0x08
#define MUSTEK_SCSI_GET_IMAGE_STATUS	0x0f
#define MUSTEK_SCSI_ADF_AND_BACKTRACK	0x10
#define MUSTEK_SCSI_CCD_DISTANCE	0x11
#define MUSTEK_SCSI_INQUIRY             0x12
#define MUSTEK_SCSI_MODE_SELECT		0x15
#define MUSTEK_SCSI_START_STOP		0x1b

int inquiry(void)
/*==============*/
{ int mustek_scanner,fw_revision;
  char buf[0x60];
  char *model_name=buf+44;

  scsi_6byte_cmd cmd = {MUSTEK_SCSI_INQUIRY, 0x00, 0x00, 
                        0x00, sizeof(buf), 0x00};
  if (scsi_handle_cmd(cmd, 6, NULL, 0, buf, sizeof(buf)) < 0) 
   {  fprintf(stderr, "inquiry: command failed (scanner not found)\n");
      return(-1);
   }
  mustek_scanner=(strncmp (buf+36,"MUSTEK",6)==0);
  if (!mustek_scanner)
    { /* check for old format: */
      mustek_scanner=(strncmp (buf+8,"MUSTEK",6)==0);
      model_name=buf+16;
    }
  mustek_scanner = mustek_scanner && (buf[0] == 0x06);

  if (!mustek_scanner)
    { fprintf(stderr,"device doesn't look like a Mustek scanner "
	  "(buf[0]=%#02x)\n", buf[0]);
      return -1;
    }

  /* get firmware revision as BCD number: */
  fw_revision=(buf[32]-'0')<<8|(buf[34]-'0')<<4|(buf[35]-'0');

  if(debug||verbose)
  fprintf(stderr,"MUSTEK scanner %s firmware revision %d.%02x.\n",
          model_name,fw_revision>>8,fw_revision&0xff);

  return(0);
}

int testready(void) 
/*================*/
{ scsi_6byte_cmd TEST = {MUSTEK_SCSI_TEST_UNIT_READY, 0x00, 0x00,
                            0x00, 0x00, 0x00};
  if(debug)fprintf(stderr,"test_ready... ");
  if (scsi_handle_cmd(TEST, 6, NULL, 0, 0, 0) < 0) 
   {  fprintf(stderr, "test_ready: command failed\n");
      return(-1);
   }
  if(debug)fprintf(stderr,"ok.\n");
  return(0);
}

int backtrack(int on_off)
/*======================*/
{ scsi_6byte_cmd cmd = {MUSTEK_SCSI_ADF_AND_BACKTRACK, 0x00, 0x00,
                        0x00, 0x80, 0x00};
  if(on_off)cmd[4]|=0x02;
  if(debug)fprintf(stderr,"backtrack %s... ",on_off?"on":"off");
  if (scsi_handle_cmd(cmd, 6, NULL, 0, 0, 0) < 0 )
   {  fprintf(stderr, "backtrack: command failed\n");
      return(-1);
   }
  if(debug)fprintf(stderr,"ok.\n");
  return(0);
}


int startscan ( int mode )
/*=======================*/
{ scsi_6byte_cmd cmd = {MUSTEK_SCSI_START_STOP, 0x00, 0x00,
                        0x00, 0x01, 0x00};

  if(mode==RGB) cmd[4]|=0x40|0x20;
  if(mode==GRAY)cmd[4]|=0x40;

  if (scsi_handle_cmd(cmd, 6, NULL, 0, 0, 0) <0 )
   {  fprintf(stderr, "start_scan: command failed\n");
      return(-1);
   }
  return(0);
}

char *store16(char *p, int v)
/*-------------------------*/
{ *p++ = v & 0xff;  *p++ = v >> 8 & 0xff; return p; }

int scanarea(double tlx, double tly, double brx, double bry,
             int dpi_range_max)
/*========================================================*/
{ scsi_6byte_cmd cmd = {MUSTEK_SCSI_AREA_AND_WINDOWS, 0x00, 0x00,
                        0x00, 0x00, 0x00};
  unsigned char data[256],*cp;
  int           datasize;
  memset(data, 0, sizeof (data));
  cp = data; 
  /* fill in frame header: */
    { double pixels_per_mm = dpi_range_max / MM_PER_INCH;

      /* pixel unit ( no halftoning) : */  
      *cp++ = 0x8;
      /* fill in scanning area: */
      cp=store16(cp, (int)floor(tlx * pixels_per_mm + 0.5));
      cp=store16(cp, (int)floor(tly * pixels_per_mm + 0.5));
      cp=store16(cp, (int)floor(brx * pixels_per_mm + 0.5));
      cp=store16(cp, (int)floor(bry * pixels_per_mm + 0.5));
    }

  datasize = cmd[4] = cp-data;
  if(debug)fprintf(stderr,"scanarea... ");
  if (scsi_handle_cmd(cmd, 6, data, datasize, 0, 0) <0 )
   {  fprintf(stderr, "scan_area: command failed\n");
      return(-1);
   }
  if(debug)fprintf(stderr,"ok.\n");
  return(0);
}


static unsigned encode_percentage (double v)
/*-----------------------------------------*/
{ int rc, mx=0xff, sg = v<0?0x80:0x00;
  rc = (int) (fabs(v) /100. * 127 + 0.5);  rc |= sg;
  if (rc > mx) rc = mx;       if (rc < 0)  rc = 0x00;
  return rc;
}


int modeselect ( int mode,
                 int dpi, int brightness, int contrast, int speed_code)
/*--------------------------------------------------------------------*/
{ int grain_code,i;
  unsigned char data[13];
  scsi_6byte_cmd cmd = {MUSTEK_SCSI_MODE_SELECT, 0x00, 0x00,
                        0x00, sizeof(data), 0x00};

  /* the scanners use a funky code for the grain size, let's compute it: */
  grain_code = 2;
  if (grain_code > 7) grain_code = 7;	/* code 0 is 8x8, not 7x7 */
  grain_code = 7 - grain_code;

  /* set mode byte: */
  if(mode==LART)data[0] = 0x8b;
  if(mode==GRAY)data[0] = 0x8b;
  if(mode==RGB) data[0] = 0xab;
  data[1] = 0;
  data[2] = encode_percentage (brightness);
  data[3] = encode_percentage (contrast);
  data[4] = grain_code;
  data[5] = speed_code;  	/* lamp setting not supported yet */
  data[6] = 0;			/* shadow param not used by Mustek */
  data[7] = 0;			/* highlist param not used by Mustek */
  data[8] = data[9] = 0;	/* paperlength not used by Mustek */
  data[10] = 0;			/* midtone param not used by Mustek */
  /* set resolution code: */
  store16(data+11, dpi);

  for(i=0;i<(mode==RGB?3:1);i++,data[0]+=0x20); /* .01. .... */
                                                /* .10. .... */
                                                /* .11. .... */
  if(debug)fprintf(stderr,"modeselect... ");
    if (scsi_handle_cmd(cmd, 6, data, sizeof(data), 0, 0) <0 )
      {  fprintf(stderr, "modeselect: command failed\n");
         return(-1);
      }
  if(debug)fprintf(stderr,"ok.\n");
  return(0);
}

int getstatus (int *bytes_per_line, int *lines)
/*--------------------------------------------*/
{ scsi_6byte_cmd cmd = {MUSTEK_SCSI_GET_IMAGE_STATUS, 0x00, 0x00, 
                        0x00, 0x06, 0x00 };
  unsigned char result[6];
  int busy;
  if(debug)fprintf(stderr,"getimagestatus... ");
  do  { if (scsi_handle_cmd(cmd, 6, NULL, 0, result, sizeof(result)) <0 )
         {  fprintf(stderr, "getimagestatus: command failed\n");
            return(-1);
         }
        busy = result[0];
        if(busy)_sleep2(100);
      } while (busy);

  *bytes_per_line = result[1] | (result[2] << 8);
  *lines = result[3] | (result[4] << 8) | (result[5] << 16);
  if(debug)
   fprintf(stderr,"ok, bytes per line=%d, lines=%d\n",*bytes_per_line, *lines);
  return 0;
}


int readbuffer( unsigned char *buf, int lines, int bpl)
/*....................................................*/
{ scsi_6byte_cmd cmd = { MUSTEK_SCSI_READ_SCANNED_DATA, 0x00, 0x00, 
                         0x00, 0x00, 0x00};
  cmd[2] = (lines >> 16) & 0xff;
  cmd[3] = (lines >> 8)  & 0xff;
  cmd[4] = (lines >> 0)  & 0xff;
  if(debug)fprintf(stderr,"readbuffer...\n");
  if (scsi_handle_cmd(cmd, 6, NULL, 0, buf, lines*bpl ) <0 )
   {  fprintf(stderr, "readbuffer: command failed\n");
      return(-1);
   }
  return 0;
}

static void write_pnm_header (int fmt, int width, int height, 
                              int dpi, char *modename)
/*---------------------------------------------------------*/
{ printf ("P%i\n# %i dpi %s\n# scanner data follows\n%d %d\n",
           fmt,dpi,modename,width,height);
  if(fmt!=LART) printf ("255\n");
  _fsetmode(stdout, "b");
}

static int width(int mode, int bpl)
/*-------------------------------*/
{ int rc=bpl;
  if(mode==LART)rc*=8;
  if(mode==RGB)rc/=3;
  return rc;
}


#define MAX_LINE_DIST 40

typedef struct { struct 
                  { int            max_value;
		    int            peak_res;
		    int            dist[3];
                    int            index[3];    
		    int            quant[3];
                    unsigned char *buf[3]; 
		    int            ld_line;    
		    int            lmod3;
                  }ld;
               } scdta;



void init_scdata(scdta *s, int dist[3], int peak_res)
{ int color;
  s->ld.peak_res=peak_res;
  s->ld.max_value=MAX_DPI;
  for(color=0;color<3;color++)
    { s->ld.dist[color]=dist[color];
      s->ld.quant[color]=s->ld.max_value;
      s->ld.index[color]=-s->ld.dist[color];
    }   
  
  s->ld.buf[0]=NULL;
  s->ld.lmod3=-1;    
  s->ld.ld_line=0;    
  
}

void close_scdata(scdta *s)
{ if(s->ld.buf[0])free(s->ld.buf[0]);
}

int fixlinedistance ( scdta *s, int num_lines, int bpl, 
                      unsigned char *raw, unsigned char *out, 
                      int num_lines_total)
/*--------------------------------------------------------------*/			
/* return num_lines processed                                   */
{  unsigned char *out_end, *out_ptr, *raw_end = raw + num_lines * bpl;
   int c, num_saved_lines, line;
   int color_seq[3]={1,2,0};
  
  if (!s->ld.buf[0])
    { if(debug>4)fprintf(stderr,"fix_line_distance_block: "
	                        "allocating temp buffer of %d*%d bytes\n", 
		                 MAX_LINE_DIST, bpl);
      s->ld.buf[0] = (unsigned char *)malloc (MAX_LINE_DIST * (long) bpl);
      if (!s->ld.buf[0])
	{ if(debug)fprintf(stderr,"fix_line_distance_block: "
	                          "failed to malloc temporary buffer\n");
	  return 0;
	}
    }
  if(debug>4)
    { fprintf(stderr,"fix_line_distance_block: "
             "num_lines = %d num_lines_total = %d\n",
              num_lines,num_lines_total);
      fprintf(stderr,"fix_line_distance_block: "
             "s->ld.index = {%d, %d, %d}, s->ld.lmod3 = %d\n", 
  	     s->ld.index[0], s->ld.index[1], s->ld.index[2], s->ld.lmod3);
      fprintf(stderr,"fix_line_distance_block: "
             "s->ld.quant = {%d, %d, %d}, s->ld.max_value = %d\n",
	      s->ld.quant[0], s->ld.quant[1], s->ld.quant[2], s->ld.max_value);
      fprintf(stderr,"fix_line_distance_block: "
             "s->ld.peak_res = %d, s->ld.ld_line = %d\n",
              s->ld.peak_res, s->ld.ld_line);
    }
  num_saved_lines=s->ld.index[0]-s->ld.index[2];
  if((num_saved_lines<0)||(s->ld.index[0]==0))num_saved_lines=0;
  /* restore the previously saved lines: */
  memcpy(out,s->ld.buf[0],num_saved_lines*bpl);
  
  if(debug>4)fprintf(stderr,"fix_line_distance_block: "
               "copied %d lines from ld.buf to buffer\n", num_saved_lines);
	       
  while (1)
    {
      if(++s->ld.lmod3>=3)s->ld.lmod3 = 0;
      c = color_seq[s->ld.lmod3];
      if(s->ld.index[c]<0)++s->ld.index[c];
      else if(s->ld.index[c]<num_lines_total)
	{
	  s->ld.quant[c]+=s->ld.peak_res;
	  if(s->ld.quant[c]>s->ld.max_value)
	    {
	      s->ld.quant[c] -= s->ld.max_value;
	      line = s->ld.index[c]++ - s->ld.ld_line;
	      out_ptr = out + line * bpl + c;
	      out_end = out_ptr + bpl;
	      while (out_ptr != out_end)
		{ *out_ptr = *raw++;
		   out_ptr += 3;
		}
	      if(debug>4)fprintf(stderr,"fix_line_distance_block: "
	                "copied line %4d (color %d)\n",line+s->ld.ld_line,c);

	      if ((raw >= raw_end) || 
	          ((s->ld.index[0] >= num_lines_total) &&
		   (s->ld.index[1] >= num_lines_total) &&
		   (s->ld.index[2] >= num_lines_total)))
		{ if(debug>4)fprintf(stderr,"fix_line_distance_block: "
		             "got num_lines: %d\n", num_lines);
		  num_lines = s->ld.index[2] - s->ld.ld_line;
		  if (num_lines < 0) num_lines = 0;
		  
		  /* copy away the lines with at least one missing
		     color component, so that we can interleave them
		     with new scan data on the next call */
		  num_saved_lines = s->ld.index[0] - s->ld.index[2];
		  
		  memcpy (s->ld.buf[0], out + num_lines * bpl,
			  num_saved_lines * bpl);
			  
		  if(debug>4)fprintf(stderr,"fix_line_distance_block: "
		       "copied %d lines to ld.buf\n", num_saved_lines);
		  
		  /* notice the number of lines we processed */
		  s->ld.ld_line = s->ld.index[2];
		  if (s->ld.ld_line < 0) s->ld.ld_line = 0;
		  
		  if(debug>3)fprintf(stderr,"fix_line_distance_block: "
		        "lmod3=%d, index=(%d,%d,%d), line = %d, lines = %d\n",
		         s->ld.lmod3,
		         s->ld.index[0], s->ld.index[1], s->ld.index[2],
		         s->ld.ld_line, num_lines);
		  /* return number of complete (r+g+b) lines */
		  return num_lines;
		}
	    }
	}
    }
}


void gamma_table(unsigned char t[256], double gamma,
                 double rel_black, double rel_white)
/*------------------------------------------------*/
{  int i,n=256;
   int black=(int)((n-1)*rel_black+0.5),
       white=(int)((n-1)*rel_white+0.5);
   double y,g=1.0/gamma,maxy=n-1,delta;

   if(black<0)black=0;
   if(white>(n-1))white=n-1;
   if(white<=black)white=black+1;
   delta=white-black;

   for(i=0;i<n;i++)
     { if(i<=black)t[i]=0;
       else if(i>=white)t[i]=n-1;
       else
        { y=maxy*pow((i-black)/delta,g)+0.5;
          t[i]=(int)floor(y);
        }
     }
}

static unsigned char gt[256];
static int           gammainit=0;

void gamma_correction(unsigned char *buf, int n)
/*---------------------------------------------*/
{ if(!gammainit) { gamma_table(gt,gamma,black_limit,white_limit); 
                   gammainit=1; 
                 }
  while(n--) { *buf=gt[*buf]; buf++; } 
}

write_pnm_data(int mode, char *buf, int bpl, int lines,
               int res, int peak, int dist[3], int start,
               unsigned char *out, scdta *s, int lines_total)
/*------------------------------------------------------*/
{ switch(mode)  
   { case LART: { int i,j; char *p=buf;
                  for(i=0;i<lines;i++,p+=bpl)
                   { for(j=0;j<bpl;j++)p[j]=~p[j];
                     switch(format)
                           { case FMT_PS : 
                               write_eps_bitmap(0,start--,bpl,1,p);
                               break;
                             case FMT_PNM:
                             default     : fwrite(p,1,bpl,stdout);
                           }
                   }
                }
          break;  
     case GRAY: { int i; char *p=buf;
                  for(i=0;i<lines;i++,p+=bpl)
                       { gamma_correction((unsigned char *)p,bpl);
                         switch(format)
                           { case FMT_PS : 
                               write_eps_graymap(0,start--,bpl,1,p);
                               break;
                             case FMT_PNM:
                             default     : fwrite(p,1,bpl,stdout);
                           }
                       }
                }  
          break;  
     case RGB:  { int  y,out_lines;
                  unsigned char *p;  
                  out_lines=fixlinedistance(s,lines,bpl,buf,out,lines_total);
                  p=out; 
                  if(debug>4)
                   { fprintf(stderr,"write_pnm_data: "
                     "out_lines = %d start = %d\n",
                      out_lines,start);
                   }
                  for(y=0;y<out_lines;y++,p+=bpl)
                    { gamma_correction((unsigned char *)p,bpl);
                      switch(format)
                        { case FMT_PS : 
                            write_eps_colormap(0,start--,bpl,1,p);
                            break;
                          case FMT_PNM:
                          default     : fwrite(p,1,bpl,stdout);
                        }
                    }
                }
          break;
     default:;
   }
} 

static int buffer_lines(int bpl)
/*----------------------------*/
{ int rc=scsi_maxdatasize()/bpl;
  return (rc/3)*3;
}



int readdata(int res, int mode, int lines, int bpl, int peak, int dist[3])
/*-----------------------------------------------------------------------*/
{ 
  int rest=lines,start,end;
  unsigned char *buf=(unsigned char *)malloc(buffer_lines(bpl)*bpl);
  unsigned char *out=0;
  scdta         *s=0;
  char *modename=mode==RGB?"color":mode==LART?"lineart":"gray";

  switch(format)
    { case FMT_PS : write_eps_header(width(mode,bpl),lines,res,modename);
                    break;
      case FMT_PNM:
      default     : write_pnm_header(mode,width(mode,bpl),lines,res,modename);
    }

  if(mode==RGB)
    { out=(unsigned char*)malloc((buffer_lines(bpl)+MAX_LINE_DIST)*bpl);
      s=(scdta*)malloc(sizeof(scdta));
      init_scdata(s,dist,peak);
    }
  while(rest)
   { 
     int m=buffer_lines(bpl);
     int n=rest<m?rest:m;
     start=lines-rest;
     end=start+n-1;

     if(debug||verbose)fprintf(stderr,"scanning from %5.1f%% to %5.1f%%... ",
                                      100.*(double)start/(lines-1),
                                      100.*(double)end/(lines-1));
     if(readbuffer(buf,n,bpl)==0)
       { 
         if(debug||verbose)fprintf(stderr,"postprocessing...\n"); 
         write_pnm_data(mode,buf,bpl,n,res,peak,dist,rest-1,out,s,n);       
                                                               /* ^ */                                          
         rest-=n;
       }
     else 
       { int newm=buffer_lines(bpl);
         if(newm==m)break;
       }
     if(s){  close_scdata(s);
             init_scdata(s,dist,peak);
          } 
   } 
  if(format==FMT_PS)write_eps_trailer();
  free(buf); 
  if(out)free(out);
  if(s) { close_scdata(s);
          free(s);
        }    
}



int ccddistance ( int res, int mode, int dpi_range_max,
                  int *peak_res, int dist[3])
/*----------------------------------------------------------------*/
/* Determine the CCD's distance between the primary color lines.  */
{ scsi_6byte_cmd cmd = { MUSTEK_SCSI_CCD_DISTANCE, 0x00, 0x00, 
                         0x00, 0x05, 0x00};
  unsigned char result[5];
  int factor, color;
  *peak_res=dpi_range_max;
  if(debug)fprintf(stderr,"ccddistance... ");
  if (scsi_handle_cmd(cmd, 6, NULL, 0, result, sizeof(result)) <0 )
   {  fprintf(stderr, "cdd_distance: command failed\n");
      return(-1);
   }

  factor = result[0] | (result[1] << 8);
  /* need to do line-distance adjustment ourselves... (color) */
  if(factor!=0xffff)
   { if (factor == 0){ if (res <= *peak_res / 2) res *= 2; }
     else res *= factor;
     *peak_res = res;
     for(color=0;color<3;++color)dist[color]=result[2+color];
   } 

  if(debug)
  fprintf(stderr,
   "ok, got factor=%d, (r/g/b)=(%d/%d/%d), peak res=%d\n",
    factor, dist[0], dist[1], dist[2], *peak_res);

  return 0;
}

int stopscan ()
/*===========*/
{ scsi_6byte_cmd cmd = {MUSTEK_SCSI_START_STOP, 0x00, 0x00,
                        0x00, 0x00, 0x00};
  if(debug)fprintf(stderr,"stopscan...\n");
  if (scsi_handle_cmd(cmd, 6, NULL, 0, 0, 0) <0 )
   {  fprintf(stderr, "stop_scan: command failed\n");
      return(-1);
   }
  return(0);
}

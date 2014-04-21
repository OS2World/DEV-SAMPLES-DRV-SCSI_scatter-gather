#ifndef __EPS_H
#define __EPS_H

#define CREATOR "scanner"

#define PS_IMAGEMASK   0
#define PS_IMAGE       1
#define PS_COLORIMAGE  2

void write_eps_header   (int w, int h, int dpi, char *modename);
void write_eps_trailer  ();
void write_eps_bitmap   (int x, int y, int bytes_per_row, int h, char *dta);
void write_eps_graymap  (int x, int y, int bytes_per_row, int h, char *dta);
void write_eps_colormap (int x, int y, int bytes_per_row, int h, char *dta);

#endif __EPS_H

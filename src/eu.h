#pragma once

#include "fileinput.h"
#include "display.h"

#include <stdlib.h>
#include <math.h>


// fastest to type while the right hand is on the mouse, copy/pasting the file name.
// this is true on a dvorak keyboard, where these are on the left homerow, middle + index fingers:
#define PROG_NAME "eu"
#define PROG_VERSION 1

typedef struct eu_gui_state_t
{
  mouse_t pointer;
  mouse_t pointer_button;
  float button_x, button_y;
  int dragging;
  int show_metadata;
}
eu_gui_state_t;


typedef struct eu_t
{
  display_t *display;
  int num_files;
  int current_file;
  fileinput_t *file;
  fileinput_conversion_t conv;

  uint8_t *pixels;
  eu_gui_state_t gui;
}
eu_t;

static inline void eu_init(eu_t *eu, int wd, int ht, int argc, char *arg[])
{
  eu->display = display_open(PROG_NAME, wd, ht);

  memset(&eu->gui, 0, sizeof(eu_gui_state_t));

  eu->conv.verbosity = s_silent;

  // default input to display conversion:
  eu->conv.roi_out.scale = 0.0f; // not used, invalidate
  eu->conv.colorin = s_xyz;
  eu->conv.colorout = s_srgb;
  eu->conv.gamutmap = s_gamut_clamp;
  eu->conv.curve = s_none;
  eu->conv.channels = s_rgb;

  const char *home = getenv("HOME");
  char file[1024];
  snprintf(file, 1024, "%s/.config/eu/eurc", home);
  FILE *f = fopen(file, "rb");
  if(f)
  {
    int v;
    fread(&v, 1, sizeof(int), f);
    if(v == PROG_VERSION)
    {
      fread(&eu->conv, 1, sizeof(fileinput_conversion_t), f);
      fread(&eu->gui, 1, sizeof(eu_gui_state_t), f);
    }
    fclose(f);
  }

  eu->conv.roi.x = 0;
  eu->conv.roi.y = 0;
  eu->conv.roi.scale = 1.0f;
  eu->conv.roi_out.x = 0;
  eu->conv.roi_out.y = 0;
  eu->conv.roi_out.w = wd;
  eu->conv.roi_out.h = ht;

  eu->num_files = argc-1;
  eu->file = (fileinput_t *)aligned_alloc(16, (argc-1)*sizeof(fileinput_t));
  for(int k=1;k<argc;k++)
  {
    if(fileinput_open(eu->file+k-1, arg[k]))
    {
      // just go on with empty frames.
      fprintf(stderr, "[eu_init] could not open file `%s'\n", arg[k]);
    }
  }

  // use dimensions of first file
  eu->conv.roi.w = fileinput_width(eu->file);
  eu->conv.roi.h = fileinput_height(eu->file);
  eu->current_file = 0;

  eu->pixels = (uint8_t *)aligned_alloc(16, wd*ht*3);
}

static inline void eu_cleanup(eu_t *eu)
{
  // write config file:
  const char *home = getenv("HOME");
  char dir[1024];
  snprintf(dir, 1024, "%s/.config/eu", home);
  mkdir(dir, 0775);
  snprintf(dir, 1024, "%s/.config/eu/eurc", home);
  FILE *f = fopen(dir, "wb");
  if(f)
  {
    int v = PROG_VERSION;
    fwrite(&v, 1, sizeof(int), f);
    fwrite(&eu->conv, 1, sizeof(fileinput_conversion_t), f);
    fwrite(&eu->gui, 1, sizeof(eu_gui_state_t), f);
    fclose(f);
  }
  for(int k=0;k<eu->num_files;k++)
    fileinput_close(eu->file+k);
  display_close(eu->display);
  free(eu->pixels);
  free(eu->file);
}


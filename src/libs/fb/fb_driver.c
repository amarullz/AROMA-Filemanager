/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename    : fb_driver.c
 * Description : linux framebuffer driver
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_driver_c__
#define __libaroma_linux_fb_driver_c__

/*
 *   Using Linux Framebuffer for Android & Linux
 *   Prefix : LINUXFBDR_
 */

/*
 * headers
 */
#include <linux/fb.h>

/*
 * device path
 */
#define LINUXFBDR_DEVICE              "/dev/graphics/fb0"
#define LINUXFBDR_DEVICE_NON_ANDROID  "/dev/fb0"

/*
 * structure : internal framebuffer data
 */
typedef struct {
  int       fb;                         /* framebuffer handler */
  byte      is32;                       /* is 32bit framebuffer? */
  struct    fb_fix_screeninfo   fix;    /* linux framebuffer fix info */
  struct    fb_var_screeninfo   var;    /* linux framebuffer var info */
  int       fb_sz;                      /* framebuffer memory size */
  voidp     buffer;                     /* direct buffer */
  int       stride;                     /* stride size */
  int       line;                       /* line size */
  byte      depth;                      /* color depth */
  byte      pixsz;                      /* memory size per pixel */
  int       synced;                     /* there is synced data */
  byte      rgb_pos[6];                 /* framebuffer 32bit rgb position */
  byte      active;
  pthread_t thread;
  
  /* ion */
  byte      ion;
  int       ion_sz;
  int       ion_fd;
  int       ion_memfd;
  voidp     ion_handle;
  voidp     ion_buffer;
  int       ion_overlay_id;
  
  LIBAROMA_MUTEX  mutex;
} LINUXFBDR_INTERNAL, *LINUXFBDR_INTERNALP;

/*
 * forward functions
 */
void LINUXFBDR_release(LIBAROMA_FBP me);
byte LINUXFBDR_flush(LIBAROMA_FBP me);
void LINUXFBDR_dump(LINUXFBDR_INTERNALP mi);

/*
 * include colorspace drivers
 */
#include "fb_colorspace/fb_16bit.c" /* 16 bit */
#include "fb_colorspace/fb_32bit.c" /* 32 bit */
#include "fb_ion/fb_ion.c" /* ion overlay */

static void * LINUXFBDR_flush_thread(void * cookie){
  LIBAROMA_FBP me=(LIBAROMA_FBP) cookie;
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal; 
  if (mi->ion){
    LINUXFBDR_ion_loop(me);
  }
  else{
    while(mi->active){
      if (!mi->synced){
        LINUXFBDR_flush(me);
      }
      usleep(16);
    }
  }
  return NULL;
}

/*
 * function : framebuffer driver initializer
 */
byte LINUXFBDR_init(LIBAROMA_FBP me) {
  LOGV("LINUXFBDR initialized internal data\n");
  /* allocating internal data */
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP)
                      malloc(sizeof(LINUXFBDR_INTERNAL));
  if (!mi) {
    LOGE("LINUXFBDR malloc internal data - memory error\n");
    return 0;
  }

  memset(mi, 0, sizeof(LINUXFBDR_INTERNAL)); /* cleanup */
  me->internal = (voidp) mi; /* set internal address */
  me->release = &LINUXFBDR_release; /* set release and refresh callback */

  libaroma_mutex_init(mi->mutex);

  mi->fb = open(LINUXFBDR_DEVICE, O_RDWR, 0); /* open framebuffer device */
  if (mi->fb < 1) {
    /* if not works, try non android standard device path */
    mi->fb = open(LINUXFBDR_DEVICE_NON_ANDROID, O_RDWR, 0);
  }
  if (mi->fb < 1) {
    /* cannot find device */
    LOGE("LINUXFBDR no framebuffer device\n");
    goto error; /* exit if error */
  }
  
  /* get framebuffer var & fix data */
  ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix); /* fix info */
  ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var); /* var info */
  
  if (mi->var.bits_per_pixel == 24) {
    /* 24bit is not supported - sorry */
    LOGE("LINUXFBDR 24bit framebuffer not supported\n");
    goto error; /* Exit If Error */
  }
  
  /* set libaroma framebuffer instance values */
  me->w        = mi->var.xres;              /* width */
  me->h        = mi->var.yres;              /* height */
  me->sz       = me->w * me->h;             /* width x height */
  
  /* set internal useful data */
  mi->line      = mi->fix.line_length;      /* line memory size */
  mi->depth     = mi->var.bits_per_pixel;   /* color depth */
  mi->pixsz     = mi->depth >> 3;           /* pixel size per byte */
  mi->fb_sz     = (me->sz * mi->pixsz);     /* framebuffer size */
  mi->synced    = 0;
  mi->var.yoffset = 0;
  
  /* map buffer */
  LOGV("LINUXFBDR mmap Framebuffer Memory\n");
  mi->buffer  = (voidp) mmap(
                  0, mi->fix.smem_len,
                  PROT_READ | PROT_WRITE, MAP_SHARED,
                  mi->fb, 0
                );

  if (!mi->buffer) {
    LOGE("LINUXFBDR mmap framebuffer memory error\n");
    goto error; /* exit if error */
  }
  
  if (LINUXFBDR_ion_init(me)){
    me->sync = &LINUXFBDR_sync_ion;
    me->snapshoot=NULL;
  }
  else if (mi->pixsz == 2) {
    /* Not 32bit Depth */
    mi->is32 = 0;
    /* Init Colorspace */
    LINUXFBDR_init_16bit(me);
    /* Set Sync Callbacks */
    me->sync     = &LINUXFBDR_sync_16bit;
    me->snapshoot = &LINUXFBDR_snapshoot_16bit;
  }
  else {
    mi->is32 = 1; /* It is 32bit Depth */
    /* Init Colorspace */
    LINUXFBDR_init_32bit(me);
    /* Set Sync Callbacks */
    me->sync     = &LINUXFBDR_sync_32bit;
    me->snapshoot = &LINUXFBDR_snapshoot_32bit;
  }
  
  /* DUMP INFO */
  LINUXFBDR_dump(mi);
  
  me->dpi = 0;
  int dpi_fallback = floor(MIN(mi->var.xres,mi->var.yres)/160) * 80;
  if ((mi->var.width<= 0)||(mi->var.height <= 0)) {
    /* phone dpi */
    me->dpi = dpi_fallback;
  }
  else{
    /* Calculate DPI */
    me->dpi = round(mi->var.xres / (mi->var.width * 0.039370) / 80) * 80;
  }
  if ((me->dpi<160)||(me->dpi>960)){
    me->dpi = dpi_fallback;
  }

  /* start flush thread */
  mi->active=1;
  pthread_create(&mi->thread, NULL, LINUXFBDR_flush_thread, (voidp) me);

  /* ok */
  goto ok;
  /* return */
error:
  free(mi);
  return 0;
ok:
  return 1;
}

/*
 * Function : release framebuffer instance (same for all colorspace)
 */
void LINUXFBDR_release(LIBAROMA_FBP me) {
  if (me == NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  
  /* wait thread */
  mi->active=0;
  pthread_join(mi->thread, NULL);
  
  LINUXFBDR_ion_release(me);
  
  LOGV("LINUXFBDR munmap buffer\n");
  munmap(mi->buffer, mi->fix.smem_len); /* unmap */
  LOGV("LINUXFBDR close fb-fd\n");
  close(mi->fb); /* close fb */
  LOGV("LINUXFBDR free internal data\n");
  
  libaroma_mutex_free(mi->mutex);
  free(me->internal); /* free internal data */
}

/*
 * Function : flush synced framebuffer - FBIOPAN_DISPLAY
 */
byte LINUXFBDR_flush(LIBAROMA_FBP me) {
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  
  /* check if there is synced data */
  fsync(mi->fb);
  
  /* refresh display */
  mi->var.activate = FB_ACTIVATE_VBL;
  mi->var.yoffset = 0;
  if (ioctl(mi->fb, FBIOPAN_DISPLAY, &mi->var)){
    mi->var.activate = FB_ACTIVATE_FORCE|FB_ACTIVATE_NOW;
    mi->var.yoffset = 0;
    ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
  }
	return 1;
}

/*
 * Function : dump framebuffer informations
 */
void LINUXFBDR_dump(LINUXFBDR_INTERNALP mi) {
  LOGS("FRAMEBUFFER INFORMATIONS:\n");
  LOGS("VAR\n");
  LOGS(" xres           : %i\n", mi->var.xres);
  LOGS(" yres           : %i\n", mi->var.yres);
  LOGS(" xres_virtual   : %i\n", mi->var.xres_virtual);
  LOGS(" yres_virtual   : %i\n", mi->var.yres_virtual);
  LOGS(" xoffset        : %i\n", mi->var.xoffset);
  LOGS(" yoffset        : %i\n", mi->var.yoffset);
  LOGS(" bits_per_pixel : %i\n", mi->var.bits_per_pixel);
  LOGS(" grayscale      : %i\n", mi->var.grayscale);
  LOGS(" red            : %i, %i, %i\n", 
    mi->var.red.offset, mi->var.red.length, mi->var.red.msb_right);
  LOGS(" green          : %i, %i, %i\n", 
    mi->var.green.offset, mi->var.green.length, mi->var.red.msb_right);
  LOGS(" blue           : %i, %i, %i\n", 
    mi->var.blue.offset, mi->var.blue.length, mi->var.red.msb_right);
  LOGS(" transp         : %i, %i, %i\n", 
    mi->var.transp.offset, mi->var.transp.length, mi->var.red.msb_right);
  LOGS(" nonstd         : %i\n", mi->var.nonstd);
  LOGS(" activate       : %i\n", mi->var.activate);
  LOGS(" height         : %i\n", mi->var.height);
  LOGS(" width          : %i\n", mi->var.width);
  LOGS(" accel_flags    : %i\n", mi->var.accel_flags);
  LOGS(" pixclock       : %i\n", mi->var.pixclock);
  LOGS(" left_margin    : %i\n", mi->var.left_margin);
  LOGS(" right_margin   : %i\n", mi->var.right_margin);
  LOGS(" upper_margin   : %i\n", mi->var.upper_margin);
  LOGS(" lower_margin   : %i\n", mi->var.lower_margin);
  LOGS(" hsync_len      : %i\n", mi->var.hsync_len);
  LOGS(" vsync_len      : %i\n", mi->var.vsync_len);
  LOGS(" sync           : %i\n", mi->var.sync);
  LOGS(" rotate         : %i\n", mi->var.rotate);
  
  LOGS("FIX\n");
  LOGS(" id             : %s\n", mi->fix.id);
  LOGS(" smem_len       : %i\n", mi->fix.smem_len);
  LOGS(" type           : %i\n", mi->fix.type);
  LOGS(" type_aux       : %i\n", mi->fix.type_aux);
  LOGS(" visual         : %i\n", mi->fix.visual);
  LOGS(" xpanstep       : %i\n", mi->fix.xpanstep);
  LOGS(" ypanstep       : %i\n", mi->fix.ypanstep);
  LOGS(" ywrapstep      : %i\n", mi->fix.ywrapstep);
  LOGS(" line_length    : %i\n", mi->fix.line_length);
  LOGS(" accel          : %i\n", mi->fix.accel);
}

/*
 * Function : libaroma init fb driver
 */
byte __linux_fb_driver_init(LIBAROMA_FBP me) {
  return LINUXFBDR_init(me);
}

#endif /* __libaroma_linux_fb_driver_c__ */

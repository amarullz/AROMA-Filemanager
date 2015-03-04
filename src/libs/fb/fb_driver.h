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
 * Filename    : fb_driver.h
 * Description : linux framebuffer driver header
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 03/03/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_driver_h__
#define __libaroma_linux_fb_driver_h__

/*
 * headers
 */
#include <linux/fb.h>
#include <aroma.h>
#ifdef LIBAROMA_CONFIG_OPENMP
  #include <omp.h>
#endif

typedef struct _LINUXFBDR_INTERNAL LINUXFBDR_INTERNAL;
typedef struct _LINUXFBDR_INTERNAL * LINUXFBDR_INTERNALP;

/* include qcom header */
#include "fb_qcom/fb_qcom.h"

/*
 * device path
 */
#define LINUXFBDR_DEVICE              "/dev/graphics/fb0"
#define LINUXFBDR_DEVICE_NON_ANDROID  "/dev/fb0"

/* omap ioctl */
#define OMAP_IOW(num, dtype) _IOW('O', num, dtype)
#define OMAP_IO(num) _IO('O', num)
#define OMAPFB_ENABLEVSYNC OMAP_IOW(64, int)
#define OMAPFB_WAITFORVSYNC OMAP_IO(57)

/*
 * structure : internal framebuffer data
 */
struct _LINUXFBDR_INTERNAL{
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
  byte      rgb_pos[6];                 /* framebuffer 32bit rgb position */

  byte      double_buffering;           /* is double buffering? */
  voidp     current_buffer;             /* current buffer to write */
  
  byte      active;                     /* thread active */
  pthread_t thread;                     /* flush thread handle */
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  
  byte      is_omap;                    /* is omap fb? - vsync */
  QCOMFB_INTERNALP  qcom;               /* qcom fb internal data */
};

/* release function */
void LINUXFBDR_release(LIBAROMA_FBP me);

/* init features */
void LINUXFBDR_init_features(LIBAROMA_FBP me);

/* flush function */
byte LINUXFBDR_flush(LIBAROMA_FBP me);

/* set dpi */
void LINUXFBDP_set_dpi(LIBAROMA_FBP me);

/* dump framebuffer info function */
void LINUXFBDR_dump(LINUXFBDR_INTERNALP mi);

/* swap back buffer */
void LINUXFBDR_swap_buffer(LINUXFBDR_INTERNALP mi);

/* flush receiver thread */
static void * LINUXFBDR_flush_receiver(void * cookie);

#endif /* __libaroma_linux_fb_driver_h__ */

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
 * Filename    : fb.h
 * Description : framebuffer
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 19/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_fb_h__
#define __libaroma_fb_h__

#define LIBAROMA_MUTEX pthread_mutex_t
#define libaroma_mutex_init(x) pthread_mutex_init(&x,NULL)
#define libaroma_mutex_free(x) pthread_mutex_destroy(&x)
#define libaroma_mutex_lock(x) pthread_mutex_lock(&x)
#define libaroma_mutex_unlock(x) pthread_mutex_unlock(&x)

/*
 * Typedef     : LIBAROMA_FB
 * Descriptions: Framebuffer type structure
 */
typedef struct _LIBAROMA_FB LIBAROMA_FB;
typedef struct _LIBAROMA_FB * LIBAROMA_FBP;

/*
 * Typedef     : LIBAROMA_FBCB_*
 * Descriptions: Framebuffer Callbacks
 */
typedef void (*LIBAROMA_FBCB_RELEASE) \
  (LIBAROMA_FBP);
typedef byte (*LIBAROMA_FBCB_SYNC) \
  (LIBAROMA_FBP, wordp, int, int, int, int);
typedef byte (*LIBAROMA_FBCB_SNAPSHOOT) \
  (LIBAROMA_FBP, wordp);
typedef byte (*LIBAROMA_FBCB_CONFIG) \
  (LIBAROMA_FBP, const char *, const char *, dword);

/*
 * Structure   : _LIBAROMA_FB
 * Typedef     : LIBAROMA_FB, * LIBAROMA_FBP
 * Descriptions: Framebuffer Structure
 */
struct _LIBAROMA_FB{
  /* main info */
  int w;            /* width */
  int h;            /* height */
  int sz;           /* width x height */
  voidp internal;   /* driver internal data */
  
  /* callbacks */
  LIBAROMA_FBCB_RELEASE release;
  LIBAROMA_FBCB_SYNC sync;
  LIBAROMA_FBCB_SNAPSHOOT snapshoot;
  LIBAROMA_FBCB_CONFIG config;
  
  /* Optional - DPI */
  byte driver_canvas;
  int dpi;
  byte bigscreen;
  
  /* AROMA CORE Runtime Data */
  wordp canvas;
};
  
/*
 * Function    : libaroma_fb
 * Return Value: LIBAROMA_FBP
 * Descriptions: get framebuffer instance
 */
LIBAROMA_FBP libaroma_fb();

/*
 * Function    : libaroma_fb_sync
 * Return Value: byte
 * Descriptions: sync framebuffer
 */
byte libaroma_fb_sync();

/*
 * Function    : libaroma_fb_sync_area
 * Return Value: byte
 * Descriptions: sync framebuffer area
 */
byte libaroma_fb_sync_area(
  int x,
  int y,
  int w,
  int h);

/*
 * Function    : libaroma_fb_is_landscape
 * Return Value: byte
 * Descriptions: is landscape?
 */
byte libaroma_fb_is_landscape();

/*
 * Function    : libaroma_dp
 * Return Value: int
 * Descriptions: convert px to dp
 */
int libaroma_dp(
    int dp);

/*
 * Function    : libaroma_px
 * Return Value: int
 * Descriptions: convert dp to px
 */
int libaroma_px(
    int px);

/*
 * Function    : libaroma_width_dp
 * Return Value: int
 * Descriptions: display width in dp
 */
int libaroma_width_dp();

/*
 * Function    : libaroma_height_dp
 * Return Value: int
 * Descriptions: display height in dp
 */
int libaroma_height_dp();

/*
 * Function    : libaroma_fb_snapshoot
 * Return Value: byte
 * Descriptions: copy display into framebuffer canvas
 */
byte libaroma_fb_snapshoot();

#define libaromafb() libaroma_fb()
/* sync & refresh aliases */
#define libaroma_sync() \
    libaroma_fb_sync()
#define libaroma_sync_ex(x,y,w,h) \
    libaroma_fb_sync_area(x,y,w,h)

#endif /* __libaroma_fb_h__ */

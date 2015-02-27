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
 * Filename    : fb.c
 * Description : framebuffer
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 19/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_fb_c__
#define __libaroma_fb_c__
#include "engine.c"
#include "fb_driver.c"

/*
 * Variable    : _libaroma_fb
 * Type        : LIBAROMA_FBP
 * Descriptions: framebuffer instance storage
 */
static LIBAROMA_FBP _libaroma_fb=NULL;

/*
 * Function    : libaroma_fb
 * Return Value: LIBAROMA_FBP
 * Descriptions: get framebuffer instance
 */
LIBAROMA_FBP libaroma_fb() {
  return _libaroma_fb;
} /* End of libaroma_fb */


/*
 * Function    : libaroma_fb_init
 * Return Value: LIBAROMA_FBP
 * Descriptions: init framebuffer
 */
LIBAROMA_FBP libaroma_fb_init() {
  if (_libaroma_fb != NULL) {
    LOGE("libaroma_fb_init framebuffer already initialized\n");
    return NULL;
  }
  
  /* allocating instance memory */
  LOGV("libaroma_fb_init allocating framebuffer instance\n");
  _libaroma_fb = (LIBAROMA_FBP) malloc(sizeof(LIBAROMA_FB));
  if (!_libaroma_fb){
    LOGE("libaroma_fb_init allocating framebuffer instance failed\n");
    return NULL;
  }
  memset(_libaroma_fb, 0, sizeof(LIBAROMA_FB));
  
  /* init driver */
  LOGV("Init framebuffer driver\n");
  if (__linux_fb_driver_init(_libaroma_fb) == 0) {
    free(_libaroma_fb);
    _libaroma_fb = NULL;
    LOGE("libaroma_fb_init driver error\n");
    return NULL;
  }
  
  /* check callbacks */
  if (
    (_libaroma_fb->release == NULL) ||
    (_libaroma_fb->sync == NULL)
  ) {
    free(_libaroma_fb);
    _libaroma_fb = NULL;
    LOGE("libaroma_fb_init driver doesn't set the callbacks\n");
    return NULL;
  }
  
  /* check dpi */
  if ((_libaroma_fb->dpi < 160)||(_libaroma_fb->dpi > 960)) {
    /* use phone dpi */
    _libaroma_fb->dpi = floor(MIN(_libaroma_fb->w, _libaroma_fb->h)/160) * 80;
    LOGW("libaroma_fb_init driver doesn't set dpi. set as : %i dpi\n",
      _libaroma_fb->dpi);
  }
  
  /* make sure the dpi is valid */
  if ((_libaroma_fb->dpi < 160)||(_libaroma_fb->dpi > 960)) {
    _libaroma_fb->dpi = 160;
  }
  
  /* check big screen */
  int dpMinWH = MIN(libaroma_width_dp(), libaroma_height_dp());
  _libaroma_fb->bigscreen = (dpMinWH >= 600); 
  
  /* create framebuffer canvas */
  if (!_libaroma_fb->driver_canvas){
    _libaroma_fb->canvas  = (wordp) malloc(_libaroma_fb->sz*2);
    memset(_libaroma_fb->canvas,0,_libaroma_fb->sz*2);
  }

  /* Show Information */
  LOGS("Framebuffer Initialized (%ix%ipx - %i dpi)\n",
    _libaroma_fb->w,
    _libaroma_fb->h,
    _libaroma_fb->dpi
  );
  
  /* Copy Current Framebuffer Into Display Canvas */
  if (_libaroma_fb->snapshoot!=NULL){
    LOGV("Copy framebuffer pixels into canvas\n");
    libaroma_fb_snapshoot();
  }
  
  /* Return The Instance */
  return _libaroma_fb;
} /* End of libaroma_fb_init */

/*
 * Function    : libaroma_fb_release
 * Return Value: byte
 * Descriptions: release framebuffer
 */
byte libaroma_fb_release() {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_fb_release framebuffer uninitialized\n");
    return 0;
  }
  
  /* Free display canvas */
  LOGV("Releasing Canvas\n");
  if (!_libaroma_fb->driver_canvas){
    free(_libaroma_fb->canvas);
  }
  
  /* Release Framebuffer Driver */
  LOGV("Releasing Framebuffer Driver\n");
  _libaroma_fb->release(_libaroma_fb);
  
  /* Show Information */
  LOGS("Framebuffer Released\n");
  
  /* Free Framebuffer Instance */
  free(_libaroma_fb);
  
  /* Set Null */
  _libaroma_fb = NULL;
  
  return 1;
} /* End of libaroma_fb_release */

/*
 * Function    : libaroma_fb_sync
 * Return Value: byte
 * Descriptions: sync framebuffer
 */
byte libaroma_fb_sync() {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_fb_sync framebuffer uninitialized\n");
    return 0;
  }
  
  /* sync */
  return _libaroma_fb->sync(
    _libaroma_fb,
    _libaroma_fb->canvas,
    0,
    0,
    0,
    0);
} /* End of libaroma_fb_sync */

/*
 * Function    : libaroma_fb_sync_area
 * Return Value: byte
 * Descriptions: sync framebuffer area
 */
byte libaroma_fb_sync_area(
  int x,
  int y,
  int w,
  int h) {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_fb_sync_area framebuffer uninitialized\n");
    return 0;
  }
  if (x < 0) {
    w -= x;
    x = 0;
  }
  if (y < 0) {
    h -= y;
    y = 0;
  }
  if (x + w > _libaroma_fb->w) {
    w = _libaroma_fb->w - x;
  }
  if (y + h > _libaroma_fb->h) {
    h = _libaroma_fb->h - x;
  }
  if ((w < 1) || (h < 1)) {
    LOGV("libaroma_fb_sync_area calculated width/height < 0 (%i,%i)\n", w, h);
    return 0;
  }
  
  /* sync */
  return _libaroma_fb->sync(
    _libaroma_fb,
    _libaroma_fb->canvas,
    x,
    y,
    w,
    h);
} /* End of libaroma_fb_sync_area */


/*
 * Function    : libaroma_fb_is_landscape
 * Return Value: byte
 * Descriptions: is landscape?
 */
byte libaroma_fb_is_landscape() {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_fb_is_landscape framebuffer uninitialized\n");
    return 0;
  }
  if (_libaroma_fb->w > _libaroma_fb->h) {
    return 1;
  }
  return 0;
} /* End of libaroma_fb_is_landscape */

/*
 * Function    : libaroma_dp
 * Return Value: int
 * Descriptions: convert px to dp
 */
int libaroma_dp(
    int dp) {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_dp framebuffer uninitialized\n");
    return 0;
  }
  return ((dp * _libaroma_fb->dpi) / 160);
} /* End of libaroma_dp */

/*
 * Function    : libaroma_px
 * Return Value: int
 * Descriptions: convert dp to px
 */
int libaroma_px(
    int px) {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_px framebuffer uninitialized\n");
    return 0;
  }
  return (px * 160 / _libaroma_fb->dpi);
} /* End of libaroma_px */

/*
 * Function    : libaroma_width_dp
 * Return Value: int
 * Descriptions: display width in dp
 */
int libaroma_width_dp() {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_width_dp framebuffer uninitialized\n");
    return 0;
  }
  return libaroma_px(_libaroma_fb->w);
} /* End of libaroma_width_dp */

/*
 * Function    : libaroma_height_dp
 * Return Value: int
 * Descriptions: display height in dp
 */
int libaroma_height_dp() {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_height_dp framebuffer uninitialized\n");
    return 0;
  }
  return libaroma_px(_libaroma_fb->h);
} /* End of libaroma_height_dp */

/*
 * Function    : libaroma_fb_snapshoot
 * Return Value: byte
 * Descriptions: copy display into framebuffer canvas
 */
byte libaroma_fb_snapshoot() {
  if (_libaroma_fb == NULL) {
    LOGW("libaroma_fb_snapshoot framebuffer uninitialized\n");
    return 0;
  }
  if (_libaroma_fb->snapshoot == NULL) {
    LOGW("framebuffer driver do not support snapshoot\n");
    return 0;
  }
  /* get */
  return _libaroma_fb->snapshoot(
    _libaroma_fb,
    _libaroma_fb->canvas
  );
} /* End of libaroma_fb_snapshoot */

#endif /* __libaroma_fb_c__ */

/*
 * Copyright (C) 2011-2013 Ahmad Amarullah ( http://amarullz.com/ )
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
 */

/*
 * UNIVERSAL DEVICE - TRANSLATOR FOR TOUCH DEVICE
 *   Prefix : INDR_
 *
 */
#ifndef __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_TOUCH__
#define __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_TOUCH__

/*
 * Defines
 */
#ifndef ABS_MT_POSITION
#define ABS_MT_POSITION 0x2a
#endif

/*
 * function : calculate touch event with calibration data
 */
static byte INDR_calibrate(
    AINPUTP me,
    INDR_POSP p,
    int * x,
    int * y) {
  /* no need calculating calibration */
  if (p->xi.minimum == p->xi.maximum || p->yi.minimum == p->yi.maximum) {
    *x = p->x;
    *y = p->y;
    return 1;
  }
  /* get internal data */
  INDR_INTERNALP mi = (INDR_INTERNALP) me->internal;
  /* screen */
  int fb_width  = me->screen_width;
  int fb_height = me->screen_height;
  if (mi->touch_swap_xy) {
    fb_width  = me->screen_height;
    fb_height = me->screen_width;
  }
  /* calculation */
  *x = (p->x - p->xi.minimum) * (fb_width - 1) /
    (p->xi.maximum - p->xi.minimum);
  *y = (p->y - p->yi.minimum) * (fb_height - 1) /
    (p->yi.maximum - p->yi.minimum);
  
  return 1;
  
  /* check result - disabled for capacitive button
  if (*x >= 0 && *x < fb_width &&
      *y >= 0 && *y < fb_height) {
    return 1;
  }
  
  return 0;
  */
}

/*
 * function : translate raw multitouch touch device
 */
byte INDR_translate_touch(
    AINPUTP me,
    INDR_DEVICEP dev,
    AINPUT_EVENTP dest_ev,
    struct input_event * ev) {
  /* get internal data */
  INDR_INTERNALP mi = (INDR_INTERNALP)
                      me->internal;
  /* dump raw events */
  static int MT_TRACKING_IS_UNTOUCHED = 0;
  static int TOUCH_RELEASE_NEXTSYN = 0;
  
  /* process EV_ABS event */
  if (ev->type==EV_ABS) {
    switch (ev->code) {
      case ABS_X:
        /* x only event */
        dev->p.state |= INDR_POS_ST_SYNC_X;
        dev->p.x = ev->value;
        break;
      case ABS_Y:
        /* y only event */
        dev->p.state |= INDR_POS_ST_SYNC_Y;
        dev->p.y = ev->value;
        break;
      case ABS_MT_POSITION_X:
        /* multitouch x only event */
        dev->p.state |= INDR_POS_ST_SYNC_X;
        dev->p.x = ev->value;
        break;
      case ABS_MT_POSITION_Y:
        /* multitouch y only event */
        dev->p.state |= INDR_POS_ST_SYNC_Y;
        dev->p.y = ev->value;
        break;
      case ABS_MT_POSITION:
        /* multitouch xy event */
        dev->p.state |= INDR_POS_ST_SYNC_X | INDR_POS_ST_SYNC_Y;
        
        if (ev->value == (1 << 31)) {
          dev->p.state |= INDR_POS_ST_LASTSYNC;
          /*dev->p.x = -1;
          dev->p.y = -1;*/
        }
        else {
          dev->p.state  &= ~INDR_POS_ST_LASTSYNC;
          dev->p.x = (ev->value & 0x7FFF0000) >> 16;
          dev->p.y = (ev->value & 0xFFFF);
        }
        ev->type = EV_SYN;
        ev->code = SYN_REPORT;
        break;
        
      case ABS_MT_TOUCH_MAJOR:
      case ABS_MT_PRESSURE:
        /* multitouch pressure event */
        if (ev->value == 0) {
          /* screen untouched */
          dev->p.state |= INDR_POS_ST_RLS_NEXT;
          /*dev->p.x = -1;
          dev->p.y = -1;*/
        }
        break;
        
      case ABS_MT_TRACKING_ID:
        if (ev->value < 0) {
          /* screen untouched */
          dev->p.state |= INDR_POS_ST_RLS_NEXT;
          /*dev->p.x = -1;
          dev->p.y = -1;*/
          TOUCH_RELEASE_NEXTSYN = 1;
          MT_TRACKING_IS_UNTOUCHED = 1;
        }
        break;
        
      default:
        /* unknown event */
        goto return_none;
    }
  }
  
  /* process ev_syn event */
  if (ev->type == EV_SYN) {
    if (ev->code == SYN_MT_REPORT) {
      goto return_none;
    }
    else if (ev->code != SYN_REPORT) {
      goto return_clear_sync;
    }
    
    if (((dev->p.state &
        (INDR_POS_ST_LASTSYNC | INDR_POS_ST_RLS_NEXT)) &&
        !MT_TRACKING_IS_UNTOUCHED) ||
        (MT_TRACKING_IS_UNTOUCHED && TOUCH_RELEASE_NEXTSYN == 1)) {
      /* set destination coordinate */
      TOUCH_RELEASE_NEXTSYN = 0;
      dest_ev->x      = dev->p.tx;
      dest_ev->y      = dev->p.ty;
      
      /* sometime it reported twice, so check this value */
      if ((dev->p.tx == -1) || (dev->p.tx == -1)) {
        /* log raw */
        dev->p.state    &= ~INDR_POS_ST_DOWNED;
        dev->p.state    &= ~INDR_POS_ST_RLS_NEXT;
        goto return_clear_sync;
      }
      
      /* reset translated coordinate */
      dev->p.tx       = -1;
      dev->p.ty       = -1;
      /* remove down flag */
      dev->p.state    &= ~INDR_POS_ST_DOWNED;
      /* reset release next */
      dev->p.state    &= ~INDR_POS_ST_RLS_NEXT;
      
      /* it was touch up event if not on virtualkey */
      if (!(dev->p.state & INDR_POS_ST_IS_VKEY)) {
        /* fill destination event */
        dest_ev->type   = AINPUT_EV_TYPE_TOUCH;
        dest_ev->state  = AINPUT_EV_STATE_UP;
        dest_ev->key    = 0;
      }
      else {
        /* reset virtual key flag */
        dev->p.state    &= ~INDR_POS_ST_IS_VKEY;
        /* set custom key event */
        struct input_event key_ev;
        key_ev.type = EV_KEY;
        key_ev.code = dev->vks[dev->p.vk].scan;
        /* state was cancel by default */
        key_ev.value = 3;
        /* check if still touch inside virtual key */
        int xd = abs(dev->vks[dev->p.vk].x - dest_ev->x);
        int yd = abs(dev->vks[dev->p.vk].y - dest_ev->y);
        
        if ((xd < dev->vks[dev->p.vk].w / 2) && (yd < dev->vks[dev->p.vk].h / 2)) {
          /* it still on virtual key. set as up */
          key_ev.value = 0;
        }
        
        /* reset virtual key id */
        dev->p.vk    = -1;
        /* if on virtual key - send as keyboard event */
        return INDR_translate_keyboard(me, dev, dest_ev, &key_ev);
      }
      
      return AINPUT_EV_RET_TOUCH;
    }
    
    /* set on EV_SYN */
    dev->p.state  |= INDR_POS_ST_LASTSYNC;
    
    /* calibrated x, y */
    int cx = -1;
    int cy = -1;
    
    
    INDR_calibrate(me, &dev->p, &cx, &cy);
    
    /* swap & flip handler */
    if (mi->touch_swap_xy) {
      cx ^= cy;
      cy ^= cx;
      cx ^= cy;
    }
    
    if (mi->touch_flip_x) {
      cx = me->screen_width - cx;
    }
    
    if (mi->touch_flip_y) {
      cy = me->screen_height - cy;
    }
    
    /* if we have nothing useful to report, skip it */
    if (cx == -1 || cy == -1 || dev->p.x == -1 || dev->p.y == -1) {
      goto return_none;
    }
    
    /* reset last sync xy event */
    dev->p.state    &= ~INDR_POS_ST_SYNC_X;
    dev->p.state    &= ~INDR_POS_ST_SYNC_Y;
    
    /* on first touch */
    if (!(dev->p.state & INDR_POS_ST_DOWNED)) {
      /* set downed */
      dev->p.state |= INDR_POS_ST_DOWNED;
      /* see if we're at a virtual key,
       * attempt mapping to virtual key
       */
      int i;
      for (i = 0; i < dev->vkn; i++) {
        int xd = abs(dev->vks[i].x - cx);
        int yd = abs(dev->vks[i].y - cy);
        if ((xd < dev->vks[i].w / 2) && (yd < dev->vks[i].h / 2)) {
          /* set as virtual key */
          dev->p.state |= INDR_POS_ST_IS_VKEY;
          /* set virtual key id */
          dev->p.vk    = i;
          /* set translated coordinat */
          dev->p.tx    = cx;
          dev->p.ty    = cy;
          /* set as custom key event */
          struct input_event key_ev;
          key_ev.type = EV_KEY;
          key_ev.code = dev->vks[i].scan;
          /* key event state = down */
          key_ev.value = 1;
          /* if on virtual key - send as keyboard event */
          return INDR_translate_keyboard(me, dev, dest_ev, &key_ev);
        }
      }
      /* set destination state as down event */
      dest_ev->state  = AINPUT_EV_STATE_DOWN;
    }
    /* on touch move */
    else {
      /* if it was virtual key, ignore it */
      if ((dev->p.state & INDR_POS_ST_IS_VKEY)) {
        /* set translated coordinat
         * needed for cancel virtual key event
         */
        dev->p.tx    = cx;
        dev->p.ty    = cy;
        /* don't send any event */
        goto return_none;
      }
      /* set destination state as move event */
      dest_ev->state  = AINPUT_EV_STATE_MOVE;
    }
    
    /* set translated coordinat */
    dev->p.tx    = cx;
    dev->p.ty    = cy;
    /* fill destination event */
    dest_ev->type   = AINPUT_EV_TYPE_TOUCH;
    dest_ev->key    = 0;
    dest_ev->x      = dev->p.tx;
    dest_ev->y      = dev->p.ty;
    /* set as touch event */
    return AINPUT_EV_RET_TOUCH;
  }
  
return_clear_sync:
  /* reset last sync event */
  dev->p.state &= ~INDR_POS_ST_LASTSYNC;
return_none:
  return AINPUT_EV_RET_NONE;
}
#endif // __AROMA_CORE_UNIVERSAL_INPUT_DRIVER_TRANSLATE_TOUCH__

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
 * Filename    : fb_ion.c
 * Description : linux framebuffer driver for ion overlay msmfb4X
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fbion_driver_c__
#define __libaroma_linux_fbion_driver_c__

#include "fb_ion.h"

static struct msmfb_overlay_data LINUXFBDR_overlay_data;
static struct mdp_display_commit LINUXFBDR_ion_commit;
  
/*
 * Free & release ion dev
 */
void LINUXFBDR_ion_release(LIBAROMA_FBP me){
  if (me == NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  if (mi->ion){
    if (mi->ion_overlay_id != MSMFB_NEW_REQUEST) {
      ioctl(mi->fb, MSMFB_OVERLAY_UNSET, &mi->ion_overlay_id);
      mi->ion_overlay_id=MSMFB_NEW_REQUEST;
      
      struct mdp_display_commit ext_commit;
      memset(&ext_commit, 0, sizeof(struct mdp_display_commit));
      ext_commit.flags = MDP_DISPLAY_COMMIT_OVERLAY;
      ext_commit.wait_for_finish = 1;
      ioctl(mi->fb,MSMFB_DISPLAY_COMMIT,&ext_commit);
    }
    
    if (me->driver_canvas){
      me->canvas=NULL;
      me->driver_canvas=0;
    }
    if (mi->ion_buffer){
      munmap(mi->ion_buffer, mi->ion_sz);
      mi->ion_buffer=0;
    }
    if (mi->ion_fd>=0) {
      ioctl(mi->ion_fd, ION_IOC_FREE, &mi->ion_handle);
    }

    if (mi->ion_memfd>=0){
      close(mi->ion_memfd);
      mi->ion_memfd=0;
    }
    if (mi->ion_fd>=0){
      close(mi->ion_fd);
      mi->ion_fd=0;
    }
    mi->ion=0;
  }
}

/*
 * Allocating overlay
 */
byte LINUXFBDR_ion_allocate_overlay(LIBAROMA_FBP me){
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;

  if (MSMFB_NEW_REQUEST == mi->ion_overlay_id) {
    struct mdp_overlay ovrl;
    memset(&ovrl, 0 , sizeof (struct mdp_overlay));

    /* fill overlay data */
    ovrl.src.format = MDP_RGB_565;
    ovrl.src.width  = me->w; // FB_IO_ALIGN(me->w, 32);
    ovrl.src.height = me->h;
    ovrl.src_rect.w = me->w;
    ovrl.src_rect.h = me->h;
    ovrl.dst_rect.w = me->w;
    ovrl.dst_rect.h = me->h;
    ovrl.alpha = 0xFF;
    ovrl.transp_mask = MDP_TRANSP_NOP;
    ovrl.id = MSMFB_NEW_REQUEST;
    ovrl.z_order = 1;
    if (ioctl(mi->fb, MSMFB_OVERLAY_SET, &ovrl)<0) {
      LOGS("Overlay Set Failed\n");
      return 0;
    }
    mi->ion_overlay_id = ovrl.id;
    
    /* prepare overlay data */
    memset(&LINUXFBDR_overlay_data, 0, sizeof(struct msmfb_overlay_data));
    LINUXFBDR_overlay_data.id = mi->ion_overlay_id;
    LINUXFBDR_overlay_data.data.flags = 0;
    LINUXFBDR_overlay_data.data.offset = 0;
    LINUXFBDR_overlay_data.data.memory_id = mi->ion_memfd;
    LOGS("FB ION Overlay ID#%i\n",mi->ion_overlay_id);
    
    /* prepare commit data */
    memset(&LINUXFBDR_ion_commit, 0, sizeof(struct mdp_display_commit));
    LINUXFBDR_ion_commit.flags = MDP_DISPLAY_COMMIT_OVERLAY;
    LINUXFBDR_ion_commit.wait_for_finish = 1;
  }
  return 1;
}

/*
 * Test & init ion dev
 */
byte LINUXFBDR_ion_init(LIBAROMA_FBP me){
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  
  /* check ion device */
  mi->ion=0;
  byte overlay_supported = 0;
  byte isMDP5 = 0;
  if(!strncmp(mi->fix.id, "msmfb", strlen("msmfb"))) {
    char str_ver[4];
    memcpy(str_ver, mi->fix.id + strlen("msmfb"), 3);
    str_ver[2] = '\0';
    int mdp_version = atoi(str_ver);
    LOGS("LINUXFBDR_ion_init ver: %i\n",mdp_version);
    if (mdp_version>=40) {
      overlay_supported = 1;
    }
  }
  else if (!strncmp(mi->fix.id, "mdssfb", strlen("mdssfb"))) {
    overlay_supported = 0; /* non support for now */
    isMDP5 = 1;
  }
  if (!overlay_supported){
    LOGS("LINUXFBDR_ion_init overlay_supported\n");
    return 0;
  }
  
  /* open ion device */
  mi->ion_fd = open("/dev/ion", O_RDWR|O_DSYNC);
  if (mi->ion_fd < 0) {
    LOGS("LINUXFBDR_ion_init mi->ion_fd<0\n");
    return 0;
  }
  
  mi->ion_sz = mi->fix.line_length * mi->var.yres;
  if (isMDP5){
    mi->ion=5;
  }
  else{
    mi->ion=4;
  }
  
  /* prepare ion */
  struct ion_fd_data fd_data;
  struct ion_allocation_data ionAllocData;
  ionAllocData.flags = 0;
  ionAllocData.len = mi->ion_sz;
  ionAllocData.align = sysconf(_SC_PAGESIZE);
  ionAllocData.heap_mask =
      ION_HEAP(ION_IOMMU_HEAP_ID) |
      ION_HEAP(ION_SYSTEM_CONTIG_HEAP_ID);
  
  /* alloc */
  if(ioctl(mi->ion_fd, ION_IOC_ALLOC,  &ionAllocData)){
    LOGS("ION_IOC_ALLOC Failed\n");
    close(mi->ion_fd);
    return 0;
  }
  
  /* map */
  fd_data.handle = ionAllocData.handle;
  mi->ion_handle = ionAllocData.handle;
  if (ioctl(mi->ion_fd, ION_IOC_MAP, &fd_data)) {
    LOGS("ION_IOC_MAP Failed\n");
    LINUXFBDR_ion_release(me);
    return 0;
  }
  
  /* mmap */
  mi->ion_buffer = (voidp) mmap(NULL, mi->ion_sz,
    PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
  mi->ion_memfd = fd_data.fd;

  if (!mi->ion_buffer) {
    LOGS("ERROR: ion_buffer MAP_FAILED \n");
    LINUXFBDR_ion_release(me);
    return 0;
  }
  
  mi->ion_overlay_id=MSMFB_NEW_REQUEST;
  if (LINUXFBDR_ion_allocate_overlay(me)){
    LOGS("LINUXFBDR using ION\n");
    me->driver_canvas = 1;
    me->canvas  = mi->ion_buffer;
    LOGS("LINUXFBDR ION use driver canvas\n");
    return 1;
  }
  LOGS("LINUXFBDR cannot allocate overlay\n");
  LINUXFBDR_ion_release(me);
  return 0;
}

/*
 * function : save display canvas into framebuffer
 */
byte LINUXFBDR_sync_ion(
    LIBAROMA_FBP me,
    wordp src,
    int x,
    int y,
    int w,
    int h) {
  if (me == NULL) {
    return 0;
  }
  
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  if (!me->driver_canvas){
    if ((w > 0)&&(h > 0)){
      /* defined area only */
      int copy_stride = (me->w-w)*2;
      int copy_pos    = (me->w*y)+x;
      wordp dst       = (wordp) mi->ion_buffer;
      libaroma_blt_align16(
        dst + copy_pos, src + copy_pos, w, h,
        copy_stride,copy_stride
      );
    }
    else {
      /* whole screen */
      memcpy(mi->ion_buffer, src, me->sz*2);
    }
  }
  
  
  //ioctl(mi->fb, MSMFB_OVERLAY_PLAY, &LINUXFBDR_overlay_data);
  libaroma_mutex_lock(mi->mutex);
  mi->synced=1;
  libaroma_mutex_unlock(mi->mutex);
  // ioctl(mi->fb,MSMFB_DISPLAY_COMMIT,&LINUXFBDR_ion_commit);
  
  return 1;
}

void LINUXFBDR_ion_loop(LIBAROMA_FBP me){
  if (me == NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  ioctl(mi->fb, MSMFB_OVERLAY_PLAY, &LINUXFBDR_overlay_data);
  while(mi->active){
    libaroma_mutex_lock(mi->mutex);
    if (mi->synced){
      mi->synced=0;
      libaroma_mutex_unlock(mi->mutex);
      ioctl(mi->fb,MSMFB_DISPLAY_COMMIT,&LINUXFBDR_ion_commit);
    }
    else{
      libaroma_mutex_unlock(mi->mutex);
      usleep(16);
    }
  }
}

#endif /* __libaroma_linux_fbion_driver_c__ */ 

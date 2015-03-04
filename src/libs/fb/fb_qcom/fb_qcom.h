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
 * Filename    : fb_qcom.h
 * Description : qcom overlay fb header - cherrypick from ion.h & msm_ion.h
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/02/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_qcom_driver_h__
#define __libaroma_linux_fb_qcom_driver_h__

/* ION HEADERS */
#define ION_HEAP(bit) (1 << (bit))
struct ion_handle;
struct ion_allocation_data {
	size_t len;
	size_t align;
	unsigned int heap_mask;
	unsigned int flags;
	struct ion_handle *handle;
};
struct ion_fd_data {
	struct ion_handle *handle;
	int fd;
};
struct ion_handle_data {
	struct ion_handle *handle;
};
enum ion_heap_ids {
	INVALID_HEAP_ID = -1,
	ION_CP_MM_HEAP_ID = 8,
	ION_CP_MFC_HEAP_ID = 12,
	ION_CP_WB_HEAP_ID = 16, /* 8660 only */
	ION_CAMERA_HEAP_ID = 20, /* 8660 only */
	ION_SYSTEM_CONTIG_HEAP_ID = 21,
	ION_ADSP_HEAP_ID = 22,
	ION_SF_HEAP_ID = 24,
	ION_IOMMU_HEAP_ID = 25,
	ION_QSECOM_HEAP_ID = 27,
	ION_AUDIO_HEAP_ID = 28,
	ION_MM_FIRMWARE_HEAP_ID = 29,
	ION_SYSTEM_HEAP_ID = 30,
	ION_HEAP_ID_RESERVED = 31 /** Bit reserved for ION_SECURE flag */
};
#define ION_IOC_MAGIC		'I'
#define ION_IOC_ALLOC		_IOWR(ION_IOC_MAGIC, 0, struct ion_allocation_data)
#define ION_IOC_MAP		  _IOWR(ION_IOC_MAGIC, 2, struct ion_fd_data)
#define ION_IOC_FREE		_IOWR(ION_IOC_MAGIC, 1, struct ion_handle_data)
#define MSMFB_NEW_REQUEST -1
#define QCOMFB_ALIGN_WIDTH(x, align) (((x) + ((align)-1)) & ~((align)-1))

#ifndef MAX_DISPLAY_DIM
#define MAX_DISPLAY_DIM 2048
#endif

enum mdss_mdp_stage_index {
	MDSS_MDP_STAGE_UNUSED,
	MDSS_MDP_STAGE_BASE,
	MDSS_MDP_STAGE_0,
	MDSS_MDP_STAGE_1,
	MDSS_MDP_STAGE_2,
	MDSS_MDP_STAGE_3,
	MDSS_MDP_STAGE_4,
	MDSS_MDP_MAX_STAGE
};

/* include msm_mdp */
#include "msm_mdp.h"


struct mdp_overlay_pp_params44 {
	uint32_t config_ops;
	struct mdp_csc_cfg csc_cfg;
	struct mdp_qseed_cfg qseed_cfg[2];
	struct mdp_pa_cfg pa_cfg;
	struct mdp_igc_lut_data igc_cfg;
	struct mdp_sharp_cfg sharp_cfg;
};
struct mdp_overlay_44 {
	struct msmfb_img src;
	struct mdp_rect src_rect;
	struct mdp_rect dst_rect;
	uint32_t z_order;	/* stage number */
	uint32_t is_fg;		/* control alpha & transp */
	uint32_t alpha;
	uint32_t transp_mask;
	uint32_t blend_op;
	uint32_t flags;
	uint32_t id;
	uint32_t user_data[8];
	struct mdp_overlay_pp_params44 overlay_pp_cfg;
};
#define MSMFB_OVERLAY_SET_44 \
  _IOWR(MSMFB_IOCTL_MAGIC, 135, struct mdp_overlay_44)

/* qcom internal data */
typedef struct{
  byte      id;
  int       ionfd;
  int       memfd;
  voidp     handle;
  
  int       yoffset;
  int       overlay_lid;
  int       overlay_rid;
  
  byte      split;
  int       split_left;
  int       split_right;
  byte      dbuf;
  
  struct msmfb_overlay_data overlay;
  struct mdp_display_commit commiter;
} QCOMFB_INTERNAL, * QCOMFB_INTERNALP;

/*
 * Function    : QCOMFB_release
 * Return Value: void
 * Descriptions: release qcom overlay driver
 */
void QCOMFB_release(LIBAROMA_FBP me);

/*
 * Function    : QCOMFB_check_id
 * Return Value: byte
 * Descriptions: check framebuffer id
 */
byte QCOMFB_check_id(LINUXFBDR_INTERNALP mi);

/*
 * Function    : QCOMFB_split_display
 * Return Value: void
 * Descriptions: get split display info
 */
void QCOMFB_split_display(LIBAROMA_FBP me);

/*
 * Function    : QCOMFB_allocate_overlays
 * Return Value: byte
 * Descriptions: allocate mdp overlays
 */
byte QCOMFB_allocate_overlays(LIBAROMA_FBP me);

/*
 * Function    : QCOMFB_swap_buffer
 * Return Value: void
 * Descriptions: qcom swap back buffer
 */
void QCOMFB_swap_buffer(LIBAROMA_FBP me);

/*
 * Function    : QCOMFB_sync
 * Return Value: byte
 * Descriptions: sync callback for qcom overlay
 */
byte QCOMFB_sync(LIBAROMA_FBP me, wordp src,int x, int y, int w, int h);

/*
 * Function    : QCOMFB_flush
 * Return Value: void
 * Descriptions: flush overlay update
 */
void QCOMFB_flush(LINUXFBDR_INTERNALP mi);

#endif /* __libaroma_linux_fb_qcom_driver_h__ */

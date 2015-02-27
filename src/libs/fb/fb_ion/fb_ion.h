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
 * Filename    : fb_ion.h
 * Description : fb ion header - modified and cherrypick from ion.h
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/02/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fbion_driver_h__
#define __libaroma_linux_fbion_driver_h__

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
#define FB_IO_ALIGN(x, align) (((x) + ((align)-1)) & ~((align)-1))

/* include msm_mdp */
#include "msm_mdp.h"

#endif /* __libaroma_linux_fbion_driver_h__ */

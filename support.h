/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <kwakwa_api/pictures.h>

APTR LoadFile(STRPTR path, ULONG *size);
struct Picture *LoadPictureFile(STRPTR path);
struct Picture *LoadPictureMemory(APTR data, QUAD *length);
struct Picture *CopyPicture(struct Picture *src);
VOID FreePicture(struct Picture *pic);

#endif /* __SUPPORT_H__ */

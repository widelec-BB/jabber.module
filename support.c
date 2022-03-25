/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/multimedia.h>
#include <classes/multimedia/video.h>
#include <classes/multimedia/streams.h>
#include <kwakwa_api/protocol.h>
#include "globaldefines.h"

extern struct Library *SysBase, *DOSBase, *IntuitionBase, *MultimediaBase;

APTR LoadFile(STRPTR path, ULONG *size)
{
	APTR result = NULL;
	BPTR fh;

	if(!size)
		return NULL;

	*size = 0;

	if((fh = Open(path, MODE_OLDFILE)) != NULL)
	{
		struct FileInfoBlock fh_fib;

		if(ExamineFH(fh, &fh_fib))
		{
			if((result = AllocMem(fh_fib.fib_Size, MEMF_ANY)) != NULL)
			{
				if(FRead(fh, result, fh_fib.fib_Size, 1) == 1)
				{
					*size = fh_fib.fib_Size;
					return result;
				}
				FreeMem(result, fh_fib.fib_Size);
				result = NULL;
			}
		}
		Close(fh);
	}
	return result;
}

struct Picture *LoadPictureFile(STRPTR path)
{
	struct Picture *result = NULL;
	Object *pic = NULL;

	tprintf("file: try to load: %s\n", path);

	pic = MediaNewObjectTags(
		MMA_StreamType, (IPTR)"file.stream",
		MMA_StreamName, (IPTR)path,
		MMA_MediaType, MMT_PICTURE,
	TAG_END);

	if(pic)
	{
		if((result = AllocMem(sizeof(struct Picture), MEMF_ANY)))
		{
			result->p_Height = MediaGetPort(pic, 0, MMA_Video_Height);
			result->p_Width = MediaGetPort(pic, 0, MMA_Video_Width);

			if((result->p_Data = (BYTE*)MediaAllocVec((result->p_Height * result->p_Width) << 2)))
			{
				DoMethod(pic, MMM_Pull, 0, (ULONG) result->p_Data, (result->p_Height * result->p_Width) << 2);
			}
		}
		DisposeObject(pic);
	}

	return result;
}

struct Picture *LoadPictureMemory(APTR data, QUAD *length)
{
	struct Picture *result = NULL;
	Object *pic = NULL;

	pic = MediaNewObjectTags(
		MMA_StreamType, (IPTR)"memory.stream",
		MMA_StreamHandle, (IPTR)data,
		MMA_StreamLength, (IPTR)length,
		MMA_MediaType, MMT_PICTURE,
	TAG_END);

	if(pic)
	{
		if((result = AllocMem(sizeof(struct Picture), MEMF_ANY)))
		{
			result->p_Height = MediaGetPort(pic, 0, MMA_Video_Height);
			result->p_Width = MediaGetPort(pic, 0, MMA_Video_Width);

			if((result->p_Data = (BYTE*)MediaAllocVec((result->p_Height * result->p_Width) << 2)))
				DoMethod(pic, MMM_Pull, 0, (ULONG) result->p_Data, (result->p_Height * result->p_Width) << 2);
		}
		DisposeObject(pic);
	}

	return result;
}

struct Picture *CopyPicture(struct Picture *src)
{
	struct Picture *result = NULL;

	if(src == NULL || src->p_Data == NULL)
		return NULL;

	if((result = AllocMem(sizeof(struct Picture), MEMF_ANY)))
	{
		result->p_Height = src->p_Height;
		result->p_Width = src->p_Width;

		if((result->p_Data = (BYTE*)MediaAllocVec((result->p_Height * result->p_Width) << 2)))
		{
			CopyMemQuick(src->p_Data, result->p_Data, (result->p_Height * result->p_Width) << 2);
		}
		else
		{
			FreeMem(result, sizeof(struct Picture));
			result = NULL; /* not longer valid */
		}
	}
	return result;
}

VOID FreePicture(struct Picture *pic)
{
	if(pic)
	{
		if(pic->p_Data)
			MediaFreeVec(pic->p_Data);

		FreeMem(pic, sizeof(struct Picture));
	}
}

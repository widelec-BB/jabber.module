/*
 * Copyright (c) 2014 - 2018 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/system.h>
#include <libraries/query.h>
#include <clib/alib_protos.h>
#include <dos/dostags.h>
#include "locale.h"
#include "class.h"

const char LibName[] = CLASSNAME;
extern const char VTag[];

struct Library *SysBase, *DOSBase, *IntuitionBase, *UtilityBase, *MUIMasterBase, *LocaleBase, *MultimediaBase;

struct Library *LibInit(struct Library *unused, APTR seglist, struct Library *sysb);
struct ClassBase *lib_init(struct ClassBase *cb, APTR seglist, struct Library *SysBase);
APTR lib_expunge(struct ClassBase *cb);
struct Library *LibOpen(void);
ULONG LibClose(void);
APTR LibExpunge(void);
ULONG LibReserved(void);
Class *GetClass(void);
extern IPTR ClassDispatcher(void);
Class *InitClass(struct ClassBase *cb);
BOOL InitResources(struct ClassBase *cb);
void FreeResources(struct ClassBase *cb);


static const struct EmulLibEntry ClassDispatcher_gate =
{
	TRAP_LIB,
	0,
	(void(*)(void))ClassDispatcher
};


LONG dummy_function(void)
{
	return -1;
}

__attribute__ ((section(".text"))) const char VTag[] = VERSTAG;


Class* InitClass(struct ClassBase *cb)
{
	Class *cl = NULL;

	if((cl = MakeClass(LibName, ROOTCLASS, NULL, sizeof(struct ObjData), 0L)))
	{
		cl->cl_Dispatcher.h_Entry = (HOOKFUNC)&ClassDispatcher_gate;
		cl->cl_UserData = (ULONG)cb;
	}

	cb->BClass = cl;
	return cl;
}


BOOL InitResources(struct ClassBase *cb)
{
	if(!(DOSBase = OpenLibrary("dos.library", 50))) return FALSE;
	if(!(IntuitionBase = OpenLibrary("intuition.library", 50))) return FALSE;
	if(!(UtilityBase = OpenLibrary("utility.library", 50))) return FALSE;
	if(!(MUIMasterBase = OpenLibrary("muimaster.library", 20))) return FALSE;
	if(!(MultimediaBase = OpenLibrary("multimedia/multimedia.class", 53))) return FALSE;
	if(!(LocaleBase = OpenLibrary("locale.library", 51))) return FALSE;
	Locale_Open(CLASSNAME".catalog", 1, 0);
	if(!(InitClass(cb))) return FALSE;
	return TRUE;
}


void FreeResources(UNUSED struct ClassBase *cb)
{
	Locale_Close();
	if(LocaleBase) CloseLibrary(LocaleBase);
	if(MultimediaBase) CloseLibrary(MultimediaBase);
	if(MUIMasterBase) CloseLibrary(MUIMasterBase);
	if(UtilityBase) CloseLibrary(UtilityBase);
	if(IntuitionBase) CloseLibrary(IntuitionBase);
	if(DOSBase) CloseLibrary(DOSBase);
}


const struct TagItem RTags[] =
{
	{QUERYINFOATTR_NAME, (IPTR)LibName},
	{QUERYINFOATTR_IDSTRING, (IPTR)&VTag[1]},
	{QUERYINFOATTR_DESCRIPTION, (IPTR)DESC},
	{QUERYINFOATTR_COPYRIGHT, (IPTR)COPYRIGHT},
	{QUERYINFOATTR_AUTHOR, (IPTR)AUTHOR},
	{QUERYINFOATTR_DATE, (IPTR)DATE},
	{QUERYINFOATTR_VERSION, VERSION},
	{QUERYINFOATTR_REVISION, REVISION},
	{QUERYINFOATTR_RELEASETAG, (ULONG)RELEASE_TAG},
	{QUERYINFOATTR_SUBTYPE, QUERYSUBTYPE_LIBRARY},
	{QUERYINFOATTR_CLASS, QUERYCLASS_MULTIMEDIA},
	{QUERYINFOATTR_SUBCLASS, QUERYSUBCLASS_MULTIMEDIA_DECODER},
	{TAG_END, 0}
};

struct Resident ROMTag =
{
	RTC_MATCHWORD,
	&ROMTag,
	&ROMTag + 1,
	RTF_EXTENDED | RTF_PPC,
	VERSION,
	NT_LIBRARY,
	0,
	(char*)LibName,
	VSTRING,
	(APTR)LibInit,
	REVISION,
	(struct TagItem*)RTags
};

APTR JumpTable[] =
{
	(APTR)FUNCARRAY_32BIT_NATIVE,
	(APTR)LibOpen,
	(APTR)LibClose,
	(APTR)LibExpunge,
	(APTR)LibReserved,
	(APTR)GetClass,
	(APTR)0xFFFFFFFF
};


struct ClassBase* lib_init(struct ClassBase *cb, APTR seglist, UNUSED struct Library *sysbase)
{
	InitSemaphore(&cb->BaseLock);
	cb->Seglist = seglist;

	return cb;
}

struct TagItem LibTags[] =
{
	{LIBTAG_FUNCTIONINIT, (IPTR)JumpTable},
	{LIBTAG_LIBRARYINIT,  (IPTR)lib_init},
	{LIBTAG_MACHINE,      MACHINE_PPC},
	{LIBTAG_BASESIZE,     sizeof(struct ClassBase)},
	{LIBTAG_SEGLIST,      0},
	{LIBTAG_TYPE,         NT_LIBRARY},
	{LIBTAG_NAME,         0},
	{LIBTAG_IDSTRING,     0},
	{LIBTAG_FLAGS,        LIBF_CHANGED | LIBF_SUMUSED},
	{LIBTAG_VERSION,      VERSION},
	{LIBTAG_REVISION,     REVISION},
	{LIBTAG_PUBLIC,       TRUE}
};

struct Library* LibInit(UNUSED struct Library *unused, APTR seglist, struct Library *sysbase)
{
	SysBase = sysbase;

	LibTags[4].ti_Data = (IPTR)seglist;
	LibTags[6].ti_Data = (IPTR)ROMTag.rt_Name;
	LibTags[7].ti_Data = (IPTR)ROMTag.rt_IdString;

	return (NewCreateLibrary(LibTags));
}


struct Library* LibOpen(void)
{
	struct ClassBase *cb = (struct ClassBase*)REG_A6;
	struct Library *lib = (struct Library*)cb;

	ObtainSemaphore(&cb->BaseLock);

	if(!cb->InitFlag)
	{
		if(InitResources(cb))
		{
			cb->InitFlag = TRUE;
		}
		else
		{
			FreeResources(cb);
			lib = NULL;
		}
	}

	if(lib)
	{
		cb->LibNode.lib_Flags &= ~LIBF_DELEXP;
		cb->LibNode.lib_OpenCnt++;
	}

	ReleaseSemaphore(&cb->BaseLock);

	return lib;
}


ULONG LibClose(void)
{
	struct ClassBase *cb = (struct ClassBase*)REG_A6;
	ULONG ret = 0;

	ObtainSemaphore(&cb->BaseLock);

	if(--cb->LibNode.lib_OpenCnt == 0)
	{
		if(cb->LibNode.lib_Flags & LIBF_DELEXP)
			ret = (ULONG)lib_expunge(cb);
	}

	if(ret == 0)
		ReleaseSemaphore(&cb->BaseLock);

	return ret;
}


APTR LibExpunge(void)
{
	struct ClassBase *cb = (struct ClassBase*)REG_A6;

	return(lib_expunge(cb));
}


APTR lib_expunge(struct ClassBase *cb)
{
	APTR seglist = NULL;

	ObtainSemaphore(&cb->BaseLock);

	if(cb->LibNode.lib_OpenCnt == 0)
	{
		if(!cb->BClass || FreeClass(cb->BClass))
		{
			cb->BClass = NULL;
			Forbid();
			Remove((struct Node*)cb);
			Permit();
			FreeResources(cb);
			seglist = cb->Seglist;
			FreeMem((UBYTE*)cb - cb->LibNode.lib_NegSize, cb->LibNode.lib_NegSize + cb->LibNode.lib_PosSize);
			cb = NULL;    /* freed memory, no more valid */
		}
		if(cb && cb->BClass)
			AddClass(cb->BClass);    /* class still has objects, can't expunge yet */
	}
	else
		cb->LibNode.lib_Flags |= LIBF_DELEXP;

	if(cb) ReleaseSemaphore(&cb->BaseLock);
	return seglist;
}


ULONG LibReserved(void)
{
	return 0;
}


Class *GetClass(void)
{
	struct ClassBase *cb = (struct ClassBase*)REG_A6;

	return cb->BClass;
}

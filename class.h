/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#ifndef __CLASS_H__
#define __CLASS_H__

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <kwakwa_api/protocol.h>
#include "globaldefines.h"
#include "iksemel/iksemel.h"

#define STATE_NOT_CONNECTED 0
#define STATE_CONNECTING 1
#define STATE_AFTER_HEADER 2
#define STATE_CONNECTED 3

struct VCardListNode
{
	struct MinNode Node;
	STRPTR Id;
	Object *Obj;
	ULONG Method;
};

struct ObjData
{
	Object *App;
	Object *PrefsPanel;
	Object *DebugLog;
	struct TagItem GuiTagList[5];
	struct MinList EventsList;
	struct MinList VCardList;
	iksparser *StreamParser;
	iksid *Id;
	ikstack *IksStack;
	ULONG State;
	BOOL WantWrite;
	BOOL Authorized;
	enum ikshowtype StatusOnConnect;
	STRPTR DescriptionOnConnect;
};

/* don't touch enything below this comment, unless you really know what you are doing... */

struct ClassBase
{
	struct Library          LibNode;
	Class                  *BClass;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;
};

#endif /* __CLASS_H__ */

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
#define STATE_CHECK_FEATURES 2
#define STATE_START_TLS 3
#define STATE_SSL_HANDSHAKE 4
#define STATE_SEND_AUTHORIZATION 5
#define STATE_AUTHORIZATION_CONFIRMATION 6
#define STATE_GET_AUTHORIZED_FEATURES 7
#define STATE_BIND_STREAM 8
#define STATE_CHECK_STREAM_BIND 9

#define STATE_CONNECTED 100

#define STATE_CHANGE(NEW_STATE, READ, WRITE) do { d->State = NEW_STATE; d->WantRead = READ, d->WantWrite = WRITE; }while(0)

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
	struct TagItem GuiTagList[6];
	struct MinList EventsList;
	struct MinList VCardList;
	iksparser *StreamParser;
	ikstack *IksStack;

	ULONG State;
	BOOL WantWrite, WantRead;
	ULONG ServerFeatures;

	BOOL Authorized;

	enum ikshowtype StatusOnConnect;
	STRPTR DescriptionOnConnect;
	iksid *Id;
	STRPTR Password;
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

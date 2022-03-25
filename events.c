/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <proto/exec.h>
#include <kwakwa_api/protocol.h>
#include <libvstring.h>
#include <proto/locale.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include "globaldefines.h"
#include "iksemel/iksemel.h"
#include "events.h"

extern struct Library *SysBase, *LocaleBase, *DOSBase;

static inline ULONG ActTimeStamp(VOID)
{
	ULONG secs = 2 * 366 * 24 * 3600 + 6 * 365 * 24 * 3600;
	struct Locale *l;

	if(LocaleBase && (l = OpenLocale(NULL)))
	{
		struct DateStamp ds;

		DateStamp(&ds);

		secs +=  ds.ds_Days * (24 * 60 * 60);
		secs += (ds.ds_Minute * 60);
		secs += (ds.ds_Tick / TICKS_PER_SECOND);

		secs += 60 * l->loc_GMTOffset;

		CloseLocale(l);
	}

	return secs;
}

static inline ULONG StatusTranslateToKwaKwa(enum ikshowtype s)
{
	switch(s)
	{
		case IKS_SHOW_AVAILABLE:
			return KWA_STATUS_AVAIL;
		case IKS_SHOW_AWAY:
			return KWA_STATUS_BUSY;
		case IKS_SHOW_CHAT:
			return KWA_STATUS_FFC;
		case IKS_SHOW_DND:
			return KWA_STATUS_DND;
		case IKS_SHOW_XA:
			return KWA_STATUS_INVISIBLE;
		case IKS_SHOW_UNAVAILABLE:
		default:
			return KWA_STATUS_NOT_AVAIL;
	}
}

struct KwaEvent *AddEvent(struct MinList *list, ULONG event_type)
{
	struct KwaEvent *event;

	if((event = AllocKwaEvent()))
	{
		event->ke_ModuleID = MODULE_ID;
		event->ke_Type = event_type;

		AddTail((struct List*)list, (struct Node*)event);
	}

	return event;
}

VOID AddErrorEvent(struct MinList *list, ULONG errno, STRPTR txt)
{
	struct KwaEvent *event;

	if((event = AddEvent(list, KE_TYPE_MODULE_MESSAGE)))
	{
		event->ke_ModuleMessage.ke_Errno = errno;
		event->ke_ModuleMessage.ke_MsgTxt = StrNew(txt);
	}

}

VOID AddEventStatusChange(struct MinList *list, ikspak *pac, ikstack *stack)
{
	struct KwaEvent *event;

	if((event = AddEvent(list, KE_TYPE_STATUS_CHANGE)))
	{
		STRPTR desc = iks_find_cdata(pac->x, "status");

		if(desc)
			event->ke_StatusChange.ke_Description = StrNew(iks_unescape(stack, desc, StrLen(desc)));

		event->ke_StatusChange.ke_NewStatus = StatusTranslateToKwaKwa(pac->show);
	}
}

VOID AddEventListChange(struct MinList *list, ikspak *pac, ikstack *stack)
{
	struct KwaEvent *event;

	if((event = AddEvent(list, KE_TYPE_LIST_CHANGE)))
	{
		if((event->ke_ListChange.ke_ContactID = StrNew(pac->from->partial)))
		{
			STRPTR desc = iks_find_cdata(pac->x, "status");

			if(desc)
				event->ke_ListChange.ke_Description = StrNew(iks_unescape(stack, desc, StrLen(desc)));

			event->ke_ListChange.ke_NewStatus = StatusTranslateToKwaKwa(pac->show);
			return;
		}
		Remove((struct Node*)event);
		FreeKwaEvent(event);
	}
}

VOID AddEventNewMsg(struct MinList *list, ikspak *pac)
{
	struct KwaEvent *event;

	if((event = AddEvent(list, KE_TYPE_NEW_MESSAGE)))
	{
		if((event->ke_NewMessage.ke_ContactID = StrNew(pac->from->partial)))
		{
			if((event->ke_NewMessage.ke_Txt = StrNew(iks_find_cdata(pac->x, "body"))))
			{
				event->ke_NewMessage.ke_TimeStamp = ActTimeStamp();
				event->ke_NewMessage.ke_Flags = MSG_FLAG_NORMAL;
				return;
			}
		}
		Remove((struct Node*)event);
		FreeKwaEvent(event);
	}
}

VOID AddEventNewInvite(struct MinList *list, ikspak *pac)
{
	struct KwaEvent *event;

	if((event = AddEvent(list, KE_TYPE_NEW_INVITE)))
	{
		if((event->ke_NewInvite.ke_ContactID = StrNew(pac->from->partial)))
		{
			event->ke_NewInvite.ke_TimeStamp = ActTimeStamp();
			return;
		}
		Remove((struct Node*)event);
		FreeKwaEvent(event);
	}
}

VOID AddEventImportList(struct MinList *list, ikspak *pac)
{
	struct KwaEvent *event;
	iks *x = iks_child(pac->query);
	ULONG no = 0;
	ENTER();

	while(x)
	{
		if(iks_strcmp(iks_name(x), "item") == 0)
			no++;
		x = iks_next(x);
	}

	if((event = AddEvent(list, KE_TYPE_IMPORT_LIST)))
	{
		event->ke_ImportList.ke_ContactsNo = no;
		if((event->ke_ImportList.ke_Contacts = AllocMem(sizeof(struct ContactEntry) * no, MEMF_ANY | MEMF_CLEAR)))
		{
			x = iks_child(pac->query);
			no = 0;

			while(x)
			{
				if(iks_strcmp(iks_name(x), "item") == 0)
				{
					event->ke_ImportList.ke_Contacts[no].entryid = StrNew(iks_find_attrib(x, "jid"));
					event->ke_ImportList.ke_Contacts[no].pluginid = MODULE_ID;
					no++;
				}
				x = iks_next(x);
			}

			return;
		}
		Remove((struct Node*)event);
		FreeKwaEvent(event);
	}

	LEAVE();
}

VOID AddNewAvatarEvent(struct MinList *list, STRPTR jid, struct Picture *pic)
{
	struct KwaEvent *event;

	if((event = AddEvent(list, KE_TYPE_NEW_AVATAR)))
	{
		if((event->ke_NewAvatar.ke_ContactID = StrNew(jid)))
		{
			event->ke_NewAvatar.ke_Picture = pic;
			return;
		}
		Remove((struct Node*)event);
		FreeKwaEvent(event);
	}
}


/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <clib/alib_protos.h>
#include <kwakwa_api/protocol.h>
#include <libvstring.h>
#include "iksemel/iksemel.h"
#include "class.h"
#include "gui.h"
#include "globaldefines.h"
#include "events.h"
#include "ikshooks.h"

extern struct Library *SysBase, *DOSBase, *IntuitionBase, *UtilityBase;
struct Library *SocketBase, *CyaSSLBase;

static inline enum ikshowtype StatusTranslateToXMPP(ULONG ks)
{
	if(KWA_S_AVAIL(ks))
		return IKS_SHOW_AVAILABLE;
	if(KWA_S_BUSY(ks))
		return IKS_SHOW_AWAY;
	if(KWA_S_DND(ks))
		return IKS_SHOW_DND;
	if(KWA_S_FFC(ks))
		return IKS_SHOW_CHAT;
	if(KWA_S_INVISIBLE(ks))
		return IKS_SHOW_XA;

	return IKS_SHOW_UNAVAILABLE;
}

static IPTR mNew(Class *cl, Object *obj, struct opSet *msg)
{
	ENTER();

	if((obj = (Object*)DoSuperMethodA(cl, obj, (Msg)msg)))
	{
		struct ObjData *d = INST_DATA(cl, obj);

		if((d->App = (Object*)GetTagData(KWAA_AppObject, (IPTR)NULL, msg->ops_AttrList)))
		{
			NewList((struct List*)&d->EventsList);
			NewList((struct List*)&d->VCardList);

			if((SocketBase = OpenLibrary("bsdsocket.library", 0)))
			{
				if((CyaSSLBase = OpenLibrary("cyassl.library", 0)))
				{
					if((d->PrefsPanel = CreatePrefsPage()))
					{
						if((d->IksStack = iks_stack_new(0, 1024)))
						{
							BPTR avatars_dir;
							LONG amake_res;

							if(!(avatars_dir = Lock(CACHE_AVATARS_DIR, ACCESS_READ)))
								amake_res = MakeDirAll(CACHE_AVATARS_DIR);

							if(avatars_dir || amake_res)
							{
								d->State = STATE_NOT_CONNECTED;

								d->GuiTagList[0].ti_Tag = KWAG_PrefsPage;
								d->GuiTagList[0].ti_Data = (IPTR)d->PrefsPanel;

								d->GuiTagList[1].ti_Tag = KWAG_PrefsEntry;
								d->GuiTagList[1].ti_Data = (IPTR)PROTOCOL_NAME;

								d->GuiTagList[2].ti_Tag = TAG_END;
								d->GuiTagList[2].ti_Data = (IPTR)NULL;

								LEAVE();
								return (IPTR)obj;
							}
						}
					}
				}
			}
		}
	}

	LEAVE();
	CoerceMethod(cl, obj, OM_DISPOSE);
	return (IPTR)NULL;
}


static IPTR mDispose(Class *cl, Object *obj, Msg msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	struct VCardListNode *n;
	ENTER();

	DoMethod(obj, KWAM_FreeEvents, (IPTR)&d->EventsList);

	while((n = (struct VCardListNode*)RemHead((struct List*)&d->VCardList)))
	{
		if((n->Id))
			StrFree(n->Id);

		FreeMem(n, sizeof(struct VCardListNode));
	}

	if(d->StreamParser)
	{
		iks_disconnect(d->StreamParser);
		iks_parser_delete(d->StreamParser);
	}

	if(d->IksStack)
		iks_stack_delete(d->IksStack);

	if(d->DescriptionOnConnect)
	{
		StrFree(d->DescriptionOnConnect);
		d->DescriptionOnConnect = NULL; /* memory freed, ptr no longer valid */
	}

	if(CyaSSLBase)
		CloseLibrary(CyaSSLBase);

	if(SocketBase)
		CloseLibrary(SocketBase);

	LEAVE();
	return DoSuperMethodA(cl, obj, msg);
}


static IPTR mGet(Class *cl, Object *obj, struct opGet *msg)
{
	struct ObjData *d = (struct ObjData*)INST_DATA(cl, obj);

	switch (msg->opg_AttrID)
	{
		case KWAA_Socket:
			if(d->State == STATE_NOT_CONNECTED)
				*msg->opg_Storage = (ULONG)-1;
			else
				*msg->opg_Storage = (ULONG)iks_fd(d->StreamParser);
		return TRUE;

		case KWAA_GuiTagList:
			*msg->opg_Storage = (ULONG)d->GuiTagList;
		return TRUE;

		case KWAA_ProtocolName:
			*msg->opg_Storage = (ULONG)PROTOCOL_NAME;
		return TRUE;

		case KWAA_ModuleID:
			*msg->opg_Storage = (ULONG)MODULE_ID;
		return TRUE;

		case KWAA_WantRead:
			*msg->opg_Storage = (ULONG)TRUE; /* we always want to read */
		return TRUE;

		case KWAA_WantWrite:
			*msg->opg_Storage = (ULONG)d->WantWrite;
		return TRUE;

		case KWAA_UserID:
			*msg->opg_Storage = (ULONG)xget(findobj(USD_PREFS_BASIC_ID_STRING, d->PrefsPanel), MUIA_String_Contents);
		return TRUE;

		default:
			return DoSuperMethodA(cl, obj, (Msg)msg);
	}
}

static IPTR mConnect(Class *cl, Object *obj, struct KWAP_Connect *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = FALSE;
	STRPTR usr = (STRPTR)xget(findobj(USD_PREFS_BASIC_ID_STRING, d->PrefsPanel), MUIA_String_Contents);
	ENTER();

	if(usr)
	{
		if((d->Id = iks_id_new(d->IksStack, usr)))
		{
			if((d->StreamParser = iks_stream_new(IKS_NS_CLIENT, d, StreamHook)))
			{
				LONG ret;
				LONG port = -1;
				STRPTR server = NULL;

#ifdef __DEBUG__
				iks_set_log_hook(d->StreamParser, DebugHook);
#endif /* __DEBUG */

				if(!d->Id->resource)
					d->Id->resource = "KwaKwa";

				if(xget(findobj(USD_PREFS_CONNECTION_ADVANCED, d->PrefsPanel), MUIA_Selected) == (ULONG)TRUE)
				{
					Object *port_str = findobj(USD_PREFS_CONNECTION_PORT, d->PrefsPanel);

					if(StrLen((STRPTR)xget(port_str, MUIA_String_Contents)))
						port = xget(port_str, MUIA_String_Integer);

					server = (STRPTR)xget(findobj(USD_PREFS_CONNECTION_SERVER, d->PrefsPanel), MUIA_String_Contents);
					if(StrLen(server) == 0)
						server = NULL;
				}

				switch((ret = iks_connect_async(d->StreamParser, d->Id->server, port == -1 ? IKS_JABBER_PORT : port, NULL, NULL, server)))
				{
					case IKS_OK:
						d->State = STATE_CONNECTING;
						d->WantWrite = TRUE;
						d->StatusOnConnect = StatusTranslateToXMPP(msg->Status);
						d->DescriptionOnConnect = StrNew(msg->Description);
						result = TRUE;
					break;

					case IKS_NET_NODNS:
					case IKS_NET_NOCONN:
					case IKS_NET_NOSOCK:
						AddErrorEvent(&d->EventsList, ERRNO_CONNECTION_FAILED, NULL);

					default:
						tprintf("iks_connect_async() error %ld!\n", ret);
				}
			}
			else
				AddErrorEvent(&d->EventsList, ERRNO_OUT_OF_MEMORY, NULL);
		}
		else
			AddErrorEvent(&d->EventsList, ERRNO_LOGIN_FAILED, NULL);
	}
	else
		AddErrorEvent(&d->EventsList, ERRNO_LOGIN_FAILED, NULL);


	LEAVE();
	return (IPTR)result;
}

static IPTR mDisconnect(Class *cl, Object *obj, struct KWAP_Disconnect *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	ENTER();

	if(d->State != STATE_NOT_CONNECTED && d->StreamParser)
	{
		iks_disconnect(d->StreamParser);
		iks_parser_delete(d->StreamParser);
		d->StreamParser = NULL; /* memory freed, ptr no longer valid */
	}

	d->Authorized = FALSE;
	d->WantWrite = FALSE;
	d->State = STATE_NOT_CONNECTED;

	if(d->DescriptionOnConnect)
	{
		StrFree(d->DescriptionOnConnect);
		d->DescriptionOnConnect = NULL; /* memory freed, ptr no longer valid */
	}

	AddEvent(&d->EventsList, KE_TYPE_DISCONNECT);

	LEAVE();
	return (IPTR)TRUE;
}

static IPTR mWatchEvents(Class *cl, Object *obj, struct KWAP_WatchEvents *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);

	if(msg->CanWrite)
	{
		switch(d->State)
		{
			case STATE_CONNECTING:
				if(iks_send_header(d->StreamParser, d->Id->server) == IKS_OK)
					d->State = STATE_AFTER_HEADER;
				d->WantWrite = FALSE;
			break;
		}
	}

	if(msg->CanRead)
		if(iks_recv(d->StreamParser, 0) != IKS_OK)
			AddErrorEvent(&d->EventsList, ERRNO_CONNECTION_FAILED, "Recv failed");

	return (IPTR)&d->EventsList;
}

static IPTR mNotifyList(Class *cl, Object *obj, struct KWAP_NotifyList *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	iks *status_on_connect;

	if((status_on_connect = iks_make_pres(d->StatusOnConnect, d->DescriptionOnConnect)))
	{
		iks_send(d->StreamParser, status_on_connect);
		iks_delete(status_on_connect);
	}
	return (IPTR)0;
}

static IPTR mFreeEvent(Class *cl, Object *obj, struct KWAP_FreeEvents *msg)
{
	struct KwaEvent *e;

	while((e = (struct KwaEvent*)RemHead((struct List*)msg->Events)))
	{
		switch(e->ke_Type)
		{
			case KE_TYPE_MODULE_MESSAGE:
				StrFree(e->ke_ModuleMessage.ke_MsgTxt);
			break;

			case KE_TYPE_STATUS_CHANGE:
				StrFree(e->ke_StatusChange.ke_Description);
			break;

			case KE_TYPE_LIST_CHANGE:
				StrFree(e->ke_NewMessage.ke_ContactID);
				StrFree(e->ke_ListChange.ke_Description);
			break;

			case KE_TYPE_NEW_INVITE:
				StrFree(e->ke_NewInvite.ke_ContactID);
			break;

			case KE_TYPE_IMPORT_LIST:
			{
				ULONG i;

				for(i = 0; i < e->ke_ImportList.ke_ContactsNo; i++)
				{
					StrFree(e->ke_ImportList.ke_Contacts[i].entryid);
				}
			}
			break;
		}

		FreeKwaEvent(e);
	}

	return (IPTR)0;
}


static IPTR mChangeStatus(Class *cl, Object *obj, struct KWAP_ChangeStatus *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);

	if(d->StreamParser)
	{
		iks *x;

		if((x = iks_make_pres(StatusTranslateToXMPP(msg->Status), msg->Description)))
		{
			iks_send(d->StreamParser, x);
			iks_delete(x);
		}
	}

	return (IPTR)0;
}


static IPTR mSendMessage(Class *cl, Object *obj, struct KWAP_SendMessage *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = FALSE;

	if(d->StreamParser)
	{
		iks *x;

		if((x = iks_make_msg(IKS_TYPE_CHAT, msg->ContactID, msg->Txt)))
		{
			result = iks_send(d->StreamParser, x) == IKS_OK;
			iks_delete(x);
		}
	}

	return (IPTR)result;
}

static IPTR mAddNotify(Class *cl, Object *obj, struct KWAP_AddNotify *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = FALSE;

	if(d->StreamParser)
	{
		iks *x;

		if((x = iks_make_s10n(IKS_TYPE_SUBSCRIBE, msg->Entry->nle_EntryID, NULL)))
		{
			result = iks_send(d->StreamParser, x) == IKS_OK;
			iks_delete(x);
		}
	}

	return (IPTR)result;
}

static IPTR mRemoveNotify(Class *cl, Object *obj, struct KWAP_RemoveNotify *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = FALSE;

	if(d->StreamParser)
	{
		iks *x;

		if((x = iks_make_s10n(IKS_TYPE_UNSUBSCRIBE, msg->Entry->nle_EntryID, NULL)))
		{
			result = iks_send(d->StreamParser, x) == IKS_OK;
			iks_delete(x);
		}
	}

	return (IPTR)result;
}

static IPTR mPing(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);
	static LONG minute = 60;

	if(d->StreamParser)
	{
		if(!--minute)
		{
			if(iks_send_raw(d->StreamParser, " ") != IKS_OK)
			{
				DoMethod(obj, KWAM_Disconnect, (IPTR)NULL);
				AddErrorEvent(&d->EventsList, ERRNO_CONNECTION_FAILED, NULL);
			}
			minute = 60;
		}
	}

	return (IPTR)0;
}

static IPTR mAnswerInvite(Class *cl, Object *obj, struct KWAP_AnswerInvite *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = FALSE;
	iks *x;

	if((x = iks_make_s10n(msg->Answer ? IKS_TYPE_SUBSCRIBED : IKS_TYPE_UNSUBSCRIBED, msg->ContactID, NULL)))
	{
		if(iks_send(d->StreamParser, x) == IKS_OK)
		{
			if(msg->Answer)
			{
				iks *xa;

				if((xa = iks_make_s10n(IKS_TYPE_SUBSCRIBE, msg->ContactID, NULL)))
				{
					result = iks_send(d->StreamParser, xa) == IKS_OK;
					iks_delete(xa);
				}
			}
			else
				result = TRUE;
		}
		iks_delete(x);
	}

	return (IPTR)result;
}

static IPTR mImportList(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);
	iks *x;
	BOOL result = FALSE;

	if((x = iks_make_iq(IKS_TYPE_GET, IKS_NS_ROSTER)))
	{
		iks_insert_attrib(x, "id", "roster");
		result = iks_send(d->StreamParser, x) == IKS_OK;
		iks_delete(x);
	}

	return (IPTR)result;
}

static IPTR mFetchContactInfo(Class *cl, Object *obj, struct KWAP_FetchContactInfo *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = FALSE;
	iks *x;

	if((x = iks_new("iq")))
	{
		iks *y;

		if((y = iks_new("vCard")))
		{
			iks_insert_attrib(x, "from", d->Id->full);
			iks_insert_attrib(x, "to", msg->ContactID);
			iks_insert_attrib(x, "type", "get");
			iks_insert_attrib(x, "id", "vc2");
			iks_insert_attrib(y, "xmlns", IKS_NS_VCARD);
			iks_insert_node(x, y);

			if(iks_send(d->StreamParser, x) == IKS_OK)
			{
				if(msg->Req && msg->ReqMethod)
				{
					struct VCardListNode *vc;

					if((vc = AllocMem(sizeof(struct VCardListNode), MEMF_ANY)))
					{
						if((vc->Id = StrNew(msg->ContactID)))
						{
							vc->Method = msg->ReqMethod;
							vc->Obj = msg->Req;

							AddTail((struct List*)&d->VCardList, (struct Node*)vc);
						}
						else
							FreeMem(vc, sizeof(struct VCardListNode));
					}
				}
			}

			iks_delete(y);
		}
		iks_delete(x);
	}

	return (IPTR)result;
}

IPTR ClassDispatcher(void)
{
	Class *cl = (Class*)REG_A0;
	Object *obj = (Object*)REG_A2;
	Msg msg = (Msg)REG_A1;

	switch(msg->MethodID)
	{
		case OM_NEW:                  return mNew(cl, obj, (struct opSet*)msg);
		case OM_DISPOSE:              return mDispose(cl, obj, msg);
		case OM_GET:                  return mGet(cl, obj, (struct opGet*)msg);
		case KWAM_Connect:            return mConnect(cl, obj, (struct KWAP_Connect*)msg);
		case KWAM_Disconnect:         return mDisconnect(cl, obj, (struct KWAP_Disconnect*)msg);
		case KWAM_WatchEvents:        return mWatchEvents(cl, obj, (struct KWAP_WatchEvents*)msg);
		case KWAM_FreeEvents:         return mFreeEvent(cl, obj, (struct KWAP_FreeEvents*)msg);
		case KWAM_NotifyList:         return mNotifyList(cl, obj, (struct KWAP_NotifyList*)msg);
		case KWAM_ChangeStatus:       return mChangeStatus(cl, obj, (struct KWAP_ChangeStatus*)msg);
		case KWAM_SendMessage:        return mSendMessage(cl, obj, (struct KWAP_SendMessage*)msg);
		case KWAM_AddNotify:          return mAddNotify(cl, obj, (struct KWAP_AddNotify*)msg);
		case KWAM_RemoveNotify:       return mRemoveNotify(cl, obj, (struct KWAP_RemoveNotify*)msg);
		case KWAM_TimedMethod:        return mPing(cl, obj);
		case KWAM_AnswerInvite:       return mAnswerInvite(cl, obj, (struct KWAP_AnswerInvite*)msg);
		case KWAM_ImportList:         return mImportList(cl, obj);
		case KWAM_FetchContactInfo:   return mFetchContactInfo(cl, obj, (struct KWAP_FetchContactInfo*)msg);
		default:                      return DoSuperMethodA(cl, obj, msg);
	}
}

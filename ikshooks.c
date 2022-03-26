/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <exec/lists.h>
#include <libvstring.h>
#include <mui/PowerTerm_mcc.h>
#include "iksemel/iksemel.h"
#include "support.h"
#include "globaldefines.h"
#include "class.h"
#include "gui.h"
#include "events.h"

extern struct Library *SysBase, *DOSBase;

static VOID HandlePresence(struct ObjData *d, ikspak *pac)
{
	iks *x;
	ENTER();

	if(!iks_id_cmp(d->Id, pac->from, IKS_ID_FULL))
		AddEventStatusChange(&d->EventsList, pac, d->IksStack);

	AddEventListChange(&d->EventsList, pac, d->IksStack);

	if((x = iks_find(pac->x, "x"))) /* check for avatar */
	{
		if(iks_strcmp(iks_find_attrib(x, "xmlns"), "vcard-temp:x:update") == 0)
		{
			char *avatar;

			if((avatar = iks_find_cdata(x, "photo")))
			{
				struct Picture *pic;
				UBYTE buffer[80];

				FmtNPut(buffer, CACHE_AVATARS_DIR"%ls", sizeof(buffer), avatar);

				if(!(pic = LoadPictureFile(buffer)))
				{
					if((x = iks_new("iq")))
					{
						iks *y;

						if((y = iks_new("vCard")))
						{
							iks_insert_attrib(x, "from", d->Id->full);
							iks_insert_attrib(x, "to", pac->from->partial);
							iks_insert_attrib(x, "type", "get");
							iks_insert_attrib(x, "id", "vc2");
							iks_insert_attrib(y, "xmlns", IKS_NS_VCARD);
							iks_insert_node(x, y);
							iks_send(d->StreamParser, x);
							iks_delete(y);
						}
						iks_delete(x);
					}
				}
				else
					AddNewAvatarEvent(&d->EventsList, pac->from->partial, pic);

			}
		}
	}

	LEAVE();
}

VOID HandleIqResult(struct ObjData *d, ikspak *pac)
{
	if(iks_strcmp(pac->id, "roster") == 0) /* list import */
		AddEventImportList(&d->EventsList, pac);

	if(iks_strcmp(pac->id, "vc2") == 0) /* vCard Data, contains avatar */
	{
		iks *card;

		if((card = iks_find(pac->x, "vCard")))
		{
			struct VCardListNode *vc;
			iks *photo;

			ForeachNode(&d->VCardList, vc)
			{
				if(StrEqu(pac->from->partial, vc->Id))
				{
					struct ContactEntry *e;

					if((e = AllocVec(sizeof(struct ContactEntry), MEMF_ANY | MEMF_CLEAR)))
					{
						char *fn;

						e->entryid = StrNew(vc->Id);

						if((fn = iks_find_cdata(card, "FN")))
						{
							ULONG space = StrLen(fn);

							while(fn[space] != ' ' && space--);

							if(space != 0)
							{
								e->lastname = StrNew(&fn[space+1]);
								fn[space] = '\0';
							}
							e->firstname = StrNew(fn);
							fn[space] = ' ';
						}
						if((fn = iks_find_cdata(card, "NICKNAME")))
							e->nickname = StrNew(fn);

						if((fn = iks_find_cdata(card, "BDAY")))
							e->birthyear = StrNew(fn);

						DoMethod(d->App, MUIM_Application_PushMethod, (IPTR)vc->Obj, 2, vc->Method, (IPTR)e);
					}

					Remove((struct Node*)vc);
					FreeVec(vc->Id);
					FreeMem(vc, sizeof(struct VCardListNode));
					break;
				}
			}

			if((photo = iks_find(card, "PHOTO")))
			{
				char *binval;

				if((binval = iks_find_cdata(photo, "BINVAL")))
				{
					char *decoded;
					size_t len;

					if((decoded = iks_base64_decode(binval, &len)))
					{
						BPTR fh;
						iksha *sha;
						UBYTE buffer[80];
						struct Picture *av;
						QUAD q;

						FmtNPut(buffer, CACHE_AVATARS_DIR, sizeof(buffer));

						if((sha = iks_sha_new()))
						{
							iks_sha_hash(sha, (const unsigned char*)decoded, len, 1);
							iks_sha_print(sha, buffer + StrLen(CACHE_AVATARS_DIR));
							iks_free(sha);

							if((fh = Open(buffer, MODE_NEWFILE)))
							{
								FWrite(fh, decoded, len, 1);
								Close(fh);
							}
						}

						q = len;

						if((av = LoadPictureMemory(decoded, &q)))
							AddNewAvatarEvent(&d->EventsList, pac->from->partial, av);

						iks_free(decoded);
					}
				}
			}
		}
	}

}

VOID HandlePacket(struct ObjData *d, iks *node)
{
	ikspak *pac;

	if((pac = iks_packet(node)))
	{
		switch(pac->type)
		{
			case IKS_PAK_MESSAGE:
				if(pac->subtype != IKS_TYPE_GROUPCHAT)
					AddEventNewMsg(&d->EventsList, pac);
			break;

			case IKS_PAK_PRESENCE:
				HandlePresence(d, pac);
			break;

			case IKS_PAK_IQ:
				if(pac->subtype == IKS_TYPE_RESULT)
					HandleIqResult(d, pac);
			break;

			case IKS_PAK_S10N:
				if(pac->subtype == IKS_TYPE_SUBSCRIBE)
					AddEventNewInvite(&d->EventsList, pac);
			break;

			case IKS_PAK_NONE:
				tprintf("IKS_PAK_NONE\n");
			break;
		}
	}
}

int StreamHook(void *user_data, int type, iks *node)
{
	struct ObjData *d = (struct ObjData*)user_data;
	ENTER();

	switch(type)
	{
		case IKS_NODE_START:
			AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, "XML stream started");
		break;

		case IKS_NODE_STOP:
			AddEvent(&d->EventsList, KE_TYPE_DISCONNECT);
		break;

		case IKS_NODE_ERROR:
			AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, "XML stream error!");
		break;

		case IKS_NODE_NORMAL:
			if(d->State == STATE_CHECK_FEATURES)
			{
				if(iks_strcmp("stream:features", iks_name(node)) == 0)
				{
					d->ServerFeatures = iks_stream_features(node);

					if(d->Authorized == FALSE)
					{
						if(!iks_is_secure(d->StreamParser) && (d->ServerFeatures & IKS_STREAM_STARTTLS))
							STATE_CHANGE(STATE_START_TLS, FALSE, TRUE);
						else
							STATE_CHANGE(STATE_SEND_AUTHORIZATION, FALSE, TRUE);
					}
					else
					{
						if(d->ServerFeatures & IKS_STREAM_BIND)
							STATE_CHANGE(STATE_BIND_STREAM, FALSE, TRUE);
						else
							STATE_CHANGE(STATE_CONNECTED, TRUE, FALSE);
					}
				}
			}
			else if(d->State == STATE_AUTHORIZATION_CONFIRMATION)
			{
				if(iks_strcmp("success", iks_name(node)) == 0)
				{
					d->Authorized = TRUE;
					STATE_CHANGE(STATE_GET_AUTHORIZED_FEATURES, FALSE, TRUE);
				}
				else
					AddErrorEvent(&d->EventsList, ERRNO_LOGIN_FAILED, iks_name(node));
			}
			if(d->State == STATE_CHECK_STREAM_BIND)
			{
				ikspak *pac = iks_packet(node);
				if(pac && pac->type == IKS_PAK_IQ && pac->subtype == IKS_TYPE_RESULT && iks_strcmp(pac->id, "session-bind") == 0)
				{
					const char *jid = iks_find_cdata(iks_child(pac->x), "jid");
					if(jid)
						d->Id = iks_id_new(d->IksStack, jid);
					STATE_CHANGE(STATE_CONNECTED, TRUE, FALSE);
					AddEvent(&d->EventsList, KE_TYPE_CONNECT);
				}
			}
			else if(d->State == STATE_CONNECTED)
				HandlePacket(d, node);
	}

	LEAVE();
	return IKS_OK;
}

void DebugHook(void *user_data, const char *data, size_t size, int is_incoming)
{
	struct ObjData *d = (struct ObjData*)user_data;
	const STRPTR red = "\e[0;31m";
	const STRPTR green = "\e[0;32m";
	const STRPTR end = "\n\r\e[0m";

	if(is_incoming)
		DoMethod(d->DebugLog, MUIM_PowerTerm_Write, (IPTR)green, StrLen(green));
	else
		DoMethod(d->DebugLog, MUIM_PowerTerm_Write, (IPTR)red, StrLen(red));

	DoMethod(d->DebugLog, MUIM_PowerTerm_Write, (IPTR)data, size);
	DoMethod(d->DebugLog, MUIM_PowerTerm_Write, (IPTR)end, StrLen(end));
}

/*
 * Copyright (c) 2014 - 2018 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <exec/lists.h>
#include <libvstring.h>
#include "iksemel/iksemel.h"
#include "support.h"
#include "globaldefines.h"
#include "class.h"
#include "gui.h"
#include "events.h"

extern struct Library *SysBase, *DOSBase;

VOID HandleFeatures(struct ObjData *d, int features, iks *node)
{
	if(features)
	{
		if(d->Authorized)
		{
			iks *t;

			if(features & IKS_STREAM_BIND)
			{
				if((t = iks_make_resource_bind(d->Id)))
				{
					iks_send(d->StreamParser, t);
					iks_delete(t);
				}
			}
			if(features & IKS_STREAM_SESSION)
			{
				if((t = iks_make_session()))
				{
					iks_send(d->StreamParser, t);
					iks_delete(t);
					d->State = STATE_CONNECTED;
					AddEvent(&d->EventsList, KE_TYPE_CONNECT);
				}
			}
		}
		else
		{
			STRPTR pass = (STRPTR)xget(findobj(USD_PREFS_BASIC_PASS_STRING, d->PrefsPanel), MUIA_String_Contents);

			if(pass)
			{
				if(features & IKS_STREAM_SASL_MD5)
					iks_start_sasl(d->StreamParser, IKS_SASL_DIGEST_MD5, d->Id->user, pass);
				else if(features & IKS_STREAM_SASL_PLAIN)
					iks_start_sasl(d->StreamParser, IKS_SASL_PLAIN, d->Id->user, pass);
			}
		}
	}
	else if(iks_strcmp("failure", iks_name(node)) == 0)
	{
		d->Authorized = FALSE;
		AddErrorEvent(&d->EventsList, ERRNO_LOGIN_FAILED, iks_name(node));
	}
	else if(iks_strcmp("success", iks_name(node)) == 0)
	{
		d->Authorized = TRUE;
		iks_send_header(d->StreamParser, d->Id->server);
	}
}

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

VOID HandleIq(struct ObjData *d, ikspak *pac)
{
	if(pac->subtype == IKS_TYPE_RESULT)
	{
		tprintf("pac->id == %ls\n", pac->id);

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
				HandleIq(d, pac);
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
	int features = 0;

	switch(type)
	{
		case IKS_NODE_START:
			AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, "XML stream started");
			if(!iks_is_secure(d->StreamParser))
				iks_start_tls(d->StreamParser);
		break;

		case IKS_NODE_STOP:
			AddEvent(&d->EventsList, KE_TYPE_DISCONNECT);
		break;

		case IKS_NODE_ERROR:
			AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, "XML stream error!");
		break;

		case IKS_NODE_NORMAL:
			if(iks_strcmp("stream:features", iks_name(node)) == 0)
				features = iks_stream_features(node);
			if(!iks_is_secure(d->StreamParser))
				break;
			if(d->State == STATE_AFTER_HEADER)
				HandleFeatures(d, features, node);
			if(d->State == STATE_CONNECTED)
				HandlePacket(d, node);
	}

	return IKS_OK;
}


#ifdef __DEBUG__
void DebugHook(void *user_data, const char *data, size_t size, int is_incoming)
{
	struct ObjData *d = (struct ObjData*)user_data;

	if(is_incoming)
	{
		tprintf("Recived XML:\n");
		AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, "Recived XML:");
	}
	else
	{
		tprintf("Send XML:\n");
		AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, "Send XML:");
	}
	tprintf("%ls\n", data);
	AddErrorEvent(&d->EventsList, ERRNO_ONLY_MESSAGE, (STRPTR)data);
}
#endif /* __DEBUG__ */

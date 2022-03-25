/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <kwakwa_api/protocol.h>

struct KwaEvent *AddEvent(struct MinList *list, ULONG event_type);
VOID AddErrorEvent(struct MinList *list, ULONG errno, STRPTR txt);
VOID AddEventStatusChange(struct MinList *list, ikspak *pac, ikstack *stack);
VOID AddEventListChange(struct MinList *list, ikspak *pac, ikstack *stack);
VOID AddEventNewMsg(struct MinList *list, ikspak *pac);
VOID AddEventNewInvite(struct MinList *list, ikspak *pac);
VOID AddEventImportList(struct MinList *list, ikspak *pac);
VOID AddNewAvatarEvent(struct MinList *list, STRPTR jid, struct Picture *pic);

#endif /* __EVENTS_H__ */

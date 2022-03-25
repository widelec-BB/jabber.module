/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#ifndef __GUI_H__
#define __GUI_H__

#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <clib/alib_protos.h>
#include "globaldefines.h"

#define MAKE_ID(x) (MODULE_ID | (x))

/* jabber basic prefs */
#define USD_PREFS_BASIC_ID_STRING            MAKE_ID(0x00000000)
#define USD_PREFS_BASIC_PASS_STRING          MAKE_ID(0x00000001)
/* jabber connection prefs */
#define USD_PREFS_CONNECTION_ADVANCED        MAKE_ID(0x00000002)
#define USD_PREFS_CONNECTION_SERVER          MAKE_ID(0x00000003)
#define USD_PREFS_CONNECTION_PORT            MAKE_ID(0x00000004)
/* jabber debug log window */
#define USD_DEBUGLOG_WINDOW                  MAKE_ID(0x00001000)

extern struct Library *IntuitionBase;

Object* MUI_NewObjectM(char *, ...);
Object *CreatePrefsPage(VOID);
Object *CreateDebugLogWindow(Object **, Object **);

#define findobj(id, parent) (Object*)DoMethod(parent, MUIM_FindUData, id)

static inline ULONG xget(Object *obj, ULONG att)
{
  ULONG result;

  GetAttr(att, obj, &result);
  return result;
}

#endif /* __GUI_H__ */

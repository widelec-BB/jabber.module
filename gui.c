/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#include <proto/muimaster.h>
#include <proto/alib.h>
#include <mui/PowerTerm_mcc.h>
#include "globaldefines.h"
#include "gui.h"
#include "locale.h"

#define EmptyRectangle(weight) MUI_NewObjectM(MUIC_Rectangle, MUIA_Weight, weight, TAG_END)


extern struct Library *MUIMasterBase, *IntuitionBase;

Object* MUI_NewObjectM(char *classname, ...)
{
	va_list args, args2;
	LONG argc = 0;
	ULONG tag;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, classname);

	while((tag = va_arg(args, ULONG)) != TAG_END)
	{
		va_arg(args, IPTR);
		argc++;
	}

	va_end(args);

	{
		struct TagItem tags[argc + 1];  // one for {TAG_END, 0}
		LONG i;

		va_start(args2, classname);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, ULONG);
			tags[i].ti_Data = va_arg(args2, IPTR);
		}

		tags[argc].ti_Tag = TAG_END;
		tags[argc].ti_Data = 0;

		va_end(args2);

		result = (Object*)MUI_NewObjectA(classname, tags);
	}
	return result;
}

static inline Object* StringLabel(STRPTR label, STRPTR preparse)
{
	Object *obj = MUI_NewObjectM(MUIC_Text,
		MUIA_FramePhantomHoriz, TRUE,
		MUIA_Frame, MUIV_Frame_String,
		MUIA_Text_PreParse, (ULONG)preparse,
		MUIA_Text_Contents, (ULONG)label,
		MUIA_HorizWeight, 0,
	TAG_END);

	return obj;
}

static inline Object* StringGadget(ULONG id)
{
	Object *obj = MUI_NewObjectM(MUIC_String,
		MUIA_UserData, id,
		MUIA_ObjectID, id,
		MUIA_Frame, MUIV_Frame_String,
		MUIA_Background, MUII_StringBack,
		MUIA_CycleChain, TRUE,
		MUIA_String_AdvanceOnCR, TRUE,
	TAG_END);

	return obj;
}

static inline Object* GfxButton(ULONG id, STRPTR pic, UBYTE control)
{
	Object *obj = MUI_NewObjectM(MUIC_Group,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_UserData, id,
		MUIA_ObjectID, id,
		MUIA_ControlChar, (ULONG)control,
		MUIA_Group_Child, (ULONG)MUI_NewObjectM(MUIC_Dtpic,
			MUIA_Dtpic_Name, (ULONG)pic,
		TAG_END),
	TAG_END);

	return obj;
}

static inline Object* NormalButton(STRPTR label, UBYTE control, LONG objid, ULONG weight)
{
	Object *obj = MUI_NewObjectM(MUIC_Text,
		MUIA_Text_Contents, (ULONG)label,
		MUIA_Text_PreParse, (ULONG)"\33c",
		MUIA_Frame, MUIV_Frame_Button,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Font, MUIV_Font_Button,
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_Text_HiChar, (ULONG)control,
		MUIA_ControlChar, (ULONG)control,
		MUIA_CycleChain, TRUE,
		MUIA_HorizWeight, weight,
		MUIA_UserData, objid,
	TAG_END);

	return obj;
}


Object *CreatePrefsPage(VOID)
{
	Object *result;
	Object *login_group, *pass_string, *conn_group;
	Object *adv_group;
	BOOL hack = TRUE; /* need to use variable, if not gcc will fuck us up (will use memcpy())... */
	ENTER();

	pass_string = 	MUI_NewObjectM(MUIC_String,
		MUIA_ObjectID, USD_PREFS_BASIC_PASS_STRING,
		MUIA_UserData, USD_PREFS_BASIC_PASS_STRING,
		MUIA_Frame, MUIV_Frame_String,
		MUIA_Background, MUII_StringBack,
		MUIA_CycleChain, hack,
		MUIA_String_Secret, hack,
	TAG_END);

	login_group = MUI_NewObjectM(MUIC_Group,
		MUIA_Frame, MUIV_Frame_Group,
		MUIA_FrameTitle, (ULONG)GetString(MSG_PREFS_BASIC),
		MUIA_Background, MUII_GroupBack,
		MUIA_Group_Columns, 2,
		MUIA_Group_Child, (ULONG)StringLabel(GetString(MSG_PREFS_BASIC_ID), "\33r"),
		MUIA_Group_Child, (ULONG)MUI_NewObjectM(MUIC_String,
			MUIA_ObjectID, USD_PREFS_BASIC_ID_STRING,
			MUIA_UserData, USD_PREFS_BASIC_ID_STRING,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_CycleChain, TRUE,
			MUIA_String_AdvanceOnCR, TRUE,
			MUIA_ShortHelp, (ULONG)GetString(MSG_PREFS_BASIC_ID_HELP),
		TAG_END),
		MUIA_Group_Child, (ULONG)StringLabel(GetString(MSG_PREFS_BASIC_PASS), "\33r"),
		MUIA_Group_Child, (ULONG)pass_string,
	TAG_END);

	conn_group = MUI_NewObjectM(MUIC_Group,
			MUIA_Frame, MUIV_Frame_Group,
			MUIA_FrameTitle, (ULONG)GetString(MSG_PREFS_CONNECTION),
			MUIA_Background, MUII_GroupBack,

			MUIA_Group_Child, (IPTR)MUI_NewObjectM(MUIC_Group,
				MUIA_Group_Horiz, TRUE,
				MUIA_Group_Child, (IPTR)MUI_NewObjectM(MUIC_Image,
					MUIA_ObjectID, USD_PREFS_CONNECTION_ADVANCED,
					MUIA_UserData, USD_PREFS_CONNECTION_ADVANCED,
					MUIA_Image_Spec, (IPTR)"6:15",
					MUIA_ShowSelState, FALSE,
					MUIA_InputMode, MUIV_InputMode_Toggle,
					MUIA_CycleChain, TRUE,
					MUIA_ShortHelp, (IPTR)GetString(MSG_PREFS_CONNECTION_ADVANCED_HELP),
				TAG_END),
				MUIA_Group_Child, (IPTR)StringLabel(GetString(MSG_PREFS_CONNECTION_ADVANCED), "\33r"),
				MUIA_Group_Child, (IPTR)EmptyRectangle(100),
			TAG_END),

			MUIA_Group_Child, (IPTR)(adv_group = MUI_NewObjectM(MUIC_Group,
				MUIA_Disabled, TRUE,
				MUIA_Group_Child, (IPTR)MUI_NewObjectM(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (ULONG)StringLabel(GetString(MSG_PREFS_CONNECTION_SERVER), "\33r"),
					MUIA_Group_Child, (ULONG)MUI_NewObjectM(MUIC_String,
						MUIA_ObjectID, USD_PREFS_CONNECTION_SERVER,
						MUIA_UserData, USD_PREFS_CONNECTION_SERVER,
						MUIA_Frame, MUIV_Frame_String,
						MUIA_Background, MUII_StringBack,
						MUIA_CycleChain, TRUE,
						MUIA_String_AdvanceOnCR, TRUE,
						MUIA_ShortHelp, (ULONG)GetString(MSG_PREFS_CONNECTION_SERVER_HELP),
					TAG_END),
				TAG_END),

				MUIA_Group_Child, (IPTR)MUI_NewObjectM(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (ULONG)StringLabel(GetString(MSG_PREFS_CONNECTION_PORT), "\33r"),
					MUIA_Group_Child, (ULONG)MUI_NewObjectM(MUIC_String,
						MUIA_ObjectID, USD_PREFS_CONNECTION_PORT,
						MUIA_UserData, USD_PREFS_CONNECTION_PORT,
						MUIA_Frame, MUIV_Frame_String,
						MUIA_Background, MUII_StringBack,
						MUIA_CycleChain, TRUE,
						MUIA_String_AdvanceOnCR, TRUE,
						MUIA_String_Accept, "123456789",
						MUIA_String_Contents, "5222",
						MUIA_ShortHelp, (ULONG)GetString(MSG_PREFS_CONNECTION_PORT_HELP),
					TAG_END),
				TAG_END),
			TAG_END)),
	TAG_END);

	DoMethod(findobj(USD_PREFS_CONNECTION_ADVANCED, conn_group), MUIM_Notify, MUIA_Selected, MUIV_EveryTime, (IPTR)adv_group, 3,
	 MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

	result = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Child, EmptyRectangle(100),
		MUIA_Group_Child, (ULONG)login_group,
		MUIA_Group_Child, (ULONG)conn_group,
		MUIA_Group_Child, EmptyRectangle(100),
	TAG_END);

	LEAVE();
	return result;
}

Object *CreateDebugLogWindow(Object **term, Object **menu_item)
{
	Object *window, *scroll;
	ENTER();

	*menu_item = MUI_NewObjectM(MUIC_Menuitem,
		MUIA_Menuitem_Title, (IPTR)GetString(MSG_MENU_DEBUGLOG_OPEN),
	TAG_END);

	if(!menu_item)
		return NULL;

	window = MUI_NewObjectM(MUIC_Window,
		MUIA_Background, MUII_GroupBack,
		MUIA_Window_ID, USD_DEBUGLOG_WINDOW,
		MUIA_UserData, USD_DEBUGLOG_WINDOW,
		MUIA_Window_Title, (IPTR)GetString(MSG_DEBUGLOG_WINDOW_TITLE),
		MUIA_Window_UseRightBorderScroller, TRUE,
		MUIA_Window_RootObject, MUI_NewObjectM(MUIC_Group,
			MUIA_Background, MUII_GroupBack,
			MUIA_Frame, MUIV_Frame_Group,
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
				MUIA_Group_Horiz, TRUE,
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
					MUIA_Text_Contents, (IPTR)GetString(MSG_DEBUGLOG_WINDOW_LEGEND_OUTGOING),
					MUIA_Text_PreParse, "\33P[FF0000]\33l",
				TAG_END),
				MUIA_Group_Child, EmptyRectangle(100),
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
					MUIA_Text_Contents, (IPTR)GetString(MSG_DEBUGLOG_WINDOW_LEGEND_INCOMING),
					MUIA_Text_PreParse, "\33P[00FF00]\33r",
				TAG_END),
			TAG_END),
			MUIA_Group_Child, (scroll = MUI_NewObjectM(MUIC_Scrollgroup,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_UseWinBorder, TRUE,
				MUIA_Scrollgroup_Contents, (IPTR)(*term = MUI_NewObjectM(MUIC_PowerTerm,
					MUIA_PowerTerm_UTFEnable, TRUE,
					MUIA_PowerTerm_Wrap, TRUE,
					MUIA_PowerTerm_OutEnable, FALSE,
					MUIA_PowerTerm_ResizableHistory, TRUE,
					MUIA_PowerTerm_EatAllInput, FALSE,
					MUIA_PowerTerm_Emulation, MUIV_PowerTerm_Emulation_ANSI,
				TAG_END)),
			TAG_END)),
		TAG_END),
	TAG_END);

	if(!window)
	{
		MUI_DisposeObject(*menu_item);
		return NULL;
	}

	set(*term, MUIA_PowerTerm_Scroller, (IPTR)scroll);

	DoMethod(*menu_item, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, (IPTR)window, 3,
	 MUIM_Set, MUIA_Window_Open, TRUE);
	DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, (IPTR)window, 3,
	 MUIM_Set, MUIA_Window_Open, FALSE);

	LEAVE();
	return window;
}

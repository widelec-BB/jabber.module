#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H 1

/* Locale Catalog Source File
 *
 * Automatically created by SimpleCat V3
 * Do NOT edit by hand!
 *
 * SimpleCat ©1992-2015 Guido Mersmann
 *
 */



/****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif



/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSG_PREFS_BASIC 0
#define MSG_PREFS_BASIC_ID 1
#define MSG_PREFS_BASIC_PASS 2
#define MSG_PREFS_BASIC_ID_HELP 3
#define MSG_PREFS_BASIC_PASS_HELP 4
#define MSG_PREFS_CONNECTION 5
#define MSG_PREFS_CONNECTION_ADVANCED_HELP 6
#define MSG_PREFS_CONNECTION_ADVANCED 7
#define MSG_PREFS_CONNECTION_SERVER_HELP 8
#define MSG_PREFS_CONNECTION_SERVER 9
#define MSG_PREFS_CONNECTION_PORT 10
#define MSG_PREFS_CONNECTION_PORT_HELP 11
#define MSG_MENU_DEBUGLOG_OPEN 12
#define MSG_DEBUGLOG_WINDOW_TITLE 13
#define MSG_DEBUGLOG_WINDOW_LEGEND_OUTGOING 14
#define MSG_DEBUGLOG_WINDOW_LEGEND_INCOMING 15

#define CATCOMP_LASTID 15

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSG_PREFS_BASIC_STR "User Settings"
#define MSG_PREFS_BASIC_ID_STR "Jabber ID:"
#define MSG_PREFS_BASIC_PASS_STR "Password:"
#define MSG_PREFS_BASIC_ID_HELP_STR "Your Jabber ID, format: user@server.com."
#define MSG_PREFS_BASIC_PASS_HELP_STR "Your password."
#define MSG_PREFS_CONNECTION_STR "Connection Settings"
#define MSG_PREFS_CONNECTION_ADVANCED_HELP_STR "Allows to set server addres and port number manually."
#define MSG_PREFS_CONNECTION_ADVANCED_STR "Turn on advanced connection settings"
#define MSG_PREFS_CONNECTION_SERVER_HELP_STR "Address of server you want connect to."
#define MSG_PREFS_CONNECTION_SERVER_STR "Server:"
#define MSG_PREFS_CONNECTION_PORT_STR "Port:"
#define MSG_PREFS_CONNECTION_PORT_HELP_STR "Server's port you want connect to."
#define MSG_MENU_DEBUGLOG_OPEN_STR "Debug Log..."
#define MSG_DEBUGLOG_WINDOW_TITLE_STR "KwaKwa: Jabber Debug Log"
#define MSG_DEBUGLOG_WINDOW_LEGEND_OUTGOING_STR "Outgoing Data"
#define MSG_DEBUGLOG_WINDOW_LEGEND_INCOMING_STR "Incoming Data"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSG_PREFS_BASIC,(STRPTR)MSG_PREFS_BASIC_STR},
    {MSG_PREFS_BASIC_ID,(STRPTR)MSG_PREFS_BASIC_ID_STR},
    {MSG_PREFS_BASIC_PASS,(STRPTR)MSG_PREFS_BASIC_PASS_STR},
    {MSG_PREFS_BASIC_ID_HELP,(STRPTR)MSG_PREFS_BASIC_ID_HELP_STR},
    {MSG_PREFS_BASIC_PASS_HELP,(STRPTR)MSG_PREFS_BASIC_PASS_HELP_STR},
    {MSG_PREFS_CONNECTION,(STRPTR)MSG_PREFS_CONNECTION_STR},
    {MSG_PREFS_CONNECTION_ADVANCED_HELP,(STRPTR)MSG_PREFS_CONNECTION_ADVANCED_HELP_STR},
    {MSG_PREFS_CONNECTION_ADVANCED,(STRPTR)MSG_PREFS_CONNECTION_ADVANCED_STR},
    {MSG_PREFS_CONNECTION_SERVER_HELP,(STRPTR)MSG_PREFS_CONNECTION_SERVER_HELP_STR},
    {MSG_PREFS_CONNECTION_SERVER,(STRPTR)MSG_PREFS_CONNECTION_SERVER_STR},
    {MSG_PREFS_CONNECTION_PORT,(STRPTR)MSG_PREFS_CONNECTION_PORT_STR},
    {MSG_PREFS_CONNECTION_PORT_HELP,(STRPTR)MSG_PREFS_CONNECTION_PORT_HELP_STR},
    {MSG_MENU_DEBUGLOG_OPEN,(STRPTR)MSG_MENU_DEBUGLOG_OPEN_STR},
    {MSG_DEBUGLOG_WINDOW_TITLE,(STRPTR)MSG_DEBUGLOG_WINDOW_TITLE_STR},
    {MSG_DEBUGLOG_WINDOW_LEGEND_OUTGOING,(STRPTR)MSG_DEBUGLOG_WINDOW_LEGEND_OUTGOING_STR},
    {MSG_DEBUGLOG_WINDOW_LEGEND_INCOMING,(STRPTR)MSG_DEBUGLOG_WINDOW_LEGEND_INCOMING_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x00\x00\x0E"
    MSG_PREFS_BASIC_STR "\x00"
    "\x00\x00\x00\x01\x00\x0C"
    MSG_PREFS_BASIC_ID_STR "\x00\x00"
    "\x00\x00\x00\x02\x00\x0A"
    MSG_PREFS_BASIC_PASS_STR "\x00"
    "\x00\x00\x00\x03\x00\x2A"
    MSG_PREFS_BASIC_ID_HELP_STR "\x00\x00"
    "\x00\x00\x00\x04\x00\x10"
    MSG_PREFS_BASIC_PASS_HELP_STR "\x00\x00"
    "\x00\x00\x00\x05\x00\x14"
    MSG_PREFS_CONNECTION_STR "\x00"
    "\x00\x00\x00\x06\x00\x36"
    MSG_PREFS_CONNECTION_ADVANCED_HELP_STR "\x00"
    "\x00\x00\x00\x07\x00\x26"
    MSG_PREFS_CONNECTION_ADVANCED_STR "\x00\x00"
    "\x00\x00\x00\x08\x00\x28"
    MSG_PREFS_CONNECTION_SERVER_HELP_STR "\x00\x00"
    "\x00\x00\x00\x09\x00\x08"
    MSG_PREFS_CONNECTION_SERVER_STR "\x00"
    "\x00\x00\x00\x0A\x00\x06"
    MSG_PREFS_CONNECTION_PORT_STR "\x00"
    "\x00\x00\x00\x0B\x00\x24"
    MSG_PREFS_CONNECTION_PORT_HELP_STR "\x00\x00"
    "\x00\x00\x00\x0C\x00\x0E"
    MSG_MENU_DEBUGLOG_OPEN_STR "\x00\x00"
    "\x00\x00\x00\x0D\x00\x1A"
    MSG_DEBUGLOG_WINDOW_TITLE_STR "\x00\x00"
    "\x00\x00\x00\x0E\x00\x0E"
    MSG_DEBUGLOG_WINDOW_LEGEND_OUTGOING_STR "\x00"
    "\x00\x00\x00\x0F\x00\x0E"
    MSG_DEBUGLOG_WINDOW_LEGEND_INCOMING_STR "\x00"
};

#endif /* CATCOMP_BLOCK */


/****************************************************************************/



#endif /* TRANSLATIONS_H */


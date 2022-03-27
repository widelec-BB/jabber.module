/*
 * Copyright (c) 2014 - 2022 Filip "widelec" Maryjanski, BlaBla group.
 * All rights reserved.
 * Distributed under the terms of the GNU Lesser General Public License v2.1.
 */

#ifndef __GLOBALDEFINES_H__
#define __GLOBALDEFINES_H__

#define CLASSNAME "jabber.module"
#define VERSION 1
#define REVISION 3
#define DATE __AMIGADATE__
#define VERS "jabber.module 1.3"
#define AUTHOR "Filip \"widelec\" Maryjañski"
#define DESC "Jabber Module for KwaKwa"
#define COPYRIGHT "© " "2014 - 2022" " " AUTHOR
#define VSTRING VERS DATE COPYRIGHT
#define VERSTAG "\0$VER: " VSTRING
#define RELEASE_TAG "F1"
#define PROTOCOL_NAME "Jabber"
#define MODULE_ID 0x01000000

#ifdef __DEBUG__
#include <clib/debug_protos.h>
#define tprintf(template, ...) KPrintF((CONST_STRPTR)CLASSNAME " " __FILE__ " %d: " template, __LINE__ , ##__VA_ARGS__)
#define ENTER(...) KPrintF((CONST_STRPTR)CLASSNAME " enters: %s\n", __PRETTY_FUNCTION__)
#define LEAVE(...) KPrintF((CONST_STRPTR)CLASSNAME " leaves: %s\n", __PRETTY_FUNCTION__)
#define strd(x)((STRPTR)x ? (STRPTR)x : (STRPTR)"NULL")
#else
#define tprintf(...)
#define ENTER(...)
#define LEAVE(...)
#endif /* __DEBUG__ */

#define UNUSED __attribute__((unused))

#define CACHE_DIR "PROGDIR:cache/jabber.module/"
#define CACHE_DIR_LEN 28
#define CACHE_AVATARS_DIR CACHE_DIR "avatars/"
#define CACHE_AVATARS_DIR_LEN (CACHE_DIR_LEN + 8)

#define SHA_1_STR_LEN 40

#endif /* __GLOBALDEFINES_H__ */

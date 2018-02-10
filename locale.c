
/*
** locale.c
**
** (c) 2011 by Guido Mersmann
**
** Object source created by SimpleCat.
**
** DO NOT EDIT THIS FILE UNLESS YOU KNOW WHAT YOU ARE DOING
**
** Usage is quite simple.
**
** 1. Add this file to object creation list in makefile.
**
** 2. Call Locale_Open() and Locale_Close() on startup and shut down of your code.
**    There is no need to check any result.
**
** 3. Make sure the marked include line within 'locale.h' header file uses
**    the name of your string header file. Thats name in .CS file which is set with
**    TARGET C header file command or when using the GUI in target list. By default
**    its "locale_strings.h". Make sure you use the following switches for the C
**    target to create a valid header file for this code:
**    NoCode NoArray NoBlockStatic
**
** 4. Include the 'locale.h' header everywhere you need a locale string.
**
** 5. Call GetString( id ) where ever you need a locale string. "id" is the label name
**    you address the strings. To follow the style guide you should name the files
**    using "MSG_" prefix.
**
** 6. Thats all!
**
*/

#include <exec/types.h>
#include <libraries/locale.h>

#include <proto/locale.h>
#include <proto/exec.h>

#define CATCOMP_BLOCK 1     /* enable CATCOMP_BLOCK */
#include "locale.h" /* prototypes and catcomp block */

/*************************************************************************/


static struct Locale      *locale_locale;
static struct Catalog     *locale_catalog;
extern struct Library *SysBase, *LocaleBase;
/*************************************************************************/

/* /// Locale_Open */

/*************************************************************************/

BOOL Locale_Open(STRPTR catname, ULONG version, ULONG revision)
{
	if((locale_locale = OpenLocale(NULL)))
	{
		if((locale_catalog = OpenCatalogA(locale_locale, catname, TAG_DONE)))
		{
			if(locale_catalog->cat_Version == version  && locale_catalog->cat_Revision == revision)
			{
				return(TRUE);
			}
			CloseCatalog(locale_catalog);
			locale_catalog = NULL;
		}
		CloseLocale(locale_locale);
		locale_locale = NULL;
	}
	return(FALSE);
}
/* \\\ Locale_Open */

/* /// Locale_Close */

/*************************************************************************/

void Locale_Close(void)
{
	if(locale_catalog)
	{
		CloseCatalog(locale_catalog);
		locale_catalog = NULL;
	}
	if(locale_locale)
	{
		CloseLocale(locale_locale);
		locale_locale = NULL;
	}
}
/* \\\ */

/* /// GetString */

/*************************************************************************/

STRPTR GetString(long id)
{
	LONG   *l;
	UWORD  *w;
	STRPTR  builtin;

	l = (LONG *)CatCompBlock;

	while(*l != id)
	{
		w = (UWORD *)((ULONG)l + 4);
		l = (LONG *)((ULONG)l + (ULONG) * w + 6);
	}
	builtin = (STRPTR)((ULONG)l + 6);

	if(locale_catalog)
	{
		return((APTR) GetCatalogStr(locale_catalog, id, builtin));
	}
	return(builtin);
}
/* \\\ GetString */


/* ***** BEGIN LICENSE BLOCK *****
 * Version: BSD License
 * 
 * Copyright (c) 2005, Diego Casorran <dcasorran@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 ** Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  
 ** Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */


#include "Locky.h"

STATIC CONST UBYTE __locale_c[] =
"\n"
"#include <proto/locale.h>		/*NL*/\n"
"\n"
"extern struct Catalog *catalog;\n"
"\n"
"STATIC CONST STRPTR LocalStrings[] = \n"
"{\n"
"/*tAg*/\n"
"	NULL,\n"
"};\n"
"\n"
"\n"
"STRPTR gCatStr(const unsigned int id)\n"
"{\n"
"	STRPTR lstr = LocalStrings[id];\n"
"	\n"
"	return catalog ? GetCatalogStr( catalog, id, lstr) : lstr;\n"
"}\n"
"\n";

STRPTR ** open_oldlocale( STRPTR path, long *MSG_number, BPTR *fd )
{
	char oldlocale[512], *localeh,*f,*ln;
	STATIC STRPTR localized_strings[(1024*1024)];
	long lnum = -1, sn = 0;
	
	localized_strings[0] = NULL;
	
	strcpy( oldlocale, path );
	AddPart(oldlocale, "locale.h", 512 );
	
	if( ! _exists( oldlocale ) || (_GetFSize( oldlocale) == 0 ))
		goto done;
	
	DBUG(("Loading \"%s\"...\n", oldlocale ));
	
	localeh = FileToMem( oldlocale );
	assert(localeh != NULL);
	
#if 0
	for( f = localeh ; ((ln = _strsep( &f, "\n"))) ; lnum++ )
	{
		if(!(lnum%3)) {
			char msg[512], *d=msg, *s=&ln[8];
			
			do {
				*d++ = *s++;
				
			} while(*s && *s != 0x09); *d=0;
			
			localized_strings[sn] = strdup( msg );
			assert(localized_strings[sn] != NULL);
			sn++;
		}
	}
#else
	f = localeh;
	do {
		if(*f == '#') {
			char msg[512], *d=msg;
			
			for( f += 8 ; *f && *f != 0x09 ; *d++ = *f++ ); *d=0;
			
			localized_strings[sn] = strdup( msg );
			assert(localized_strings[sn] != NULL);
			sn++;
		}
	} while(*f++);
#endif
	
done:
	(*MSG_number) = sn;
	
	(*fd) = Open( oldlocale, MODE_READWRITE );
	assert((*fd) != NULL);
	
	Seek((*fd), 0, OFFSET_END );
	
	strcpy( oldlocale, path );
	AddPart(oldlocale, "locale.c", 512 );
	
	if( ! _exists( oldlocale ))
	{
		BOOL localec_saved = _WriteFile( oldlocale, (STRPTR)__locale_c );
		assert(localec_saved == TRUE);
	}
	
	return (STRPTR **) localized_strings;
}


/* ***** BEGIN LICENSE BLOCK *****
 * Version: BSD License
 * 
 * Copyright (c) 2006, Diego Casorran <dcasorran@gmail.com>
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


/*:ts=4 */
#include "Locky.h"
#include "localize.h"
#include <proto/utility.h>

extern ULONG __ftm_lsize;

#define LOCALE_SEPTAG		"/*tAg*/\n"
#define LOCALE_SEPTAG_LEN	8

/**************************************************************************/

#define STRTOLOCKY_LENGTH	262144

#define IsSpace(ch)	\
	((ch) == 0x20 || (ch) == 0x09 || (ch) == '\r' || (ch) == '\n')
#define IsUpper(ch)	\
	((ch) >= 'A' && (ch) <= 'Z')
#define IsLower(ch)	\
	((ch) >= 'a' && (ch) <= 'z')
#define IsDigit(ch)	\
	((ch) >= '0' && (ch) <= '9')
#define IsAlpha(ch)	\
	(IsLower(ch) || IsUpper(ch))
#define IsHex(ch)	\
	(IsDigit(ch) || ((ch) >= 'a' && (ch) <= 'f')||((ch) >= 'A' && (ch) <= 'F'))

// ALLOWEDCHARs to the MSG_#? localized string define
#define ALLOWEDCHAR(ch)	\
	(IsAlpha(ch) || IsDigit(ch))


/**************************************************************************/

STATIC BPTR ThisLocale = 0;
STATIC STRPTR ToEOFLocalePart = NULL;

INLINE VOID LoadLocaleFile( STRPTR filename )
{
	STRPTR file, f;
	long s;
	
	f = file = _FileToMem( filename );
	assert(file != NULL);
	
	while( *f && strncmp(f,LOCALE_SEPTAG,LOCALE_SEPTAG_LEN)) f++;
	
	if(!(*f)) {
		Printf("cannot found TAG (%s) on %s\n",(long) LOCALE_SEPTAG,(long)filename);
		abort();
	}
	
	ToEOFLocalePart = strdup( f );
	assert( ToEOFLocalePart != NULL );
	
	ThisLocale = Open( filename, MODE_READWRITE );
	assert( ThisLocale != 0 );
	
	Seek( ThisLocale, (long) (~(file-f))+1, OFFSET_BEGINNING );
	s = SetFileSize( ThisLocale, 0, OFFSET_CURRENT );
	assert( s != -1 );
	
//	free( file );
}

INLINE BOOL add_locale_string( STRPTR file, STRPTR string )
{
	STATIC BOOL FirstCall = TRUE;
	
	if( FirstCall == TRUE )
	{
		FirstCall = FALSE;
		
		LoadLocaleFile( file );
	}
	
	Write( ThisLocale, string, strlen(string));
	
	return TRUE;
}

DESTRUCTOR(__close_ThisLocale)
{
	if( ThisLocale )
	{
		if( ToEOFLocalePart )
		{
			Write( ThisLocale, ToEOFLocalePart, strlen( ToEOFLocalePart ));
		}
		
		Close( ThisLocale );
	}
	
	if( ToEOFLocalePart ) {
		free( ToEOFLocalePart );
		ToEOFLocalePart = NULL;
	}
}

#undef appendT
#define appendT(a,b,c)	add_locale_string(a,b)

/**************************************************************************/

#define SLASH_	\
	(*((*str)-1) != '\\' || (*((*str)-1) == '\\' && *((*str)-2) == '\\'))

INLINE int ASM EndBracePos( REG(a3, STRPTR *str), REG(d4, int braceType), 
	REG(d3, BOOL forward), REG(d2, STRPTR *dest), REG(d5, ULONG *line_count) )
{
   BOOL com = FALSE, ace = FALSE;
   int s = braceType ? braceType : '{';
   int e = (s == '{') ? '}' : ((s == '(') ? ')' : ((s == '[') ? ']' : 0));
   
   register int braces = 1;
   STRPTR orig = (*str);
   
   if(forward) {
       while(*(*str) && *((*str)-1) != braceType) *(*dest)++ = *(*str)++;
       
       assert(*(*str));
   }
   
   //if(e)
   assert(e > 0);
   { // Found a "brace", perform normal operation
     for( ; *(*str) && braces > 0; (*str)++)
     {
             if(*(*str) == '\"' && SLASH_ && (*((*str)-1) != '\'' || *((*str)+1) != '\'')) com = !com;
    	else if(*(*str) == '\'' && SLASH_ && com==FALSE) ace = !ace;
    	else if(*(*str) == '/' && *((*str)+1) == '*')
    	{
    		/* into a comment */
    		while(!(*((*str)-1) == '*' && *(*str) == '/'))
    			*(*dest)++ = *(*str)++;
    	}
    	else if(*(*str) == s && com==FALSE && ace==FALSE) braces++;
    	else if(*(*str) == e && com==FALSE && ace==FALSE) braces--;
    	else if(*(*str) == '\n') (*line_count)++;
    	
    	*(*dest)++ = *(*str);
     }
   }
#if 0
   else
   { // no proper "brace", where end the comment:
     for(; *(*str) && *((*str)-1) != '*' && *(*str) != '/'; *(*dest)++ = *(*str)++);
   }
#endif
   
   return *(*str) ? ((int) ~(orig - (*str))) : -1;
}


/**************************************************************************/
/**************************************************************************/

INLINE BOOL short_string( STRPTR string_to_localize, Locky *dat )
{
	int cstrlen = 0;
	char * str = (char *) string_to_localize;
	
	if(dat->skip_ss)
	{
		while(*str && *str == ' ')
			str++;
		
		while(*str)
		{
			if(*str == '\\')
			{
				for( str++; IsDigit(*str) ; str++ );
				
				str++;
			}
			else if(*str == '%')
			{
				for( str++; IsDigit(*str) ; str++ );
				
				if(*str == 'l') str++;
				str++;
			}
			else
			{
				BOOL incr = TRUE;
				
				if((dat->anumcnt==TRUE) && !IsAlpha(*str) && !IsDigit(*str))
					incr = FALSE;
				
				if( incr )
					cstrlen++;
				
				str++;
			}
		}
		
		if(*(str-1) == ' ') {
			
			for( str-- ; *str && *str == ' ' ; cstrlen--, str++ );
		}
		
		if(++cstrlen > dat->minstrlen)
			return FALSE;
	}
	else {
		while(*str)
		{
			BOOL incr = TRUE;
			
			if((dat->anumcnt==TRUE) && !IsAlpha(*str) && !IsDigit(*str))
				incr = FALSE;
			
			if( incr )
			{
				if(++cstrlen > dat->minstrlen)
					return FALSE;
			}
			
			str++;
		}
	}
	
	return TRUE;
}

/**************************************************************************/
/**************************************************************************/

/** match_patterns: returns TRUE if 'str' match with some pattern
 ** from env:lock/patterns which contains a list of patterns string to 
 ** not localize.
 ***
 ** v0.2: now check also if 'str' match first with dat->validstr
 ** v0.4: this is now a general check function, not only for patterns..
 **/
#define match_patterns __dont_localize /* v0.4 change */
INLINE BOOL match_patterns( STRPTR str, Locky *dat )
{
	Arry p = (*dat->patterns);
	
	assert(str && *str);
	
	if(dat->msl_before)
	{
		if(short_string( str, dat ))
			return TRUE;
	}
	
	if(dat->validstr != NULL)
	{
		if(!_CheckMatch( str, dat->validstr ))
			return TRUE;
	}
	
	for( ; (*p) ; p++ )
	{
		if(_CheckMatch( str, ((STRPTR)(*p))))
			return TRUE;
	}
	
	return FALSE;
}


/**************************************************************************/
/**************************************************************************/


INLINE long already_localized ( STRPTR msg, Locky *dat )
{
	Arry p = (*dat->locale);
	long pos;
	
	if(dat->dupcheck) /* new from version 0.3 */
	{
		for( pos = 0 ; (*p) ; p++, pos++ )
		{
			if(!strcmp( msg, ((STRPTR)(*p))))
				return pos+1;
		}
	}
	
	(*p) = (APTR) strdup( msg );
	assert((*p) != NULL);
	
 //	DBUG(("added new localized string \"%s\"\n", ((STRPTR)(*p))));
	
	(*(++p)) = NULL;
	
	return 0 ;
}

/**************************************************************************/
/**************************************************************************/

#if 0
char *convert_to_upper(char *s)
{
	unsigned char *s1;

	s1 = (unsigned char *)s;
	while (*s1) {
		if ((*s1 > ('a'-1)) && (*s1 < ('z'+1)))
			*s1 += 'A'-'a';
		s1++;
	}
	return (s);
}
#endif

/**************************************************************************/
/**************************************************************************/

INLINE int denc_ch( int ch )
{
	if(ch < 256)
		return( 256 - ch );
	return( ch - 256 );
}

/**
 * DENC(): the function you must use to decode the strings !
static __inline char *DENC( unsigned char * s )
{
	char * string=s;
	for( ; *s ; *s = ((*s<256)?(256-*s):(*s-256)), s++ );
	return string;
}
	OR
#define DENC(str)						\
({								\
	unsigned char * s, * string = s = (unsigned char *)str;	\
	for( ; *s ; *s = ((*s<256)?(256-*s):(*s-256)), s++ );	\
	string;							\
})
 OR basically 256-s ..
 */

INLINE char * Encode(unsigned char * string)
{
	static unsigned char encstr[4096];
	char * es=encstr;
	int maxlen = sizeof(encstr)-10;
	
	*es++ = 'D';
	*es++ = 'E';
	*es++ = 'N';
	*es++ = 'C';
	*es++ = '(';
	*es++ = '\"';
	
	for( *es = 0 ; *string && --maxlen > 0; string++ )
	{
		snprintf( &es[strlen(es)], 5, "\\x%02lX",(long) denc_ch(*string));
	}
	
	assert(maxlen > 0);
	sprintf( &es[strlen(es)], "\")");
	
	return encstr;
}

/**************************************************************************/
/**************************************************************************/


void localize_file( STRPTR filename, Locky *dat )
{
	STRPTR file, s, new_file, d;
	STRPTR string_to_localize = NULL, cbmls = NULL;
	ULONG line_number = 1, __flags = 0;
	BOOL something_localized = FALSE;
	
	DBUG(("Localizing \"%s\"...\n", filename ));
	
	s = file = FileToMem( filename );
	assert(file != NULL);
	//DBUG(("\"%s\"...\n", file ));
	
	assert(__ftm_lsize > 0);
	d = new_file = malloc((__ftm_lsize*10));
	assert(new_file != NULL);
	
	string_to_localize = malloc( STRTOLOCKY_LENGTH );
	assert(string_to_localize != NULL);
	
	/* Comment Beetwhen Multi-Line String */
	cbmls = malloc( 16384 );
	assert(cbmls != NULL);
	
	for( ; *s ; s++ )
	{
		Arry func;
		int brace = 0, ebp_ret;
		
		if(isFlagSet(SetSignal(0,SIGBREAKF_CTRL_C),SIGBREAKF_CTRL_C)) abort ( ) ;
		
		
		// skip C comments
		if(*s == '/' && *(s+1) == '*') {
			if(!strncmp( s, NOLOCKYSTR, strlen(NOLOCKYSTR))) {
				
				/** is we found NOLOCKYSTR, following text is ignored until
				 ** the next NOLOCKYSTR occurence (or EOF)
				 **/
				if(FLAG(NOLOCKY))	CLEARFLAG( NOLOCKY );
				else			SETFLAG( NOLOCKY );
			}
			do {
				if(*s == '\n')
					line_number++;
				
				*d++ = *s++;
				
				if(*(s-1) == '*' && *s == '/')
					break;
				
			} while(*s);
			
			assert((*s));
			
			goto next;
		}
		
		// skip C++ comments
		if(*s == '/' && *(s+1) == '/') {
			if(!strncmp( s, NOLOCKYSTR_NEW, strlen(NOLOCKYSTR_NEW)))
			{
				if(FLAG(NOLOCKY))	CLEARFLAG( NOLOCKY );
				else			SETFLAG( NOLOCKY );
			}
			goto newline;
		}
		
		
		if(FLAG( NOLOCKY ))
			goto next;
		
		
		
		// skip DEBUG calls (those which I use on amigift/giftmui for now)
		if(IsSpace(*s) && (*(s+1) == 'D' && *(s+2) == 'B'))
		{
			if(*(s+3) == 'E') // DBENTER or DBEXIT
				goto newline;
			
			brace = '('; goto move;
		}
		
		
		
		if(*s == '\"') {
			
			// start reading a string to be localized
			
			STRPTR stl = string_to_localize, save, st=s, c=cbmls;
			BOOL defstr = FALSE;
			
			if(*(s+1) == '\"') {	*d++ = *s++;	goto next;	}
			
			cbmls[0] = 0;
			
		sagain:
			
			for( s++ ; *s && (*s != '\"' || (*s == '\"' && *(s-1) == '\\')) ; *stl++ = *s++ );
			
			assert((*s));
			
			save=s;
			for( s++ ; *s && IsSpace(*s) && *s != '\"' ; s++ ) {
				
				#define CRorCRLF	\
					(*(s+2) == '\n' || (*(s+2) == '\r' && *(s+3) == '\n'))
				
				if(*(s+1) == '\\' && CRorCRLF)
				{
					/* a macro-defined multi-line string! */
					s++;
					defstr = TRUE;
				}
				else if(defstr && ((*(s+1) != '\\' && CRorCRLF) /* <-&\/ optimize me! */
					|| (*(s-1) != '\\' && (*s == '\n' || (*s == '\r' && *(s+1) == '\n')))))
				{
					/* EOL for the multi-line defined string */
					break;
				}
				
				if(*(s+1) == '/' && *(s+2) == '*') {
					
					/* a comment between a multi-line string */
					
					for(s++;*s && !(*(s-2) == '*' && *(s-1) == '/');*c++=*s++);
					assert((*s)); --s; *c = 0;
				}
				
				if(*(s+1) == '/' && *(s+2) == '/') {
					
					/* a C++ comment between a multi-line string */
					
					// convert to plain C comment
					*c++ = '/'; *c++ = '*';
					
					for( s += 3 ; *s && *s != '\n' ; *c++ = *s++ );
					assert((*s)); --s;
					
					*c++ = ' '; *c++ = '*'; *c++ = '/'; *c = 0;
				}
				
				
//				if( *s == '\n' )
//					line_number++;
// linecount commented out as after localizing the line number at Locale.h are ok as well
			}
			
			assert((*s));
			
			if(*s == '\"') {
				// its a multi-line string
				goto sagain;
			}
			else
				s=save;
			
			*stl = 0;
			
		//	DBUG(("stl = \"%s\"\n", string_to_localize ));
			
			if( ! match_patterns( string_to_localize, dat ))
			{
				UBYTE msg_[(MSG_MAXLENGTH_BUFLEN+2)], *m=msg_;
				BOOL got_alpha_ch = 0;
				long x, al_pos;
				
				*m++ = 'M'; *m++ = 'S'; *m++ = 'G'; *m++ = '_';
				
				for( x = 0, stl = string_to_localize ; *stl && x < dat->maxmsglen ; x++, stl++ )
				{
					if( ALLOWEDCHAR( *stl )) {
						
						int ch = *stl;
						
						if(dat->uppercase)
						{
							if((ch > ('a'-1)) && (ch < ('z'+1)))
								ch += 'A'-'a';
						}
						
						*m++ = (ch);
						
						got_alpha_ch = 1;
					}
					else *m++ = '_';
					
					if(*stl == '\\' && *(stl+1) != ' ') stl++;
					else if(*stl =='%' && (*(stl+1) != ' ' || *(stl+1) != '%'))
					{
						// hmmm, this isnt safe...
						for( stl++; IsDigit(*stl) ; stl++ );
						
						if(*stl == 'l') stl++;
					}
				}
				
				*m = 0;
				
				if( ! got_alpha_ch || (dat->msl_before==FALSE && ((long)strlen(&msg_[4]) < dat->minstrlen))) {
					
					goto dontlocalize;
				}
				
				// encode the string if requested (new from v0.2)
				if( dat->encode )
				{
					// note than the strings arent saved to Locale.c/locale.h
					
					char * enc = Encode( string_to_localize );
					
					for( ; *enc ; *d++ = *enc++ );
					
					goto locky;
				}
				
				if( ! ( al_pos = already_localized ( msg_, dat )))
				{
					UBYTE temp[4096];
					unsigned int a, b;
					
					snprintf( temp, sizeof(temp)-1,
					
						"// %s:%ld\n#define %s\tgCat( %ld )\n\n",
						
							filename, line_number, msg_, dat->MSG_number );
					
					a = strlen( temp );
					b = Write( dat->locale_fd, temp, a );
					assert(a == b);
					
					snprintf( temp, sizeof(temp)-1,
						"MSG_%04ld (//)\n%s\n;\n", dat->MSG_number, string_to_localize );
					
					a = strlen( temp );
					b = Write( dat->catalog_fd, temp, a );
					assert(a == b);
					
					a = snprintf( temp, sizeof(temp)-1,
						"\t\"%s\",\t/* %04ld */\n", string_to_localize, dat->MSG_number );
					
					assert(a < sizeof(temp)-2);
					
					a = appendT( "locale.c", temp, LOCALE_SEPTAG);
					assert(a == TRUE);
					
					dat->MSG_number++;
				}
				else {
					DBUG(("\"%s\" already localized, as %s (#%ld)\n", string_to_localize, msg_, al_pos ));
				}
				
				for( m=msg_ ; *m ; *d++ = *m++ );
				
				if(cbmls[0]) {
					/* there was a comment betwhen a multi-line string */
					
					for(c=cbmls; *c ; *d++ = *c++ );
				}
				
			  locky:
				something_localized = TRUE;
				continue;
			}
			
			dontlocalize:
			{
				long to_copy = ((long) ~(st - ((STRPTR)s)))+2;
				
				DBUG(("\"%s\" not localized, to_copy = %ld\n", string_to_localize, to_copy));
				
				for( s=st ; to_copy-- > 0 ; *d++ = *s++ );
				goto next ;
			}
		}
		
		
		// skip directives
		if(*s == '#')
		{
			int i = 1;
			
			for( *d++ = *s++ ; *s == 0x20 ; *d++ = *s++ );
			
			if(!strncmp( s, "include", 7 ) || !strncmp( s, "warning", 7 ))
				goto newline;
			
			if(!strncmp( s, "if 0", 4 ) || !strncmp( s, "ifdef DEBUG", 11 ) || !strncmp( s, "ifdef TEST", 10 ))
			 do {
				if(*s == '#') {
					for( *d++ = *s++ ; *s == 0x20 ; *d++ = *s++ );
					
					if(!strncmp( s, "if", 2 )) i++;
					else if(!strncmp( s, "el", 2 ))
					{
						if((i-1) <= 0) break;
					}
					else if(!strncmp( s, "endif", 5 ))
					{
						if(--i <= 0) break;
					}
				}
				else
				if(*s == '\n')
					line_number++;
			} while((*d++ = *s++));
			
			assert((*s));
			
			goto next;
		}
		
		
		// skip strings from functions like getenv as saved into env:locky/
		for(func = (*dat->functions) ; (*func) ; func++ )
		{
			int flen = strlen(((char *)(*func)));
			
			// only "func(" or "func (" are assumed...
			
			if(!strncmp( s, ((char *)(*func)), flen) 
			 && (*(&s[flen]) == '(' || (*(&s[flen]) == ' ' && *(&s[flen+1]) == '(')))
			{
				int ret = EndBracePos( &s, '(', TRUE, &d, &line_number );
				
			//	DBUG(("%s ret = %d\n", ((char *)(*func)), ret ));
				assert( ret != -1 );
			}
		}
		
next:	
		
		if(*s == '\n')
			line_number++;
		
		*d++ = *s;
		
		continue;
		
move:
		assert(brace != 0);
		
		ebp_ret = EndBracePos( &s, brace, TRUE, &d, &line_number );
		
		assert(ebp_ret != -1);
		
		*d++ = *s;
		
		continue;
		
newline:
		while( *s && *s != '\n' ) *d++ = *s++;		assert( (*s) );
		
		goto next;
	}
	
	if( something_localized )
	{
		char bakfile[512];
		BOOL saved;
		
		snprintf( bakfile, sizeof(bakfile)-1, "%s-nl", filename );
		
		DeleteFile( bakfile );
		Rename( filename, bakfile );
		
		*d = 0;
		saved = _WriteFile( filename, new_file );
		assert(saved == TRUE);
		
		DBUG((" - %s localized.\n", filename ));
	}
	
	free(cbmls);
	free(string_to_localize);
	free(new_file);
	free(file);
}

void localize_tree( Locky *dat )
{
	Arry f;
	
	for( f = (*dat->tree) ; (*f) ; f++ )
	{
		//if(!Stricmp((STRPTR)(*f),"locale.h")||!Stricmp((STRPTR)(*f),"locale.c"))
		if(_CheckMatch((STRPTR)(*f), "#?locale#?.(c|h)"))
			continue;
		
		if(_CheckMatch((STRPTR)(*f), dat->spat )) {
			
			if(dat->omitpat != NULL)
			{
				if(_CheckMatch((STRPTR)(*f), dat->omitpat)) {
					
					DBUG(("file \"%s\" omited by user-request.\n", (char *)(*f)));
					continue;
				}
			}
			
			localize_file((STRPTR)(*f), dat);
		}
		else {
			DBUG(("file \"%s\" do not match sources pattern.\n", (char *)(*f)));
		}
	}
	
	DBUG(("tree localized!.\n"));
	exit(3);
}

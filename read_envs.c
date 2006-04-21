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


#include "Locky.h"

#define LOCKY_ENVAR_PATH	"ENV:Locky/"

#define LOCKY_PATTERNS_FILE	LOCKY_ENVAR_PATH "patterns"
#define LOCKY_FUNCTIONS_FILE	LOCKY_ENVAR_PATH "functions"

int file2arry( STRPTR filename, Arry *dest_arry )
{
	int p=0;
	
	char *file = FileToMem( filename ), *f=file, *ln;
	assert(file != NULL);
	
	(*dest_arry)[p] = NULL;
	
	while((ln = _strsep( &f, "\n")))
	{
		if(!(*ln))
			continue;
		
		(*dest_arry)[p++] = (APTR) strdup( ln );
		(*dest_arry)[ p ] = NULL;
		
		assert((*dest_arry)[p-1] != NULL);
	}
	
	free(file);
	
	return p;
}

INLINE Arry read_patterns( void )
{
	STATIC STRPTR patterns[(1024*1024)];
	Arry ptr=(Arry)patterns;
	
	file2arry( LOCKY_PATTERNS_FILE, &ptr );
	
	return (Arry )patterns;
}

INLINE Arry read_functions( char * optfuncs )
{
	STATIC STRPTR functions[(1024*1024)];
	Arry ptr=(Arry)functions;
	int pos;
	
	pos = file2arry( LOCKY_FUNCTIONS_FILE, &ptr );
	
	if(optfuncs)
	{
		char * func, * p = optfuncs;
		
		while((func = _strsep( &p, ",")))
		{
			functions[pos++] = strdup( func );
			functions[ pos ] = NULL;
			
			assert(functions[pos-1] != NULL);
		}
	}
	
	return (Arry )functions;
}


void read_envs( Arry *patterns, Arry *functions, char * optfuncs )
{
	(*patterns) = read_patterns ( ) ;
	(*functions) = read_functions ( optfuncs ) ;
}


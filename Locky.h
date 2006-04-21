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


#ifndef LOCKY_LOCKY_H
#define LOCKY_LOCKY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stabs.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/casorran.h>

#include <clib/debug_protos.h>

#define PROGRAM_NAME		"Locky"
#define PROGRAM_VERSION		"0.4"

#define STRBUFSIZE 65536

typedef STRPTR ** Arry;

typedef struct __locky_data
{
	Arry *tree;		/* sources tree */
	Arry *locale;		/* previous localized strings (MSG_#?) */
	Arry *patterns;		/* patterns which if match with a string to
				   be localized this one get no localized */
	Arry *functions;	/* strings at functions get not localized */
	
	BPTR locale_fd;		/* FD to Locale.h */
	BPTR catalog_fd;	/* FD to #?.cd */
	
	ULONG MSG_number;	/* next localized string number */
	
	STRPTR spat;		/* sources pattern, #?.(c|h) or argv[3] */
	STRPTR validstr;	/* valid string (to localize) pattern */
	STRPTR omitpat;		/* dont localize files matching 'omitpat' */
	
	long minstrlen;		/* minimun string length to localize */
	long maxmsglen;		/* maximun length for MSG_#? strings */
	
	BOOL encode;		/* encode build-in language strings? */
	BOOL dupcheck;		/* check for duplicated string ?.. */
	BOOL uppercase;		/* convert MSG_#? strings to uppercase */
	BOOL msl_before;	/* check minstrlen before localizing */
	BOOL skip_ss;		/* skip scape sequences and formatting
				   tags while using msl_before */
	BOOL anumcnt;
} Locky;

extern Arry read_tree( STRPTR tree, BOOL  __recursive_scan, char * igndirs );
extern Arry open_oldlocale( STRPTR path, long *MSG_number, BPTR *fd );
extern BPTR open_catalog_descriptor( STRPTR path, STRPTR catalog );
extern void free_arry( Arry arry );

extern BOOL ReadDirExAll( STRPTR path, STRPTR *dst_arry, ULONG dst_size, ULONG *arryPos, BOOL recursive, char * igndirs );
extern STRPTR FileToMem( STRPTR FileName );
extern void localize_tree( Locky *dat );
extern void localize_file( STRPTR filename, Locky *dat );

extern void read_envs( Arry *patterns, Arry *functions, char * optfuncs );
extern int file2arry( STRPTR filename, Arry *dest_arry );
extern BOOL appendT( STRPTR file, STRPTR buf, STRPTR apdAt );


#include "istring.h"
#include "debug.h"

/* from version 0.2 this value is no longer hardcoded */
#define LOCALIZED_STRING_MINLEN		4
#define MSG_MAXLENGTH			44
#define MSG_MAXLENGTH_BUFLEN		1024


#endif /* LOCKY_LOCKY_H */

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


#ifndef AMIGIFT_ISTRING_H
#define AMIGIFT_ISTRING_H

#include <SDI_compiler.h>

#define strcmp		__strcmp
#define strncmp		__strncmp
#define strlen		__strlen
#define strstr		__strstr

INLINE int strcmp(const char *s1,const char *s2)
{ unsigned char *p1=(unsigned char *)s1, *p2=(unsigned char *)s2;
  unsigned long r,c;

  do;while(r=*p1++,c=*p2++,!(r-=c) && (char)c); return r;
}

INLINE int strncmp(const char *s1,const char *s2,size_t n)
{ unsigned char *p1=(unsigned char *)s1,*p2=(unsigned char *)s2;
  unsigned long r,c;

  if ((r=n))
    do;while(r=*p1++,c=*p2++,!(r-=c) && (char)c && --n);
  return r;
}

INLINE size_t strlen(const char *string)
{ const char *s=string;
  
//  if((string == NULL) || (string[0] == '\0'))
//  	return 0;
  
  do;while(*s++); return ~(string-s);
}

INLINE char *strstr(const char *s1,const char *s2)
{ const char *c1,*c2;

  do {
    c1 = s1; c2 = s2;
    while(*c1 && *c1==*c2) {
      c1++; c2++;
    }
    if (!*c2)
      return (char *)s1;
  } while(*s1++);
  return (char *)0;
}


/**************************************************************************/

/**************************************************************************/
/**************************************************************************/

/**************************************************************************/


#ifdef NEED_SEPSET
static unsigned char string_set[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

INLINE char *string_sep_set_func (char *string, const char *charset)
{
	unsigned char *str_ptr;
	unsigned char *ptr;

	for (ptr = (unsigned char *)charset; *ptr; ptr++)
		string_set[(int)*ptr] = 1;

	for (str_ptr = string; *str_ptr; str_ptr++)
	{
		if (string_set[(int)*str_ptr])
			break;
	}

	for (ptr = (unsigned char *)charset; *ptr; ptr++)
		string_set[(int)*ptr] = 0;

	if (!str_ptr[0])
		str_ptr = NULL;

	return str_ptr;
}

INLINE char *string_sep_set (char **string, const char *charset)
{
	char *iter, *str;

	if (!string || !*string || !**string)
		return NULL;

	str = *string;

	if((iter = string_sep_set_func(str, charset)))
	{
		*iter = 0;
		iter += sizeof (char);
	}

	*string = iter;

	return str;
}
#endif /* NEED_SEPSET */


INLINE char *string_move(char *dst, const char *src)
{
	if (!dst || !src)
		return dst;

	return memmove (dst, src, strlen (src) + 1);
}

INLINE char *string_trim (char *string)
{
	char *ptr;

	if (!string || !string[0])
		return string;

	/* skip leading whitespace */
	for (ptr = string; (*ptr) ==  0x20; ptr++);

	/* shift downward */
	if (ptr != string)
		string_move (string, ptr);

	if (!string[0])
		return string;

	/* look backwards */
	ptr = string + strlen (string) - 1;

	if((*ptr)==' ')
	{
		while(ptr >= string && ((*ptr) == 0x20))
			ptr--;

		ptr[1] = 0;
	}

	return string;
}

INLINE char *string_sep (char **string, const char *delim)
{
	char *iter, *str;
	
	if (!string || !*string || !**string)
		return NULL;

	str = *string;

	if((iter = strstr(str, delim)))
	{
		*iter = 0;
		iter += strlen(delim);
	}

	*string = iter;

	return str;

}

#undef _strsep
#undef _strsep_set
#undef _strtrim
#undef _strmove

#define _strsep		string_sep
#define _strsep_set	string_sep_set
#define _strtrim	string_trim
#define _strmove	string_move




#endif /* AMIGIFT_ISTRING_H */

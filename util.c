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

#ifndef DEBUG
# define aSSert(exp)	(exp)
#else
# define aSSert(exp)						\
({								\
	BOOL _exp = (BOOL) (exp);				\
	if(_exp) {						\
		DBUG(("unwanted assertion '%s'\n", #exp ));	\
	}	_exp;						\
})
#endif
#undef _sDup
#define _sDup		strdup


#define NUM_OF_BUFS		128
#define ADDPATHBUFS		1024
#define EXALL_FAILED_ABNORMALLY	( (!more) && (IoErr() != ERROR_NO_MORE_ENTRIES) )
#define ALLOCDOSOBJECT_FAILED	( eaControl == NULL )
#define ALLOCMEM_FAILED		( eaBuffer == NULL )
#define BUFFER_TOO_SMALL	((*arryPos) == dst_size)

BOOL ReadDirExAll( STRPTR path, STRPTR *dst_arry, ULONG dst_size, ULONG *arryPos, BOOL recursive, char * igndirs )
{
	struct ExAllControl    *eaControl;
	BPTR                    dirLock;
	struct ExAllData       *eaBuffer, *eaData;
	LONG                    eaBuffSize = NUM_OF_BUFS * sizeof(struct ExAllData);
	LONG                    more;
	UBYTE			addPathBuf[ADDPATHBUFS];
	BOOL			ReadingSubdir = ((*arryPos) != 0);
	
	dirLock = Lock( path, ACCESS_READ);
	
	if(aSSert(dirLock == NULL))	return FALSE;
	
	eaControl = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL, NULL);
	
	if(aSSert(ALLOCDOSOBJECT_FAILED))
	{
		//printf("AllocDosObject failed\n");
		UnLock(dirLock);
		return FALSE;
	}
	
	eaControl->eac_LastKey = 0;
	eaControl->eac_MatchString = (UBYTE *) NULL;
	eaControl->eac_MatchFunc = (struct Hook *) NULL;
	
	eaBuffer = (struct ExAllData *) AllocMem(eaBuffSize, MEMF_ANY);
	
	if(aSSert(ALLOCMEM_FAILED))
	{
		//printf("AllocMem(eaBuffer) failed\n");
		FreeDosObject(DOS_EXALLCONTROL, eaControl);
		UnLock(dirLock);
		return FALSE;
	}
	
	dst_arry[(*arryPos)] = NULL;
	
	do
	{
		more = ExAll(dirLock, eaBuffer, eaBuffSize, ED_COMMENT, eaControl);

		if(aSSert(EXALL_FAILED_ABNORMALLY))
		{
			//printf("ExAll failed abnormally\n");
			break;
		}

		if (eaControl->eac_Entries == 0)
		{
			//printf("no more entries\n");
			continue;
		}
		else
		{
			eaData = (struct ExAllData *) eaBuffer;
			while (eaData)
			{
				if((eaData->ed_Type == ST_ROOT) || (eaData->ed_Type == ST_USERDIR))
				{
				    if(recursive)
				    {
				    	BOOL doit = TRUE;
				    	
				    	if((igndirs != NULL) && _CheckMatch( eaData->ed_Name, igndirs ))
				    	{
				    		DBUG(("Skipping subdirectory \"%s\" by user-request.\n", eaData->ed_Name ));
				    		doit = FALSE;
				    	}
				    	
				    	if(doit == TRUE)
				    	{
						char np[512]; np[0] = '\0';
						AddPart( np, path, 512);
						AddPart( np, eaData->ed_Name, 512);
						
						ReadDirExAll( np, dst_arry, dst_size-(*arryPos), arryPos, TRUE, igndirs );
					}
				    }
				}
				else /*if(ReadingSubdir)*/
				{
					addPathBuf[0] = '\0';
					AddPart( addPathBuf, path, ADDPATHBUFS);
					AddPart( addPathBuf, eaData->ed_Name, ADDPATHBUFS);
					dst_arry[(*arryPos)++] = _sDup(addPathBuf);
					assert(dst_arry[(*arryPos)-1] != NULL);
				}
			/*	else {
					dst_arry[(*arryPos)++] = _sDup(eaData->ed_Name);
					assert(dst_arry[(*arryPos)-1] != NULL);
				}
			*/	
				dst_arry[(*arryPos)] = NULL;
				
				if(aSSert(BUFFER_TOO_SMALL)) {
					more = 0; break;
				}
				
				eaData = eaData->ed_Next;
			}
		}
	}
	while (more);
	
	FreeMem(eaBuffer, eaBuffSize);
	FreeDosObject(DOS_EXALLCONTROL, eaControl);
	UnLock(dirLock);
	
	return TRUE;
}


ULONG __ftm_lsize = 0;

STRPTR FileToMem( STRPTR FileName )
{
  BPTR fh;
  STRPTR buf;
  
  if((fh = Open( FileName, MODE_OLDFILE)))
  {
    struct FileInfoBlock fib;
    ULONG size=0;
    
    if(ExamineFH( fh, &fib))
    	size = fib.fib_Size;
    
    if((__ftm_lsize = size))
    {
    //	DBUG(("%s's size = %ld\n", FileName, size ));
      if((buf = (STRPTR) malloc(size+2)))
      {
        ULONG rEad = Read(fh, buf, size);
        Close(fh); fh = NULL;
        buf[size] = '\0';
        if((rEad == size)) return buf;
        free(buf);
      }
    }
    if(fh)
	Close(fh);
  }
return NULL;
}





BOOL appendT( STRPTR file, STRPTR buf, STRPTR apdAt )
{
	STRPTR fmem, f;
	int apdAt_len;
	BPTR fd;
	
	assert(file != NULL);
	assert(buf != NULL);
	assert(apdAt != NULL);
	
	fmem = f = FileToMem( file );
	assert(fmem != NULL);
	
	apdAt_len = strlen(apdAt);
	
	do {
		if(!strncmp( fmem, apdAt, apdAt_len))
			break;
		
	} while(*++fmem);
	
	assert((*fmem));
	
	fd = Open( file, MODE_NEWFILE);
	assert(fd != NULL);
	
	Write( fd, f, ((long) ~(f-fmem))+1);
	Write( fd, buf, strlen(buf));
	Write( fd, fmem, strlen(fmem));
	Close( fd );
	
	free(f);
	
	return TRUE;
}





void print_arry( STRPTR ** arry, STRPTR arryname )
{
	long x = 0;
	
	for( ; (*arry) ; arry++ )
	{
		kprintf(" %s[%ld] = \"%s\"\n", arryname, x++, (*arry));
	}
}



void free_arry( Arry arry )
{
	for( ; (*arry) ; arry++ )
	{
		free((*arry));
	}
}



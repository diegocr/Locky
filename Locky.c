
/**
 * Locky (c)2005 Diego Casorran <diegocr@users.sf.net>
 * All rights reserved - Brought to you under the BSD Licence!.
 * 
 * ChangeLog:
 * 	0.1, 21-oct-2005: First Version.
 * 	0.2, 28-feb-2006: Added options -s, -m, -e
 * 	0.3, 08-mar-2006: Added options -n, -u, -x
 * 	0.4, 16-abr-2006: Added options -b, -j, -g, -o, -r, -i, -a
 * 
 * $VER: $Id: Locky.c,v 0.3 2006/03/08 11:35:17 diegocr Exp $
 */

#include "Locky.h"

Arry tree=NULL, locale=NULL, patterns=NULL, functions=NULL;
BPTR catalog_fd=0, locale_fd=0;

#ifdef DEBUG
int optdebug = 1;
#else
int optdebug = 0;
#endif

int main(int argc, char *argv[])
{
	long MSG_number = 0;
	Locky *data;
	
	int opt, opthelp=0, optdupcheck=1, optuppercase=0, encode = 0, 
		minlen = LOCALIZED_STRING_MINLEN, maxmsglen = MSG_MAXLENGTH,
		msl_before = FALSE, skip_ss = FALSE, recursive_tree = FALSE,
		anumcnt = FALSE ;
	
	char *path = "", *file = NULL, *spat = NULL, *cdfile = NULL, 
		*localepath = NULL, * vstr = NULL, * optfuncs = NULL,
		* omitpat = NULL, * igndirs = NULL;
	
	while((opt = getopt( argc, argv, "c:Vhdt:p:f:l:s:m:enux:o:bjg:ari:")) != -1)
	{
		switch( opt )
		{
			case 'c':
				cdfile = strdup(optarg);
				break;
			
			case 'd':
				optdebug = TRUE;
				break;
			
			case 't':
				path = strdup(optarg);
				assert(path != NULL);
				break;
			
			case 'l':
				localepath = strdup(optarg);
				assert(localepath != NULL);
				break;
			
			case 'p':
				spat = strdup(optarg);
				assert(spat != NULL);
				break;
			
			case 'f':
				file = strdup(optarg);
				assert(file != NULL);
				break;
			
			case 's':
				vstr = strdup(optarg);
				assert(vstr != NULL);
				break;
			
			case 'x':
				optfuncs = strdup(optarg);
				assert(optfuncs != NULL);
				break;
			
			case 'o':
				omitpat = strdup(optarg);
				assert(omitpat != NULL);
				break;
			
			case 'm':
				minlen = atoi(optarg);
				assert(minlen > 0);
				break;
			
			case 'g':
				maxmsglen = atoi(optarg);
				assert(maxmsglen > 0);
				assert(maxmsglen < MSG_MAXLENGTH_BUFLEN);
				break;
			
			case 'e':
				encode = TRUE;
				break;
			
			case 'a':
				anumcnt = TRUE;
				break;
			
			case 'j':
				skip_ss = TRUE;
				/* -j implies -b */
			case 'b':
				msl_before = TRUE;
				break;
			
			case 'i':
				igndirs = strdup(optarg);
				assert(igndirs != NULL);
				/* -i implies -r */
			case 'r':
				recursive_tree = TRUE;
				break;
			
			case 'u':
				optuppercase = TRUE;
				/* -u implies -n */
			case 'n':
				optdupcheck = FALSE;
				break;
			
			case 'V':
				printf( PROGRAM_NAME " v" PROGRAM_VERSION " (c)2005 Diego Casorran.\n");
				printf("Compiled for " BUILD_SYSTEM " On " __DATE__ ", " __TIME__ "\n");
				
			case 'h':
			default:
				opthelp = TRUE;
				break;
		}
	}
	
	if(/*optind >= argc ||*/ opthelp)
	{	
		printf("Usage: %s -c CDFILE [options]\n", argv[0] );
		printf("Options:\n");
		printf("  -h            This help message\n");
		printf("  -c<catalog>   Catalog descriptor file (REQUIRED)\n");
		printf("  -t<tree>      Read sources from <tree> path\n");
		printf("  -r            Do recursive scan to get sources while using -t\n");
		printf("  -i<pattern>   do not recurse into subdirs matching <pattern>, implies -r\n");
		printf("  -p<pattern>   Use <pattern> to source files [#?.(c|h)]\n");
		printf("  -f<file>      Localize <file> only.\n");
		printf("  -l<locale>    load/save locatized string from/to <locale> path\n");
		printf("  -s<pattern>   Only localize strings which match with <pattern>\n");
		printf("  -m<value>     Only localize strings which length is greater than <value>\n");
		printf("  -b            Check for minstrlen (-m) on the string before localizing it\n");
		printf("  -j            Skip all kind of scape sequences and formating tags while using -b\n");
		printf("  -a            Count alpha-numeric chars only, upto -m, while using -b\n");
		printf("  -g<value>     Maximun length of MSG_#? strings. (default: %d)\n", MSG_MAXLENGTH );
		printf("  -o<pattern>   Do not localize files which match with <pattern>.(c|h)\n");
		printf("  -n            localize all strings even if duplicated\n");
		printf("  -u            Convert localized MSG_#? strings to uppercase (implies -n)\n");
		printf("  -x<func,func> Comma separated list of functions whose strings get not localized\n");
		printf("  -e            Encode build-in language strings...\n");
		printf("  -d            Print debugging messages\n");
		printf("  -V            print version info.\n");
		return EXIT_FAILURE;
	}
	
	assert(cdfile != NULL);
	
	if(spat == NULL)
		spat = strdup("#?.(c|h)");
	
	assert(spat != NULL);
	
	data = malloc(sizeof(*data));
	assert(data != NULL);
	
	memset( data, 0, sizeof(*data));
	
	if(file == NULL) {
		
		tree = read_tree( path, recursive_tree, igndirs );
		assert(tree != NULL);
	}
	
	catalog_fd = open_catalog_descriptor((localepath != NULL) ? localepath:path, cdfile );
	
	locale = open_oldlocale((localepath != NULL) ? localepath:path, &MSG_number, &locale_fd);
	
	read_envs( &patterns, &functions, optfuncs );
	assert(patterns != NULL);
	assert(functions != NULL);
	
	#ifdef DEBUG
	if(optdebug)
	{
		if(tree)
			print_arry( tree, "sources_tree");
		if(locale[0])
			print_arry( locale, "localized_strings" );
		print_arry( patterns, "patterns" );
		print_arry( functions, "functions" );
	}
	#endif
	
	
	data->tree		= &tree;
	data->locale		= &locale;
	data->patterns		= &patterns;
	data->functions		= &functions;
	data->locale_fd		= locale_fd;
	data->catalog_fd	= catalog_fd;
	data->MSG_number	= MSG_number;
	data->spat		= spat;
	data->validstr		= vstr;
	data->minstrlen		= minlen;
	data->encode		= encode;
	data->dupcheck		= optdupcheck;
	data->uppercase		= optuppercase;
	data->omitpat		= omitpat;
	data->msl_before	= msl_before;
	data->maxmsglen		= maxmsglen;
	data->skip_ss		= skip_ss;
	data->anumcnt		= anumcnt;
	
	if( file ) {
		localize_file ( file, data );
		free(file);
	}
	else {
		localize_tree ( data );
	}
	
	free(cdfile);
	free(data);
	
	if(path && *path)
		free(path);
	
	if(localepath)
		free(localepath);
	
	if(optfuncs)
		free(optfuncs);
	
	if(igndirs)
		free(igndirs);
	
	if(omitpat)
		free(omitpat);
	
	//return EXIT_SUCCESS;
	exit(EXIT_SUCCESS); // why there is a 'illegal instruction' when using return ?
}


DESTRUCTOR(finish)
{
	DBUG(("freeing internals...\n"));
	
	if(catalog_fd)
		Close(catalog_fd);
	
	if(locale_fd)
		Close(locale_fd);
	
	if(tree)
		free_arry(tree);
	
	if(locale)
		free_arry(locale);
	
	if(patterns)
		free_arry(patterns);
	
	if(functions)
		free_arry(functions);
	
	DBUG(("all done, have a nice day :-)\n"));
}


/*************************************************************************/
/*************************************************************************/
#if 1
#include <stdarg.h>

int __vsnprintf_nfp(char *buffer, size_t buffersize, const char *fmt0, _BSD_VA_LIST_ ap);

int vsnprintf(char *buffer, size_t buffersize, const char *fmt0, _BSD_VA_LIST_ ap)
{
	return __vsnprintf_nfp( buffer, buffersize, fmt0, ap );
}
#endif
/*************************************************************************/
/*************************************************************************/

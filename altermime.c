/*------------------------------------------------------------------------
Module:        /extra/development/altermime-0.1.1/altermime.c
Author:        Paul L Daniels
Project:       AlterMime
State:         Development
Creation Date: 02/05/2001
Description:   Altermime is a program/object which will allow arbitary alterations to a given MIME encoded mailpack, such as disclaimer additions and attachment-nullification.
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "mime_alter.h"
#include "logger.h"

/* Global DEFINES */
char ALTERMIMEAPP_VERSION[]="alterMIME v0.3.11 (November-2008) by Paul L Daniels - http://www.pldaniels.com/altermime\n";
char ALTERMIMEAPP_USAGE[]="altermime --input=<input mime pack>   ( --input=- for stdin )\n"
"	[--disclaimer=<disclaimer file>]\n"
"	[--disclaimer-html=<HTML disclaimer file>]\n"
"	[--disclaimer-b64=<BASE64 encoded dislcaimer>]\n"
"	[--htmltoo]\n"
#ifdef DISPOS
"	[--textpos=<positioning code>]\n"
"	[--htmlpos=<positioning code>]\n"
#endif

#ifdef ALTERMIME_PRETEXT
/*"	[--pretext] Insert disclaimer files in pretext mode.\n"*/ // By implication, pretext is inserted based on the file status below.
"	[--pretext=<pretext file>]\n"
"	[--pretext-html=<pretext HTML file>]\n"
"	[--pretext-b64=<BASE64 encoded pretext >]\n"
#endif

"	[--force-into-b64]\n"
"	[--force-for-bad-html]\n"
"	[--multipart-insert]\n"
"	[--remove=<remove file name (regex)>] (if filename contains a /, matches on mime-type )\n"
"	[--removeall]\n"
"	[--replace=<filename to replace> --with=<replace with>]\n"
"	[--xheader=\"...\"\n"
"	[--alter-header=\"...\" --alter-with=\"...\" --alter-mode=<prefix|suffix|replace>]\n"
"	[--altersigned]\n"
"	[--tmpdir=<directory>\n"
"	[--no-qmail-bounce]\n"
"	[--verbose]\n"
"	[--log-stdout]\n"
"	[--log-stderr]\n"
"	[--log-syslog]\n"
"	[--debug]\n"
"	[--version]\n\n"
"Option Descriptions:\n"
"\t--input=, Sets the mailpack file to be the filename supplied,\n"
"\t\tif the filename is a single '-' (hyphen) then the mailpack\n"
"\t\tis sourced via stdin and outputted via stdout.\n"
"\n"
"\t--disclaimer=, Set the plaintext disclaimer source file.\n"
"\t--disclaimer-html=, Set the HTML disclaimer source file.\n"
"\t--disclaimer-b64=, Set the BASE64 encoded disclaimer source file (implies --force-into-b64).\n"
"\n"
"\t--htmltoo, Sets alterMIME to insert the plaintext disclaimer into\n"
"\t--force-into-b64, Sets alterMIME to insert disclaimers into BASE64 encoded text segments\n"
"\t--force-for-bad-html, Force adding of the HTML disclaimer even when HTML is not correctly formatted\n"
"\t\tthe HTML portion of the email body ( if there is no explicitly\n"
"\t\tdefined HTML dislcaimer, see --disclaimer-html )\n"
"\n"
"\t--disclaimer-attachment=<filename> (use $CID in the disclaimer text to reference)\n"
"\n"
"\t--remove=, Remove any attachments which match the filename supplied,\n"
"\t\tif the filename text contains a forward-slash '/', then the\n"
"\t\tmatching will occur based on content-type headers rather than\n"
"\t\tby filename.\n"
"\t--removeall, Remove all attachments\n"
"\t--replace=, Replace attachments matching the given filename. Requires to\n"
"\t\tbe used with --with.\n"
"\t--with=, Replace the attachments specified by --replace with the file\n"
"\t\tspecified.\n"
"\t--xheader=, Insert a header line as specified into the first set of headers.\n"
"\t--alter-header=\"...\" --alter-with=\"...\" --alter-mode=(prefix|suffix|replace)\n"
"\t\tAlter an existing header in the mailpack.  This function modifies the\n"
"\t\tvalue of the header, as apposed to the header name.\n"
"\t--altersigned, Force alterMIME to modify 'signed' emails\n"
"\t--tmpdir=<directory>,  where to store temporary files when processing from STDIN\n"
"\t--no-qmail-bounce,  Don't search into email bodies for attachment headers\n"
"\t--verbose, Describe details of the process occuring\n"
"\t--log-stdout, Send all output messages to stdout\n"
"\t--log-stderr, Send all output messages to stderr\n"
"\t--log-syslog, Send all output messages to syslog\n"
"\t--debug, Provide greater verbosity and debugging information\n"
"\t--version, display the alterMIME version string\n"
"\n";

/* Global variables / settings */
char ALTERMIMEAPP_default_remove_prefix[]="removed";
char ALTERMIMEAPP_removeall_filename[]=".*";

struct ALTERMIMEAPP_globals {
	char tmpdir[1024];
	char *input_file;
	int input_is_stdin;
	char *disclaimer_file;
	char *disclaimer_html_file;
	char *disclaimer_b64_file;
	char *disclaimer_attachment;
	int   disclaimer_insert;
	char *pretext_file;
	char *pretext_html_file;
	char *pretext_b64_file;
	int   pretext_insert;
	char *remove_filename;
	char *replace;
	char *with;
	char *xheader;

	char *alter_header;
	char *alter_with;
	int alter_mode;

	int verbose;
};


/*------------------------------------------------------------------------
Procedure:     parse_args ID:1
Purpose:       Parses the command line arguments and sets up internal parameters
Input:         argc: The number of arguments
argv: Array of strings
Output:        none
Errors:
------------------------------------------------------------------------*/
int ALTERMIMEAPP_parse_args( struct ALTERMIMEAPP_globals *glb, int argc, char **argv )
{
	int i;
	char *p=NULL;
	char *q=NULL;

	for (i = 1; i < argc; i++){
		if ((argv[i][0] == '-')&&(argv[i][1] == '-'))
		{
			p = argv[i];
			p += 2;

			if (strncmp(p,"input=",6)==0) {
				glb->input_file = p +strlen("input=");

			}	else if (strncmp(p,"tmpdir=",7)==0) {
				snprintf(glb->tmpdir, sizeof(glb->tmpdir), "%s" ,p +strlen("tmpdir="));

			} else if (strncmp(p,"htmltoo",7)==0) {
				AM_set_HTMLtoo(1);

			} else if (strncmp(p,"force-into-b64",sizeof("force-into-b64"))==0) {
				AM_set_force_into_b64(1);

			} else if (strncmp(p,"force-for-bad-html",sizeof("force-for-bad-html"))==0) {
				AM_set_force_for_bad_html(1);

			} else if (strncmp(p,"multipart-insert", strlen("multipart-insert"))==0) {
				AM_set_multipart_insert(1);


			} else if (strncmp(p,"pretext=", strlen("pretext="))==0) {
				glb->pretext_file = p +strlen("pretext=");
				glb->pretext_insert = 1;
				//				LOGGER_log("%s:%d:ALTERMIMEAPP_parse_args:DEBUG: pretext set to '%s'", FL, glb->pretext_file);

			} else if (strncmp(p,"pretext-html=",strlen("pretext-html="))==0) {
				glb->pretext_html_file = p +strlen("pretext-html=");
				AM_set_HTMLtoo(1);
				glb->pretext_insert = 1;

			} else if (strncmp(p,"pretext-b64=",strlen("pretext-b64="))==0) {
				glb->pretext_html_file = p +strlen("pretext-b64=");
				AM_set_force_into_b64(1);
				glb->pretext_insert = 1;



			} else if (strncmp(p,"disclaimer=",11)==0) {
				glb->disclaimer_file = p +strlen("disclaimer=");
				AM_set_disclaimer_plain( glb->disclaimer_file, AM_DISCLAIMER_TYPE_FILENAME );
				glb->disclaimer_insert = 1;

			} else if (strncmp(p,"disclaimer-html=",16)==0) {
				glb->disclaimer_html_file = p +strlen("disclaimer-html=");
				AM_set_disclaimer_HTML( glb->disclaimer_html_file, AM_DISCLAIMER_TYPE_FILENAME );
				glb->disclaimer_insert = 1;
				AM_set_HTMLtoo(1);

			} else if (strncmp(p,"disclaimer-b64=",strlen("disclaimer-b64="))==0) {
				glb->disclaimer_b64_file = p +strlen("disclaimer-b64=");
				AM_set_disclaimer_b64( glb->disclaimer_b64_file, AM_DISCLAIMER_TYPE_FILENAME );
				glb->disclaimer_insert = 1;
				AM_set_force_into_b64(1);

			} else if (strncmp(p,"disclaimer-attachment=",strlen("disclaimer-attachment="))==0) {
				glb->disclaimer_attachment = p +strlen("disclaimer-attachment=");
				AM_set_disclaimer_attachment( glb->disclaimer_attachment, 0);

			} else if (strncmp(p,"remove=",7)==0) {
				glb->remove_filename = p +strlen("remove=");

			} else if (strncmp(p, "remove_prefix=",13)==0) {
				LOGGER_log("--remove_prefix is depricated, ignoring");
				//glb->remove_prefix = p +strlen("remove_prefix=");
				//
			} else if (strncmp(p, "removeall",9)==0) {
				glb->remove_filename = ALTERMIMEAPP_removeall_filename;

			} else if (strncmp(p, "altersigned",11)==0) {
				AM_set_altersigned(1);

			} else if (strncmp(p, "replace",7)==0) {
				glb->replace = p +strlen("replace=");

			} else if (strncmp(p, "with",4)==0) {
				glb->with = p +strlen("with=");

			} else if (strncmp(p, "xheader",7)==0) {
				glb->xheader = p +strlen("xheader=");

			} else if (strncmp(p, "version",7)==0) {
				fprintf(stdout,"%s",ALTERMIMEAPP_VERSION);
				exit(0);

			} else if (strncmp(p, "debug", 5)==0) {
				AM_set_debug(1);

			} else if (strncmp(p, "no-qmail-bounce",15)==0) {
				AM_set_header_long_search(0);

			} else if (strncmp(p, "verbose",7)==0) {
				AM_set_verbose(1);

			} else if (strncmp(p, "alter-header", strlen("alter-header"))==0) {
				glb->alter_header = p +strlen("alter-header=");
				if (*glb->alter_header == '\"') glb->alter_header++;

			} else if (strncmp(p, "alter-with", strlen("alter-with"))==0) {
				glb->alter_with = p +strlen("alter-with=");
				if (*glb->alter_with == '\"') glb->alter_with++;

			} else if (strncmp(p, "alter-mode", strlen("alter-mode"))==0) {
				q = p +strlen("alter-mode=");
				if (*q == '\"') q++;

				glb->alter_mode = AM_HEADER_ADJUST_MODE_NONE;
				if (strncmp( q, "prefix", strlen("prefix"))==0) { glb->alter_mode = AM_HEADER_ADJUST_MODE_PREFIX; }
				else if (strncmp( q, "suffix", strlen("suffix"))==0) { glb->alter_mode = AM_HEADER_ADJUST_MODE_SUFFIX; }
				else if (strncmp( q, "replace", strlen("replace"))==0) { glb->alter_mode = AM_HEADER_ADJUST_MODE_REPLACE; }
				else { LOGGER_log("ERROR: Unknown header alter mode '%s'. Please use either of prefix, suffix or replace.", q); }
			}
			else if (strncmp(p, "log-stdout",strlen("log-stdout"))==0) { LOGGER_set_output_mode(_LOGGER_STDOUT); }
			else if (strncmp(p, "log-stderr",strlen("log-stderr"))==0) { LOGGER_set_output_mode(_LOGGER_STDERR); }
			else if (strncmp(p, "log-syslog",strlen("log-syslog"))==0) { LOGGER_set_output_mode(_LOGGER_SYSLOG); LOGGER_set_syslog_mode( LOG_MAIL|LOG_INFO ); }
			else
			{
				LOGGER_log("Error, unknown parameter \"%s\"\n",p);
				LOGGER_log("%s",ALTERMIMEAPP_USAGE);
				exit(1);
			} /* if-ELSE */
		} /* if argv == '--' */
	} /* for */

	return 0;
}

/*-----------------------------------------------------------------\
  Function Name	: ALTERMIMEAPP_init
  Returns Type	: int
  ----Parameter List
  1. struct ALTERMIMEAPP_globals *glb , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int ALTERMIMEAPP_init( struct ALTERMIMEAPP_globals *glb )
{
	glb->input_is_stdin			= 0;
	glb->input_file 				= NULL;
	glb->disclaimer_file 		= NULL;
	glb->disclaimer_html_file 	= NULL;
	glb->disclaimer_b64_file	= NULL;
	glb->disclaimer_insert		= 0;

#ifdef ALTERMIME_PRETEXT
	glb->pretext_insert		= 0;
	glb->pretext_file			= NULL;
	glb->pretext_html_file	= NULL;
	glb->pretext_b64_file	= NULL;
#endif

	glb->remove_filename 		= NULL;
	glb->replace 	= NULL;
	glb->with 		= NULL;
	glb->xheader 	= NULL;
	glb->verbose 	= 0;

	glb->alter_header = NULL;
	glb->alter_with = NULL;
	glb->alter_mode = AM_HEADER_ADJUST_MODE_NONE;

	snprintf(glb->tmpdir, sizeof(glb->tmpdir),"/tmp");
	return 0;
}



/*-----------------------------------------------------------------\
  Function Name	: main
  Returns Type	: int
  ----Parameter List
  1. int argc, 
  2.  char **argv , 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
int main( int argc, char **argv )
{
	struct ALTERMIMEAPP_globals glb;
	FILE *f;

	LOGGER_set_output_mode(_LOGGER_STDOUT);

	ALTERMIMEAPP_init( &glb );

	ALTERMIMEAPP_parse_args(&glb,argc, argv);

	if (!glb.input_file) {
		LOGGER_log("Error: No input file specified\n");
		LOGGER_log(ALTERMIMEAPP_USAGE);
		exit(1);
	}

	if (((!glb.replace)&&(glb.with))||((glb.replace)&&(!glb.with)))
	{
		LOGGER_log("Error: Both --replace= and --with= must be set\n");
		exit(1);
	}

	if( glb.input_file && !(glb.alter_mode||glb.replace||glb.disclaimer_file||glb.remove_filename||glb.xheader||glb.pretext_file||glb.pretext_html_file)) {
		LOGGER_log("Error: Must specify an action for the input file.\n");
		LOGGER_log( ALTERMIMEAPP_USAGE);
		exit(1);
	}

	if ((strcmp(glb.input_file,"-") == 0) && (/**glb.disclaimer_file||**/glb.replace||glb.xheader)) {
		LOGGER_log("Error: reading/writing from stdin/stdout not implemented for --xheader,--disclaimer, or --replace.\n");
		LOGGER_log(ALTERMIMEAPP_USAGE);
		exit(1);
	}

	if (glb.input_file[0]=='-') {
		char scratch[1024];
		size_t rc;

		glb.input_is_stdin = 1;

		snprintf(scratch,sizeof(scratch),"%s/%ld%d.amtmp", glb.tmpdir, time(NULL), getpid());
		//		free(glb.input_file);

		glb.input_file = strdup(scratch);
		f = fopen(glb.input_file, "w");
		if (!f) {
			LOGGER_log("%s:%d:ERROR: Cannot open '%s' for writing as a temporary file for alterMIME (%s)", FL, scratch, strerror(errno));
			exit(1);
		} else {

			do {
				rc = fread(scratch, 1, sizeof(scratch), stdin);
				if (rc == -1) {
					LOGGER_log("%s:%d:ERROR: Cannot read from  '%s' (%s)", FL, glb.input_file, strerror(errno));
					exit(1);
				}
				if (rc) {
					rc = fwrite(scratch, 1, rc, f);
					if (rc == -1) {
						LOGGER_log("%s:%d:ERROR: Cannot write to '%s' (%s)", FL, glb.input_file, strerror(errno));
						exit(1);
					}
				}
			} while (rc);
			fclose(f);
		}

	}

	if ((glb.alter_mode != AM_HEADER_ADJUST_MODE_NONE)&&(glb.alter_with != NULL)&&(glb.alter_header != NULL))
	{
		AM_alter_header( glb.input_file, glb.alter_header, glb.alter_with, glb.alter_mode );
	}

	if ((glb.replace)&&(glb.with)) AM_attachment_replace( glb.input_file, glb.replace, glb.with);

#ifdef ALTERMIME_PRETEXT
	AM_set_pretext(0);
#endif
	if (glb.disclaimer_file) AM_add_disclaimer( glb.input_file );
	if (glb.remove_filename) AM_nullify_attachment(glb.input_file, glb.remove_filename);
	if (glb.xheader) AM_insert_Xheader( glb.input_file, glb.xheader);
	AM_done();

#ifdef ALTERMIME_PRETEXT
	if (glb.pretext_insert) {
		//		LOGGER_log("%s:%d:alterMIME: PRETEXT STAGE",FL);
		//		LOGGER_log("%s:%d:alterMIME: PRETEXT STAGE",FL);
		//		LOGGER_log("%s:%d:alterMIME: PRETEXT STAGE",FL);
		//		LOGGER_log("%s:%d:alterMIME: PRETEXT STAGE",FL);
		AM_init();
		//		AM_set_debug(1);
		AM_set_pretext(1);
		if (glb.pretext_file) AM_set_disclaimer_plain( glb.pretext_file, AM_DISCLAIMER_TYPE_FILENAME );
		if (glb.pretext_html_file) {
			AM_set_disclaimer_HTML( glb.pretext_html_file, AM_DISCLAIMER_TYPE_FILENAME );
			AM_set_HTMLtoo(1);
		}
		if (glb.pretext_b64_file) AM_set_disclaimer_b64( glb.pretext_b64_file, AM_DISCLAIMER_TYPE_FILENAME );
		AM_add_disclaimer( glb.input_file );
	}
#endif


	if (glb.input_is_stdin) {
		char scratch[1024];
		size_t rc;
		FILE *f;

		f = fopen(glb.input_file, "r");
		if (!f) {
			LOGGER_log("%s:%d:ERROR: Cannot open alterMIME temporary file '%s' for reading (%s)", FL, scratch, strerror(errno));
			exit(1);
		}

		do {
			rc = fread(scratch, 1, sizeof(scratch), f);
			if (rc == -1) {
				LOGGER_log("%s:%d:ERROR: Cannot read from  '%s' (%s)", FL, glb.input_file, strerror(errno));
				exit(1);
			}
			if (rc) {
				rc = fwrite(scratch, 1, rc, stdout);
				if (rc == -1) {
					LOGGER_log("%s:%d:ERROR: Cannot write to '%s' (%s)", FL, glb.input_file, strerror(errno));
					exit(1);
				}
			}
		} while (rc);
		fclose(f);

		unlink(glb.input_file);

	}


	return 0;
}

/*-------------------------------------------------END. */

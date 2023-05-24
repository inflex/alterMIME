#ifndef __alterMIME__
#define __alterMIME__

#define LIBAM_VERSION "202305242149"

#define AM_RETURN_SIGNED_EMAIL 10
#define AM_RETURN_B64_ENCODED_EMAIL 12

#define AM_DISCLAIMER_TYPE_NONE 0
#define AM_DISCLAIMER_TYPE_FILENAME 1
#define AM_DISCLAIMER_TYPE_TEXT 2

#define AM_PRETEXT_TYPE_NONE 0
#define AM_PRETEXT_TYPE_FILENAME 1
#define AM_PRETEXT_TYPE_TEXT 2

#define AM_HEADER_ADJUST_MODE_NONE 0
#define AM_HEADER_ADJUST_MODE_PREFIX 1
#define AM_HEADER_ADJUST_MODE_SUFFIX 2
#define AM_HEADER_ADJUST_MODE_REPLACE 4

#define AM_NULLIFY_MATCH_MODE_NONE 0
#define AM_NULLIFY_MATCH_MODE_FILENAME 1
#define AM_NULLIFY_MATCH_MODE_CONTENT_TYPE 2

#define AM_BOUNDARY_BUFFER_SIZE 1024

struct AM_disclaimer_details {

	// File details
	long segment_start; // used to remember where the current MIME segment starts in the file.

	// Header details

	int content_type;
	int content_encoding;
	int boundary_found;
	char boundary[AM_BOUNDARY_BUFFER_SIZE];

	//

	int isb64;
	int ishtml;
	int isfile;
	int text_inserted;
	int html_inserted;
	int b64_inserted;

	//

	char *disclaimer_text_plain;
	char *disclaimer_text_HTML;
	char *disclaimer_text_b64;
	char *disclaimer_attachment; 


	/** Positional definitions for the HTML and text disclaimers **/
	char textpos[1024];
	char htmlpos[1024];

};


#define AM_HEADERBUFFER_MAX 100
#define AM_HEADERBUFFER_ITEM_SIZE 1024

struct AM_globals {

	int debug;				// Low level debugging
	int verbose;				/* do we talk as we walk */
	int paranoid;				/* set paranoid to yes! */
	int HTML_too;				/* Add footer to the HTML email too */
	int force_for_bad_html;	/** Force insertion of HTML disclaimer even when we can't find the end **/
	int force_into_b64;		/** Force headers into Base64 encoded bodies **/
	int multipart_insert;	/* Should we insert into emails which are embedded into another */
	int nullify_all;			/* Remove ALL filename'd attachments */
	int alter_signed;		/* Do we alter signed emails ? */
	int header_long_search;	/* do we search through email bodies for more headers, like qmail bounces */

	char ldelimeter[3];

	char *disclaimer_plain_base_fname; // will always hold the primary filename of the plain-text disclaimer. Need this for the charset extensions
	char *disclaimer_plain;
	int disclaimer_plain_type;

	char *disclaimer_HTML_base_fname; // will always hold the primary filename of the HTML disclaimer. Need this for the charset extensions
	char *disclaimer_HTML;
	int disclaimer_HTML_type;

	char *disclaimer_b64;
	int disclaimer_b64_type;

	char *disclaimer_attachment;

#ifdef ALTERMIME_PRETEXT

	char *pretext_plain;
	int pretext_plain_type;
	char *pretext_HTML;
	int pretext_HTML_type;
	char *pretext_b64;
	int pretext_b64_type;

	int pretext_insert;
#endif

	char *headerbuffer[ AM_HEADERBUFFER_MAX ];	// 100 lines for the header buffers

	int headerbuffermax;
};


#define AMSTATUSFLAGS_TEXT_INSERTED 1
#define AMSTATUSFLAGS_HTML_INSERTED 2
#define AMSTATUSFLAGS_B64_INSERTED 3
#define AMSTATUSFLAGS_XHEADER_INSERTED 4

extern unsigned int altermime_status_flags; // Status flags


int AM_version( void );

int AM_init( void );
int AM_done( void );
int AM_set_debug( int level );

char *AM_set_disclaimer_b64( char *filename, int disclaimer_type );
char *AM_set_disclaimer_plain( char *filename, int disclaimer_type );
char *AM_set_disclaimer_HTML( char *filename, int disclaimer_type );
char *AM_set_disclaimer_attachment( char *filename, int index );

#ifdef ALTERMIME_PRETEXT
int AM_set_pretext_plain( char *filename, int disclaimer_type );
int AM_set_pretext_HTML( char *filename, int disclaimer_type );
int AM_set_pretext_insert( int level );
int AM_add_pretext( char *mpackname );
#endif

int AM_add_disclaimer( char *mpackname );
int AM_nullify_attachment( char *mpackname, char *attachmentname );
int AM_set_verbose( int level );
int AM_set_HTMLtoo( int level );
int AM_set_force_for_bad_html( int level );
int AM_set_force_into_b64( int level );
int AM_set_multipart_insert( int level );
int AM_set_nullifyall( int level );
int AM_set_altersigned( int level );
int AM_set_pretext( int level );
int AM_set_header_long_search( int level );
int AM_base64_encode( char *enc_fname, char *out_fname );
int AM_attachment_replace( char *mpackname, char *attachmentname, char *new_attachment_name );
int AM_insert_Xheader( char *fname, char *xheader);
int AM_alter_header( char *filename, char *header, char *change, int change_mode );

int AM_insert_file( char *fname, int encoding_format, char *headers, char *boundary_to_match );

//int AM_force_into_b64( int level );

#endif

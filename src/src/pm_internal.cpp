/* -----------------------------------------------------------------------------

   PicoModel Library

   Copyright (c) 2002, Randy Reddig & seaw0lf
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the names of the copyright holders nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCidentAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   ----------------------------------------------------------------------------- */

/* todo:
 * - fix p->curLine for parser routines. increased twice
 */

#include <string.h>
#include <pmpmesh/pm_internal.hpp>

union floatSwapUnion
{
	float f;
	char c[4];
};

/* _pico_clone_alloc:
 *  handy function for quick string allocation/copy. it clones
 *  the given string and returns a pointer to the new allocated
 *  clone (which must be freed by caller of course) or returns
 *  nullptr on memory alloc or param errors. if 'size' is -1 the
 *  length of the input string is used, otherwise 'size' is used
 *  as custom clone size (the string is cropped to fit into mem
 *  if needed). -sea
 */
char *_pico_clone_alloc( const char *str ){
	char* cloned;

	/* sanity check */
	if ( str == nullptr ) {
		return nullptr;
	}

	/* allocate memory */
	cloned = reinterpret_cast<decltype(cloned)>(pmm::man.pp_m_new( strlen( str ) + 1 ));
	if ( cloned == nullptr ) {
		return nullptr;
	}

	/* copy input string to cloned string */
	strcpy( cloned, str );

	/* return ptr to cloned string */
	return cloned;
}

/* _pico_first_token:
 * trims everything after the first whitespace-delimited token
 */

void _pico_first_token( char *str ){
	if ( !str || !*str ) {
		return;
	}
	while ( *str && !isspace( *str ) )
		str++;
	*str = '\0';
}

/* _pico_strltrim:
 * left trims the given string -sea
 */
char *_pico_strltrim( char *str ){
	char *str1 = str, *str2 = str;

	while ( isspace( *str2 ) ) str2++;
	if ( str2 != str ) {
		while ( *str2 != '\0' ) /* fix: ydnar */
			*str1++ = *str2++;
	}
	return str;
}

/* _pico_strrtrim:
 * right trims the given string -sea
 */
char *_pico_strrtrim( char *str ){
	if ( str && *str ) {
		char *str1 = str;
		int allspace = 1;

		while ( *str1 )
		{
			if ( allspace && !isspace( *str1 ) ) {
				allspace = 0;
			}
			str1++;
		}
		if ( allspace ) {
			*str = '\0';
		}
		else {
			str1--;
			while ( ( isspace( *str1 ) ) && ( str1 >= str ) )
				*str1-- = '\0';
		}
	}
	return str;
}

/* _pico_strlwr:
 *  pico internal string-to-lower routine.
 */
char *_pico_strlwr( char *str ){
	char *cp;
	for ( cp = str; *cp; ++cp )
	{
		if ( 'A' <= *cp && *cp <= 'Z' ) {
			*cp += ( 'a' - 'A' );
		}
	}
	return str;
}

/* _pico_strchcount:
 *  counts how often the given char appears in str. -sea
 */
int _pico_strchcount( char *str, int ch ){
	int count = 0;
	while ( *str++ ) if ( *str == ch ) {
			count++;
		}
	return count;
}

void _pico_zero_bounds( pmm::vec3_t mins, pmm::vec3_t maxs ){
	int i;
	for ( i = 0; i < 3; i++ )
	{
		mins[i] = +999999;
		maxs[i] = -999999;
	}
}

void _pico_expand_bounds( pmm::vec3_t p, pmm::vec3_t mins, pmm::vec3_t maxs ){
	int i;
	for ( i = 0; i < 3; i++ )
	{
		float value = p[i];
		if ( value < mins[i] ) {
			mins[i] = value;
		}
		if ( value > maxs[i] ) {
			maxs[i] = value;
		}
	}
}

void _pico_zero_vec( pmm::vec3_t vec ){
	vec[ 0 ] = vec[ 1 ] = vec[ 2 ] = 0;
}

void _pico_zero_vec2( pmm::vec2_t vec ){
	vec[ 0 ] = vec[ 1 ] = 0;
}

void _pico_zero_vec4( pmm::vec4_t vec ){
	vec[ 0 ] = vec[ 1 ] = vec[ 2 ] = vec[ 3 ] = 0;
}

void _pico_set_vec( pmm::vec3_t v, float a, float b, float c ){
	v[ 0 ] = a;
	v[ 1 ] = b;
	v[ 2 ] = c;
}

void _pico_set_vec4( pmm::vec4_t v, float a, float b, float c, float d ){
	v[ 0 ] = a;
	v[ 1 ] = b;
	v[ 2 ] = c;
	v[ 3 ] = d;
}

void _pico_copy_vec( pmm::vec3_t src, pmm::vec3_t dest ){
	dest[ 0 ] = src[ 0 ];
	dest[ 1 ] = src[ 1 ];
	dest[ 2 ] = src[ 2 ];
}

void _pico_copy_vec2( pmm::vec2_t src, pmm::vec2_t dest ){
	dest[ 0 ] = src[ 0 ];
	dest[ 1 ] = src[ 1 ];
}

void _pico_copy_vec4( pmm::vec4_t src, pmm::vec4_t dest ){
	dest[ 0 ] = src[ 0 ];
	dest[ 1 ] = src[ 1 ];
	dest[ 2 ] = src[ 2 ];
	dest[ 3 ] = src[ 3 ];
}

/* ydnar */
pmm::vec_t _pico_normalize_vec( pmm::vec3_t vec ){
	double len, ilen;

	len = sqrt( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] + vec[ 2 ] * vec[ 2 ] );
	if ( len == 0.0 ) {
		return 0.0;
	}
	ilen = 1.0 / len;
	vec[ 0 ] *= (pmm::vec_t) ilen;
	vec[ 1 ] *= (pmm::vec_t) ilen;
	vec[ 2 ] *= (pmm::vec_t) ilen;
	return (pmm::vec_t) len;
}

void _pico_add_vec( pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t dest ){
	dest[ 0 ] = a[ 0 ] + b[ 0 ];
	dest[ 1 ] = a[ 1 ] + b[ 1 ];
	dest[ 2 ] = a[ 2 ] + b[ 2 ];
}

void _pico_subtract_vec( pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t dest ){
	dest[ 0 ] = a[ 0 ] - b[ 0 ];
	dest[ 1 ] = a[ 1 ] - b[ 1 ];
	dest[ 2 ] = a[ 2 ] - b[ 2 ];
}

void _pico_scale_vec( pmm::vec3_t v, float scale, pmm::vec3_t dest ){
	dest[ 0 ] = v[ 0 ] * scale;
	dest[ 1 ] = v[ 1 ] * scale;
	dest[ 2 ] = v[ 2 ] * scale;
}

void _pico_scale_vec4( pmm::vec4_t v, float scale, pmm::vec4_t dest ){
	dest[ 0 ] = v[ 0 ] * scale;
	dest[ 1 ] = v[ 1 ] * scale;
	dest[ 2 ] = v[ 2 ] * scale;
	dest[ 3 ] = v[ 3 ] * scale;
}

pmm::vec_t _pico_dot_vec( pmm::vec3_t a, pmm::vec3_t b ){
	return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}

void _pico_cross_vec( pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t dest ){
	dest[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
	dest[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
	dest[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}

pmm::vec_t _pico_calc_plane( pmm::vec4_t plane, pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t c ){
	pmm::vec3_t ba, ca;

	_pico_subtract_vec( b, a, ba );
	_pico_subtract_vec( c, a, ca );
	_pico_cross_vec( ca, ba, plane );
	plane[ 3 ] = _pico_dot_vec( a, plane );
	return _pico_normalize_vec( plane );
}

/* separate from _pico_set_vec4 */
void _pico_set_color( pmm::color_t c, int r, int g, int b, int a ){
	c[ 0 ] = r;
	c[ 1 ] = g;
	c[ 2 ] = b;
	c[ 3 ] = a;
}

void _pico_copy_color( pmm::color_t src, pmm::color_t dest ){
	dest[ 0 ] = src[ 0 ];
	dest[ 1 ] = src[ 1 ];
	dest[ 2 ] = src[ 2 ];
	dest[ 3 ] = src[ 3 ];
}

#if GDEF_ARCH_ENDIAN_BIG

int   _pico_big_long( int src ) { return src; }
short _pico_big_short( short src ) { return src; }
float _pico_big_float( float src ) { return src; }

int _pico_little_long( int src ){
	return ( ( src & 0xFF000000 ) >> 24 ) |
		   ( ( src & 0x00FF0000 ) >> 8 ) |
		   ( ( src & 0x0000FF00 ) << 8 ) |
		   ( ( src & 0x000000FF ) << 24 );
}

short _pico_little_short( short src ){
	return ( ( src & 0xFF00 ) >> 8 ) |
		   ( ( src & 0x00FF ) << 8 );
}

float _pico_little_float( float src ){
	floatSwapUnion in,out;
	in.f = src;
	out.c[ 0 ] = in.c[ 3 ];
	out.c[ 1 ] = in.c[ 2 ];
	out.c[ 2 ] = in.c[ 1 ];
	out.c[ 3 ] = in.c[ 0 ];
	return out.f;
}
#else /*__BIG_ENDIAN__*/

int   _pico_little_long( int src ) { return src; }
short _pico_little_short( short src ) { return src; }
float _pico_little_float( float src ) { return src; }

int _pico_big_long( int src ){
	return ( ( src & 0xFF000000 ) >> 24 ) |
		   ( ( src & 0x00FF0000 ) >> 8 ) |
		   ( ( src & 0x0000FF00 ) << 8 ) |
		   ( ( src & 0x000000FF ) << 24 );
}

short _pico_big_short( short src ){
	return ( ( src & 0xFF00 ) >> 8 ) |
		   ( ( src & 0x00FF ) << 8 );
}

float _pico_big_float( float src ){
	floatSwapUnion in,out;
	in.f = src;
	out.c[ 0 ] = in.c[ 3 ];
	out.c[ 1 ] = in.c[ 2 ];
	out.c[ 2 ] = in.c[ 1 ];
	out.c[ 3 ] = in.c[ 0 ];
	return out.f;
}
#endif /*__BIG_ENDIAN__*/

/* _pico_stristr:
 *  case-insensitive strstr. -sea
 */
const char *_pico_stristr( const char *str, const char *substr ){
	const pmm::size_type sublen = strlen( substr );
	while ( *str )
	{
		if ( !_pico_strnicmp( str,substr,sublen ) ) {
			break;
		}
		str++;
	}
	if ( !( *str ) ) {
		str = nullptr;
	}
	return str;
}

/*
   _pico_unixify()
   changes dos \ style path separators to /
 */

void _pico_unixify( char *path ){
	if ( path == nullptr ) {
		return;
	}
	while ( *path )
	{
		if ( *path == '\\' ) {
			*path = '/';
		}
		path++;
	}
}

/* _pico_nofname:
 *  removes file name portion from given file path and converts
 *  the directory separators to un*x style. returns 1 on success
 *  or 0 when 'destSize' was exceeded. -sea
 */
int _pico_nofname( const char *path, char *dest, int destSize ){
	int left  = destSize;
	char *temp  = dest;

	while ( ( *dest = *path ) != '\0' )
	{
		if ( *dest == '/' || *dest == '\\' ) {
			temp = ( dest + 1 );
			*dest = '/';
		}
		dest++; path++;

		if ( --left < 1 ) {
			*temp = '\0';
			return 0;
		}
	}
	*temp = '\0';
	return 1;
}

/* _pico_nopath:
 *  returns ptr to filename portion in given path or an empty
 *  string otherwise. given 'path' is not altered. -sea
 */
const char *_pico_nopath( const char *path ){
	const char *src;
	src = path + ( strlen( path ) - 1 );

	if ( path == nullptr ) {
		return "";
	}
	if ( !strchr( path,'/' ) && !strchr( path,'\\' ) ) {
		return ( path );
	}

	while ( ( src-- ) != path )
	{
		if ( *src == '/' || *src == '\\' ) {
			return ( ++src );
		}
	}
	return "";
}

/* _pico_setfext:
 *  sets/changes the file extension for the given filename
 *  or filepath's filename portion. the given 'path' *is*
 *  altered. leave 'ext' empty to remove extension. -sea
 */
char *_pico_setfext( char *path, const char *ext ){
	char *src;
	int remfext = 0;

	src = path + ( strlen( path ) - 1 );

	if ( ext == nullptr ) {
		ext = "";
	}
	if ( strlen( ext ) < 1 ) {
		remfext = 1;
	}
	if ( strlen( path ) < 1 ) {
		return path;
	}

	while ( ( src-- ) != path )
	{
		if ( *src == '/' || *src == '\\' ) {
			return path;
		}

		if ( *src == '.' ) {
			if ( remfext ) {
				*src = '\0';
				return path;
			}
			*( ++src ) = '\0';
			break;
		}
	}
	strcat( path,ext );
	return path;
}

/* _pico_getline:
 *  extracts one line from the given buffer and stores it in dest.
 *  returns -1 on error or the length of the line on success. i've
 *  removed string trimming here. this can be done manually by the
 *  calling func.
 */
int _pico_getline( char *buf, int bufsize, char *dest, int destsize ){
	int pos;

	/* check output */
	if ( dest == nullptr || destsize < 1 ) {
		return -1;
	}
	memset( dest,0,destsize );

	/* check input */
	if ( buf == nullptr || bufsize < 1 ) {
		return -1;
	}

	/* get next line */
	for ( pos = 0; pos < bufsize && pos < destsize; pos++ )
	{
		if ( buf[pos] == '\n' ) {
			pos++; break;
		}
		dest[pos] = buf[pos];
	}
	/* terminate dest and return */
	dest[pos] = '\0';
	return pos;
}

/* _pico_parse_skip_white:
 *  skips white spaces in current pico parser, sets *hasLFs
 *  to 1 if linefeeds were skipped, and either returns the
 *  parser's cursor pointer or nullptr on error. -sea
 */
void _pico_parse_skip_white( picoParser_t *p, int *hasLFs ){
	/* sanity checks */
	if ( p == nullptr || p->cursor == nullptr ) {
		return;
	}

	/* skin white spaces */
	while ( 1 )
	{
		/* sanity checks */
		if ( p->cursor <  p->buffer ||
			 p->cursor >= p->max ) {
			return;
		}
		/* break for chars other than white spaces */
		if ( *p->cursor >  0x20 ) {
			break;
		}
		if ( *p->cursor == 0x00 ) {
			return;
		}

		/* a bit of linefeed handling */
		if ( *p->cursor == '\n' ) {
			*hasLFs = 1;
			p->curLine++;
		}
		/* go to next character */
		p->cursor++;
	}
}

/* _pico_new_parser:
 *  allocates a new ascii parser object.
 */
picoParser_t *_pico_new_parser( const pmm::ub8_t *buffer, int bufSize ){
	picoParser_t *p;

	/* sanity check */
	if ( buffer == nullptr || bufSize <= 0 ) {
		return nullptr;
	}

	/* allocate reader */
	p = reinterpret_cast<decltype(p)>(pmm::man.pp_m_new( sizeof( picoParser_t ) ));
	if ( p == nullptr ) {
		return nullptr;
	}
	memset( p,0,sizeof( picoParser_t ) );

	/* allocate token space */
	p->tokenSize = 0;
	p->tokenMax = 1024;
	p->token = reinterpret_cast<decltype(p->token)>(pmm::man.pp_m_new( p->tokenMax ));
	if ( p->token == nullptr ) {
		pmm::man.pp_m_delete( p );
		return nullptr;
	}
	/* setup */
	p->buffer   = (const char *) buffer;
	p->cursor   = p->buffer;
	p->bufSize  = bufSize;
	p->max      = p->buffer + bufSize;
	p->curLine = 1; /* sea: new */

	/* return ptr to parser */
	return p;
}

/* _pico_free_parser:
 *  frees an existing pico parser object.
 */
void _pico_free_parser( picoParser_t *p ){
	/* sanity check */
	if ( p == nullptr ) {
		return;
	}

	/* free the parser */
	if ( p->token != nullptr ) {
		pmm::man.pp_m_delete( p->token );
	}
	pmm::man.pp_m_delete( p );
}

/* _pico_parse_ex:
 *  reads the next token from given pico parser object. if param
 * 'allowLFs' is 1 it will read beyond linefeeds and return 0 when
 *  the EOF is reached. if 'allowLFs' is 0 it will return 0 when
 *  the EOL is reached. if 'handleQuoted' is 1 the parser function
 *  will handle "quoted" strings and return the data between the
 *  quotes as token. returns 0 on end/error or 1 on success. -sea
 */
int _pico_parse_ex( picoParser_t *p, int allowLFs, int handleQuoted ){
	int hasLFs = 0;
	const char *old;

	/* sanity checks */
	if ( p == nullptr || p->buffer == nullptr ||
		 p->cursor <  p->buffer ||
		 p->cursor >= p->max ) {
		return 0;
	}
	/* clear parser token */
	p->tokenSize = 0;
	p->token[ 0 ] = '\0';
	old = p->cursor;

	/* skip whitespaces */
	while ( p->cursor < p->max && *p->cursor <= 32 )
	{
		if ( *p->cursor == '\n' ) {
			p->curLine++;
			hasLFs++;
		}
		p->cursor++;
	}
	/* return if we're not allowed to go beyond lfs */
	if ( ( hasLFs > 0 ) && !allowLFs ) {
		p->cursor = old;
		return 0;
	}
	/* get next quoted string */
	if ( *p->cursor == '\"' && handleQuoted ) {
		p->cursor++;
		while ( p->cursor < p->max && *p->cursor )
		{
			if ( *p->cursor == '\\' ) {
				if ( *( p->cursor + 1 ) == '"' ) {
					p->cursor++;
				}
				p->token[ p->tokenSize++ ] = *p->cursor++;
				continue;
			}
			else if ( *p->cursor == '\"' ) {
				p->cursor++;
				break;
			}
			else if ( *p->cursor == '\n' ) {
				p->curLine++;
			}
			p->token[ p->tokenSize++ ] = *p->cursor++;
		}
		/* terminate token */
		p->token[ p->tokenSize ] = '\0';
		return 1;
	}
	/* otherwise get next word */
	while ( p->cursor < p->max && *p->cursor > 32 )
	{
		if ( *p->cursor == '\n' ) {
			p->curLine++;
		}
		p->token[ p->tokenSize++ ] = *p->cursor++;
	}
	/* terminate token */
	p->token[ p->tokenSize ] = '\0';
	return 1;
}

/* _pico_parse_first:
 *  reads the first token from the next line and returns
 *  a pointer to it. returns nullptr on EOL or EOF. -sea
 */
char *_pico_parse_first( picoParser_t *p ){
	/* sanity check */
	if ( p == nullptr ) {
		return nullptr;
	}

	/* try to read next token (with lfs & quots) */
	if ( !_pico_parse_ex( p,1,1 ) ) {
		return nullptr;
	}

	/* return ptr to the token string */
	return p->token;
}

/* _pico_parse:
 *  reads the next token from the parser and returns a pointer
 *  to it. quoted strings are handled as usual. returns nullptr
 *  on EOL or EOF. -sea
 */
char *_pico_parse( picoParser_t *p, int allowLFs ){
	/* sanity check */
	if ( p == nullptr ) {
		return nullptr;
	}

	/* try to read next token (with quots) */
	if ( !_pico_parse_ex( p,allowLFs,1 ) ) {
		return nullptr;
	}

	/* return ptr to the token string */
	return p->token;
}

/* _pico_parse_skip_rest:
 *  skips the rest of the current line in parser.
 */
void _pico_parse_skip_rest( picoParser_t *p ){
	while ( _pico_parse_ex( p,0,0 ) ) ;
}

/* _pico_parse_skip_braced:
 *  parses/skips over a braced section. returns 1 on success
 *  or 0 on error (when there was no closing bracket and the
 *  end of buffer was reached or when the opening bracket was
 *  missing).
 */
int _pico_parse_skip_braced( picoParser_t *p ){
	int firstToken = 1;
	int level;

	/* sanity check */
	if ( p == nullptr ) {
		return 0;
	}

	/* set the initial level for parsing */
	level = 0;

	/* skip braced section */
	while ( 1 )
	{
		/* read next token (lfs allowed) */
		if ( !_pico_parse_ex( p,1,1 ) ) {
			/* end of parser buffer reached */
			return 0;
		}
		/* first token must be an opening bracket */
		if ( firstToken && p->token[0] != '{' ) {
			/* opening bracket missing */
			return 0;
		}
		/* we only check this once */
		firstToken = 0;

		/* update level */
		if ( p->token[1] == '\0' ) {
			if ( p->token[0] == '{' ) {
				level++;
			}
			if ( p->token[0] == '}' ) {
				level--;
			}
		}
		/* break if we're back at our starting level */
		if ( level == 0 ) {
			break;
		}
	}
	/* successfully skipped braced section */
	return 1;
}

int _pico_parse_check( picoParser_t *p, int allowLFs, const char *str ){
	if ( !_pico_parse_ex( p,allowLFs,1 ) ) {
		return 0;
	}
	if ( !strcmp( p->token,str ) ) {
		return 1;
	}
	return 0;
}

int _pico_parse_checki( picoParser_t *p, int allowLFs, const char *str ){
	if ( !_pico_parse_ex( p,allowLFs,1 ) ) {
		return 0;
	}
	if ( !_pico_stricmp( p->token,str ) ) {
		return 1;
	}
	return 0;
}

int _pico_parse_int( picoParser_t *p, int *out ){
	char *token;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* get token and turn it into an integer */
	*out = 0;
	token = _pico_parse( p,0 );
	if ( token == nullptr ) {
		return 0;
	}
	*out = atoi( token );

	/* success */
	return 1;
}

int _pico_parse_int_def( picoParser_t *p, int *out, int def ){
	char *token;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* get token and turn it into an integer */
	*out = def;
	token = _pico_parse( p,0 );
	if ( token == nullptr ) {
		return 0;
	}
	*out = atoi( token );

	/* success */
	return 1;
}

int _pico_parse_float( picoParser_t *p, float *out ){
	char *token;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* get token and turn it into a float */
	*out = 0.0f;
	token = _pico_parse( p,0 );
	if ( token == nullptr ) {
		return 0;
	}
	*out = (float) atof( token );

	/* success */
	return 1;
}

int _pico_parse_float_def( picoParser_t *p, float *out, float def ){
	char *token;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* get token and turn it into a float */
	*out = def;
	token = _pico_parse( p,0 );
	if ( token == nullptr ) {
		return 0;
	}
	*out = (float) atof( token );

	/* success */
	return 1;
}

int _pico_parse_vec( picoParser_t *p, pmm::vec3_t out ){
	char *token;
	int i;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* zero out outination vector */
	_pico_zero_vec( out );

	/* parse three vector components */
	for ( i = 0; i < 3; i++ )
	{
		token = _pico_parse( p,0 );
		if ( token == nullptr ) {
			_pico_zero_vec( out );
			return 0;
		}
		out[ i ] = (float) atof( token );
	}
	/* success */
	return 1;
}

int _pico_parse_vec_def( picoParser_t *p, pmm::vec3_t out, pmm::vec3_t def ){
	char *token;
	int i;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* assign default vector value */
	_pico_copy_vec( def,out );

	/* parse three vector components */
	for ( i = 0; i < 3; i++ )
	{
		token = _pico_parse( p,0 );
		if ( token == nullptr ) {
			_pico_copy_vec( def,out );
			return 0;
		}
		out[ i ] = (float) atof( token );
	}
	/* success */
	return 1;
}

int _pico_parse_vec2( picoParser_t *p, pmm::vec2_t out ){
	char *token;
	int i;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* zero out outination vector */
	_pico_zero_vec2( out );

	/* parse two vector components */
	for ( i = 0; i < 2; i++ )
	{
		token = _pico_parse( p,0 );
		if ( token == nullptr ) {
			_pico_zero_vec2( out );
			return 0;
		}
		out[ i ] = (float) atof( token );
	}
	/* success */
	return 1;
}

int _pico_parse_vec2_def( picoParser_t *p, pmm::vec2_t out, pmm::vec2_t def ){
	char *token;
	int i;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* assign default vector value */
	_pico_copy_vec2( def,out );

	/* parse two vector components */
	for ( i = 0; i < 2; i++ )
	{
		token = _pico_parse( p,0 );
		if ( token == nullptr ) {
			_pico_copy_vec2( def,out );
			return 0;
		}
		out[ i ] = (float) atof( token );
	}
	/* success */
	return 1;
}

int _pico_parse_vec4( picoParser_t *p, pmm::vec4_t out ){
	char *token;
	int i;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* zero out outination vector */
	_pico_zero_vec4( out );

	/* parse four vector components */
	for ( i = 0; i < 4; i++ )
	{
		token = _pico_parse( p,0 );
		if ( token == nullptr ) {
			_pico_zero_vec4( out );
			return 0;
		}
		out[ i ] = (float) atof( token );
	}
	/* success */
	return 1;
}

int _pico_parse_vec4_def( picoParser_t *p, pmm::vec4_t out, pmm::vec4_t def ){
	char *token;
	int i;

	/* sanity checks */
	if ( p == nullptr || out == nullptr ) {
		return 0;
	}

	/* assign default vector value */
	_pico_copy_vec4( def,out );

	/* parse four vector components */
	for ( i = 0; i < 4; i++ )
	{
		token = _pico_parse( p,0 );
		if ( token == nullptr ) {
			_pico_copy_vec4( def,out );
			return 0;
		}
		out[ i ] = (float) atof( token );
	}
	/* success */
	return 1;
}

/* _pico_new_memstream:
 *  allocates a new memorystream object.
 */
picoMemStream_t *_pico_new_memstream( const pmm::ub8_t *buffer, int bufSize ){
	picoMemStream_t *s;

	/* sanity check */
	if ( buffer == nullptr || bufSize <= 0 ) {
		return nullptr;
	}

	/* allocate stream */
	s = reinterpret_cast<decltype(s)>(pmm::man.pp_m_new( sizeof( picoMemStream_t ) ));
	if ( s == nullptr ) {
		return nullptr;
	}
	memset( s,0,sizeof( picoMemStream_t ) );

	/* setup */
	s->buffer   = buffer;
	s->curPos   = buffer;
	s->bufSize  = bufSize;
	s->flag     = 0;

	/* return ptr to stream */
	return s;
}

/* _pico_free_memstream:
 *  frees an existing pico memorystream object.
 */
void _pico_free_memstream( picoMemStream_t *s ){
	/* sanity check */
	if ( s == nullptr ) {
		return;
	}

	/* free the stream */
	pmm::man.pp_m_delete( s );
}

/* _pico_memstream_read:
 *  reads data from a pico memorystream into a buffer.
 */
int _pico_memstream_read( picoMemStream_t *s, void *buffer, int len ){
	int ret = 1;

	/* sanity checks */
	if ( s == nullptr || buffer == nullptr ) {
		return 0;
	}

	if ( s->curPos + len > s->buffer + s->bufSize ) {
		s->flag |= PICO_IOEOF;
		len = s->buffer + s->bufSize - s->curPos;
		ret = 0;
	}

	/* read the data */
	memcpy( buffer, s->curPos, len );
	s->curPos += len;
	return ret;
}

/* _pico_memstream_read:
 *  reads a character from a pico memorystream
 */
int _pico_memstream_getc( picoMemStream_t *s ){
	int c = 0;

	/* sanity check */
	if ( s == nullptr ) {
		return -1;
	}

	/* read the character */
	if ( _pico_memstream_read( s, &c, 1 ) == 0 ) {
		return -1;
	}

	return c;
}

/* _pico_memstream_seek:
 *  sets the current read position to a different location
 */
int _pico_memstream_seek( picoMemStream_t *s, long offset, int origin ){
	int overflow;

	/* sanity check */
	if ( s == nullptr ) {
		return -1;
	}

	if ( origin == PICO_SEEK_SET ) {
		s->curPos = s->buffer + offset;
		overflow = s->curPos - ( s->buffer + s->bufSize );
		if ( overflow > 0 ) {
			s->curPos = s->buffer + s->bufSize;
			return offset - overflow;
		}
		return 0;
	}
	else if ( origin == PICO_SEEK_CUR ) {
		s->curPos += offset;
		overflow = s->curPos - ( s->buffer + s->bufSize );
		if ( overflow > 0 ) {
			s->curPos = s->buffer + s->bufSize;
			return offset - overflow;
		}
		return 0;
	}
	else if ( origin == PICO_SEEK_END ) {
		s->curPos = ( s->buffer + s->bufSize ) - offset;
		overflow = s->buffer - s->curPos;
		if ( overflow > 0 ) {
			s->curPos = s->buffer;
			return offset - overflow;
		}
		return 0;
	}

	return -1;
}

/* _pico_memstream_tell:
 *  returns the current read position in the pico memorystream
 */
long _pico_memstream_tell( picoMemStream_t *s ){
	/* sanity check */
	if ( s == nullptr ) {
		return -1;
	}

	return s->curPos - s->buffer;
}

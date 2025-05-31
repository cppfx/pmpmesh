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

#pragma once

#include <pmpmesh/platfdefs.hpp>

/* dependencies */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <pmpmesh/pmpmesh.hpp>


/* os dependent replacements */
#if GDEF_OS_WINDOWS
	#define _pico_stricmp stricmp
	#define _pico_strnicmp strnicmp
#else
	#define _pico_stricmp strcasecmp
	#define _pico_strnicmp strncasecmp
#endif


/* constants */
#define PICO_PI 3.14159265358979323846

#define PICO_SEEK_SET    0
#define PICO_SEEK_CUR    1
#define PICO_SEEK_END    2

#define PICO_IOEOF  1
#define PICO_IOERR  2


/* types */
class picoParser_t
{
public:
	const char *buffer;
	int bufSize;
	char *token;
	int tokenSize;
	int tokenMax;
	const char *cursor;
	const char *max;
	int curLine;
};

class picoMemStream_t
{
public:
	const pmm::ub8_t *buffer;
	int bufSize;
	const pmm::ub8_t *curPos;
	int flag;
};


/* variables */
extern const pmm::module_t   *picoModules[];

extern void ( *_pico_ptr_print )( int, const char* );



/* prototypes */

/* memory */
char            *_pico_clone_alloc( const char *str );

/* strings */
void            _pico_first_token( char *str );
char            *_pico_strltrim( char *str );
char            *_pico_strrtrim( char *str );
int             _pico_strchcount( char *str, int ch );
void            _pico_printf( int level, const char *format, ... );
const char      *_pico_stristr( const char *str, const char *substr );
void            _pico_unixify( char *path );
int             _pico_nofname( const char *path, char *dest, int destSize );
const char *_pico_nopath( const char *path );
char            *_pico_setfext( char *path, const char *ext );
int             _pico_getline( char *buf, int bufsize, char *dest, int destsize );
char            *_pico_strlwr( char *str );

/* vectors */
void            _pico_zero_bounds( pmm::vec3_t mins, pmm::vec3_t maxs );
void            _pico_expand_bounds( pmm::vec3_t p, pmm::vec3_t mins, pmm::vec3_t maxs );
void            _pico_zero_vec( pmm::vec3_t vec );
void            _pico_zero_vec2( pmm::vec2_t vec );
void            _pico_zero_vec4( pmm::vec4_t vec );
void            _pico_set_vec( pmm::vec3_t v, float a, float b, float c );
void            _pico_set_vec4( pmm::vec4_t v, float a, float b, float c, float d );
void            _pico_set_color( pmm::color_t c, int r, int g, int b, int a );
void            _pico_copy_color( pmm::color_t src, pmm::color_t dest );
void            _pico_copy_vec( pmm::vec3_t src, pmm::vec3_t dest );
void            _pico_copy_vec2( pmm::vec2_t src, pmm::vec2_t dest );
pmm::vec_t       _pico_normalize_vec( pmm::vec3_t vec );
void            _pico_add_vec( pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t dest );
void            _pico_subtract_vec( pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t dest );
pmm::vec_t       _pico_dot_vec( pmm::vec3_t a, pmm::vec3_t b );
void            _pico_cross_vec( pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t dest );
pmm::vec_t       _pico_calc_plane( pmm::vec4_t plane, pmm::vec3_t a, pmm::vec3_t b, pmm::vec3_t c );
void            _pico_scale_vec( pmm::vec3_t v, float scale, pmm::vec3_t dest );
void            _pico_scale_vec4( pmm::vec4_t v, float scale, pmm::vec4_t dest );

/* endian */
int             _pico_big_long( int src );
short           _pico_big_short( short src );
float           _pico_big_float( float src );

int             _pico_little_long( int src );
short           _pico_little_short( short src );
float           _pico_little_float( float src );

/* pico ascii parser */
picoParser_t    *_pico_new_parser( const pmm::ub8_t *buffer, int bufSize );
void            _pico_free_parser( picoParser_t *p );
int             _pico_parse_ex( picoParser_t *p, int allowLFs, int handleQuoted );
char            *_pico_parse_first( picoParser_t *p );
char            *_pico_parse( picoParser_t *p, int allowLFs );
void            _pico_parse_skip_rest( picoParser_t *p );
int             _pico_parse_skip_braced( picoParser_t *p );
int             _pico_parse_check( picoParser_t *p, int allowLFs, const char *str );
int             _pico_parse_checki( picoParser_t *p, int allowLFs, const char *str );
int             _pico_parse_int( picoParser_t *p, int *out );
int             _pico_parse_int_def( picoParser_t *p, int *out, int def );
int             _pico_parse_float( picoParser_t *p, float *out );
int             _pico_parse_float_def( picoParser_t *p, float *out, float def );
int             _pico_parse_vec( picoParser_t *p, pmm::vec3_t out );
int             _pico_parse_vec_def( picoParser_t *p, pmm::vec3_t out, pmm::vec3_t def );
int             _pico_parse_vec2( picoParser_t *p, pmm::vec2_t out );
int             _pico_parse_vec2_def( picoParser_t *p, pmm::vec2_t out, pmm::vec2_t def );
int             _pico_parse_vec4( picoParser_t *p, pmm::vec4_t out );
int             _pico_parse_vec4_def( picoParser_t *p, pmm::vec4_t out, pmm::vec4_t def );

/* pico memory stream */
picoMemStream_t *_pico_new_memstream( const pmm::ub8_t *buffer, int bufSize );
void            _pico_free_memstream( picoMemStream_t *s );
int             _pico_memstream_read( picoMemStream_t *s, void *buffer, int len );
int             _pico_memstream_getc( picoMemStream_t *s );
int             _pico_memstream_seek( picoMemStream_t *s, long offset, int origin );
long            _pico_memstream_tell( picoMemStream_t *s );
#define         _pico_memstream_eof( _pico_memstream )      ( ( _pico_memstream )->flag & PICO_IOEOF )
#define         _pico_memstream_error( _pico_memstream )    ( ( _pico_memstream )->flag & PICO_IOERR )

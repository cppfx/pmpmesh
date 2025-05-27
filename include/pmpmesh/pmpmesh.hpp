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

namespace pmm
{

/* version */
#define	PICOMODEL_version 0.8.20

/* constants */
#define PICO_GROW_SHADERS       16
#define PICO_GROW_SURFACES      16
#define PICO_GROW_VERTEXES      1024
#define PICO_GROW_INDEXES       1024
#define PICO_GROW_ARRAYS        8
#define PICO_GROW_FACES         256
#define PICO_MAX_SPECIAL        8
#define PICO_MAX_DEFAULT_EXTS   4       /* max default extensions per module */

/* types */
using byte_t = unsigned char;
using vec_t = float;
using vec2_t = float[2];
using vec3_t = float[3];
using vec4_t = float[4];
using color_t = pmm::byte_t[4];
using index_t = int;
using std_size_t = decltype(sizeof 0);

enum surface_type
{
	st_bad,
	st_triangles,
	st_patch
};

enum print_level
{
	pl_normal,
	pl_verbose,
	pl_warning,
	pl_error,
	pl_fatal
};

class surface_t;
class shader_t;
class model_t;
class module_t;

class surface_t
{
public:
	void                        *data;

	pmm::model_t                 *model;     /* owner model */

	pmm::surface_type type;
	char                        *name;      /* sea: surface name */
	pmm::shader_t                *shader;    /* ydnar: changed to ptr */

	int numVertexes, maxVertexes;
	pmm::vec3_t                  *xyz;
	pmm::vec3_t                  *normal;
	pmm::index_t                 *smoothingGroup;

	int numSTArrays, maxSTArrays;
	pmm::vec2_t                  **st;

	int numColorArrays, maxColorArrays;
	pmm::color_t                 **color;

	int numIndexes, maxIndexes;
	pmm::index_t                 *index;

	int numFaceNormals, maxFaceNormals;
	pmm::vec3_t                  *faceNormal;

	int special[ PICO_MAX_SPECIAL ];
};

/* seaw0lf */
class shader_t
{
public:
	pmm::model_t                 *model;         /* owner model */

	char                        *name;          /* shader name */
	char                        *mapName;       /* shader file name (name of diffuse texturemap) */
	pmm::color_t ambientColor;                   /* ambient color of mesh (rgba) */
	pmm::color_t diffuseColor;                   /* diffuse color of mesh (rgba) */
	pmm::color_t specularColor;                  /* specular color of mesh (rgba) */
	float transparency;                         /* transparency (0..1; 1 = 100% transparent) */
	float shininess;                            /* shininess (0..128; 128 = 100% shiny) */
};

class model_t
{
public:
	void                        *data;
	char                        *name;          /* model name */
	char                        *fileName;      /* sea: model file name */
	int frameNum;                               /* sea: renamed to frameNum */
	int numFrames;                              /* sea: number of frames */
	pmm::vec3_t mins;
	pmm::vec3_t maxs;

	int num_shaders, maxShaders;
	pmm::shader_t                **shader;

	int num_surfaces, maxSurfaces;
	pmm::surface_t               **surface;

	const pmm::module_t          *module;        /* sea */
};

/* seaw0lf */
/* return codes used by the validation callbacks; pmv is short */
/* for 'pico module validation'. everything >pmm::pmv_ok means */
/* that there was an error. */
enum
{
	pmv_ok,            /* file valid */
	pmv_error,         /* file not valid */
	pmv_error_ident,   /* unknown file magic (aka ident) */
	pmv_error_version, /* unsupported file version */
	pmv_error_size,    /* file size error */
	pmv_error_memory,  /* out of memory error */
};

/* convenience (makes it easy to add new params to the callbacks) */
#define PM_PARAMS_CANLOAD \
	const char *fileName, const void *buffer, int bufSize

#define PM_PARAMS_LOAD \
	const char *fileName, int frameNum, const void *buffer, int bufSize

#define PM_PARAMS_CANSAVE \
	void

#define PM_PARAMS_SAVE \
	const char *fileName, pmm::model_t * model

/* pico file format module class */
class module_t
{
public:
	const char         *version;          /* internal module version (e.g. '1.5-b2') */

	const char         *displayName;      /* string used to display in guis, etc. */
	const char         *authorName;       /* author name (eg. 'My Real Name') */
	const char         *copyright;        /* copyright year and holder (eg. '2002 My Company') */

	const char         *defaultExts[ PICO_MAX_DEFAULT_EXTS ];  /* default file extensions used by this file type */
	int ( *canload )( PM_PARAMS_CANLOAD );     /* checks whether module can load given file (returns pmvR_*) */
	pmm::model_t  *( *load )( PM_PARAMS_LOAD );  /* parses model file data */
	int ( *cansave )( PM_PARAMS_CANSAVE );     /* checks whether module can save (returns 1 or 0 and might spit out a message) */
	int ( *save )( PM_PARAMS_SAVE );           /* saves a pico model in module's native model format */
};

/* general functions */
int pp_init( void );
void pp_shutdown( void );
int pp_error( void );

void pp_set_malloc_func( void *( *func )( pmm::std_size_t ) );
void pp_set_free_func( void ( *func )( void* ) );
void pp_set_load_file_func( void ( *func )( const char*, unsigned char**, int* ) );
void pp_set_free_file_func( void ( *func )( void* ) );
void pp_set_print_func( void ( *func )( int, const char* ) );

const pmm::module_t ** pp_module_list( int *numModules );

pmm::model_t * pp_load_model( const char *name, int frameNum );

//										(inputStream, buffer, length)
using pp_input_stream_read_func = pmm::std_size_t (*)(void *, unsigned char *, pmm::std_size_t);

pmm::model_t* pp_module_load_model_stream(
	const pmm::module_t * module,
	void* inputStream,
	pmm::pp_input_stream_read_func inputStreamRead,
	pmm::std_size_t streamLength,
	int frameNum,
	const char *fileName
);

/* model functions */
pmm::model_t * pp_new_model( void );
void pp_free_model(pmm::model_t * model);
int pp_adjust_model(pmm::model_t * model, int num_shaders, int num_surfaces);

/* shader functions */
pmm::shader_t * pp_new_shader( pmm::model_t *model );
void pp_free_shader( pmm::shader_t *shader );
pmm::shader_t * pp_find_shader( pmm::model_t *model, char *name, int caseSensitive );

/* surface functions */
pmm::surface_t * pp_new_surface( pmm::model_t *model );
void pp_free_surface( pmm::surface_t *surface );
pmm::surface_t * pp_find_surface( pmm::model_t *model, char *name, int caseSensitive );
int pp_adjust_surface( pmm::surface_t *surface, int numVertexes, int numSTArrays, int numColorArrays, int numIndexes, int numFaceNormals );

/* setter functions */
void                        pp_set_model_name( pmm::model_t *model, const char *name );
void                        pp_set_model_file_name( pmm::model_t *model, const char *fileName );
void                        pp_set_model_frame_num( pmm::model_t *model, int frameNum );
void                        pp_set_model_num_frames( pmm::model_t *model, int numFrames );
void                        pp_set_model_data( pmm::model_t *model, void *data );

void                        pp_set_shader_name( pmm::shader_t *shader, char *name );
void                        pp_set_shader_map_name( pmm::shader_t *shader, char *mapName );
void                        pp_set_shader_ambient_color( pmm::shader_t *shader, pmm::color_t color );
void                        pp_set_shader_diffuse_color( pmm::shader_t *shader, pmm::color_t color );
void                        pp_set_shader_specular_color( pmm::shader_t *shader, pmm::color_t color );
void                        pp_set_shader_transparency( pmm::shader_t *shader, float value );
void                        pp_set_shader_shininess( pmm::shader_t *shader, float value );

void                        pp_set_surface_data( pmm::surface_t *surface, void *data );
void                        pp_set_surface_type( pmm::surface_t *surface, pmm::surface_type type );
void                        pp_set_surface_name( pmm::surface_t *surface, const char *name );
void                        pp_set_surface_shader( pmm::surface_t *surface, pmm::shader_t *shader );
void                        pp_set_surface_xyz( pmm::surface_t *surface, int num, pmm::vec3_t xyz );
void                        pp_set_surface_normal( pmm::surface_t *surface, int num, pmm::vec3_t normal );
void                        pp_set_surface_st( pmm::surface_t *surface, int array, int num, pmm::vec2_t st );
void                        pp_set_surface_color( pmm::surface_t *surface, int array, int num, pmm::color_t color );
void                        pp_set_surface_index( pmm::surface_t *surface, int num, pmm::index_t index );
void                        pp_set_surface_indices( pmm::surface_t *surface, int num, pmm::index_t *index, int count );
void                        pp_set_face_normal( pmm::surface_t *surface, int num, pmm::vec3_t normal );
void                        pp_set_surface_special( pmm::surface_t *surface, int num, int special );
void                        pp_set_surface_smoothing_group( pmm::surface_t *surface, int num, pmm::index_t smoothingGroup );

/* getter functions */
char                        *pp_get_model_name( pmm::model_t *model );
char                        *pp_get_model_file_name( pmm::model_t *model );
int                         pp_get_model_frame_num( pmm::model_t *model );
int                         pp_get_model_num_frames( pmm::model_t *model );
void                        *pp_get_model_data( pmm::model_t *model );
int                         pp_get_model_num_shaders( pmm::model_t *model );
pmm::shader_t                *pp_get_model_shader( pmm::model_t *model, int num ); /* sea */
int                         pp_get_model_num_surfaces( pmm::model_t *model );
pmm::surface_t               *pp_get_model_surface( pmm::model_t *model, int num );
int                         pp_get_model_total_vertices( pmm::model_t *model );
int                         pp_get_model_total_indices( pmm::model_t *model );

char                        *pp_get_shader_name( pmm::shader_t *shader );
char                        *pp_get_shader_map_name( pmm::shader_t *shader );
pmm::byte_t                  *pp_get_shader_ambient_color( pmm::shader_t *shader );
pmm::byte_t                  *pp_get_shader_diffuse_color( pmm::shader_t *shader );
pmm::byte_t                  *pp_get_shader_specular_color( pmm::shader_t *shader );
float                       pp_get_shader_transparency( pmm::shader_t *shader );
float                       pp_get_shader_shininess( pmm::shader_t *shader );

void                        *pp_get_surface_data( pmm::surface_t *surface );
char                        *pp_get_surface_name( pmm::surface_t *surface );      /* sea */
pmm::surface_type           pp_get_surface_type( pmm::surface_t *surface );
pmm::shader_t                *pp_get_surface_shader( pmm::surface_t *surface );    /* sea */
int                         pp_get_surface_num_vertices( pmm::surface_t *surface );
pmm::vec_t                   *pp_get_surface_xyz( pmm::surface_t *surface, int num );
pmm::vec_t                   *pp_get_surface_normal( pmm::surface_t *surface, int num );
pmm::vec_t                   *pp_get_surface_st( pmm::surface_t *surface, int array, int num );
pmm::byte_t                  *pp_get_surface_color( pmm::surface_t *surface, int array, int num );
int                         pp_get_surface_num_indices( pmm::surface_t *surface );
pmm::index_t                 pp_get_surface_index( pmm::surface_t *surface, int num );
pmm::index_t                 *pp_get_surface_indices( pmm::surface_t *surface, int num );
pmm::vec_t                   *pp_get_face_normal( pmm::surface_t *surface, int num );
int                         pp_get_surface_special( pmm::surface_t *surface, int num );

/* hashtable related functions */
class pp_vertex_combination_data_t
{
public:
	pmm::vec3_t xyz, normal;
	pmm::vec2_t st;
	pmm::color_t color;
};

class pp_vertex_comnination_hash_t
{
public:
	pmm::pp_vertex_combination_data_t vcd;
	pmm::index_t index;

	void                        *data;

	pmm::pp_vertex_comnination_hash_t  *next;
};

int pp_get_hash_table_size( void );
unsigned int pp_vertex_coord_generate_hash( pmm::vec3_t xyz );

pmm::pp_vertex_comnination_hash_t ** pp_new_vertex_combination_hash_table();
void pp_free_vertex_combination_hash_table( pmm::pp_vertex_comnination_hash_t **hashTable );

pp_vertex_comnination_hash_t *
	pp_find_vertex_combination_in_hash_table(
		pmm::pp_vertex_comnination_hash_t **hashTable,
		pmm::vec3_t xyz,
		pmm::vec3_t normal,
		pmm::vec3_t st,
		pmm::color_t color
	);

pp_vertex_comnination_hash_t *
	pp_add_vertex_combination_to_hash_table(
		pmm::pp_vertex_comnination_hash_t **hashTable,
		pmm::vec3_t xyz,
		pmm::vec3_t normal,
		pmm::vec3_t st,
		pmm::color_t color,
		pmm::index_t index
	);

/* specialized functions */
int pp_find_surface_vertex_num(
	pmm::surface_t *surface,
	pmm::vec3_t xyz,
	pmm::vec3_t normal,
	int numSTs,
	pmm::vec2_t *st,
	int numColors,
	pmm::color_t *color,
	pmm::index_t smoothingGroup
);

void pp_fix_surface_normals( pmm::surface_t *surface );
int pp_remap_model( pmm::model_t *model, char *remapFile );

void pp_add_triangle_to_model( pmm::model_t *model, pmm::vec3_t** xyz, pmm::vec3_t** normals, int numSTs, pmm::vec2_t **st, int numColors, pmm::color_t **colors, pmm::shader_t* shader, const char *name, pmm::index_t* smoothingGroup );

}	// namespace pmm

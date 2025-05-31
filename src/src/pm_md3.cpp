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

/* dependencies */
#include <pmpmesh/pm_internal.hpp>

/* md3 model format */
const char *MD3_MAGIC = "IDP3";
const int MD3_version = 15;

/* md3 vertex scale */
const float MD3_SCALE = ( 1.0f / 64.0f );

/* md3 model frame information */
class md3Frame_t
{
public:
	float bounds[ 2 ][ 3 ];
	float localOrigin[ 3 ];
	float radius;
	char creator[ 16 ];
};

/* md3 model tag information */
class md3Tag_t
{
public:
	char name[ 64 ];
	float origin[ 3 ];
	float axis[ 3 ][ 3 ];
};

/* md3 surface md3 (one object mesh) */
class md3Surface_t
{
public:
	char magic[ 4 ];
	char name[ 64 ];            /* polyset name */
	int flags;
	int numFrames;              /* all model surfaces should have the same */
	int num_shaders;             /* all model surfaces should have the same */
	int numVerts;
	int numTriangles;
	int ofsTriangles;
	int ofsShaders;             /* offset from start of md3Surface_t */
	int ofsSt;                  /* texture coords are common for all frames */
	int ofsVertexes;            /* numVerts * numFrames */
	int ofsEnd;                 /* next surface follows */
};

class md3Shader_t
{
public:
	char name[ 64 ];
	int shaderIndex;            /* for ingame use */
};

class md3Triangle_t
{
public:
	int indices[ 3 ];
};

class md3TexCoord_t
{
public:
	float st[ 2 ];
};

class md3Vertex_t
{
public:
	short xyz[ 3 ];
	short normal;
};


/* md3 model file md3 class  */
class md3_t
{
public:
	char magic[ 4 ];            /* MD3_MAGIC */
	int version;
	char name[ 64 ];            /* model name */
	int flags;
	int numFrames;
	int numTags;
	int num_surfaces;
	int numSkins;               /* number of skins for the mesh */
	int ofsFrames;              /* offset for first frame */
	int ofsTags;                /* numFrames * numTags */
	int ofsSurfaces;            /* first surface, others follow */
	int ofsEnd;                 /* end of file */
};




/*
   _md3_canload()
   validates a quake3 arena md3 model file. btw, i use the
   preceding underscore cause it's a static func referenced
   by one class only.
 */

static int _md3_canload( PM_PARAMS_CANLOAD ){
	const md3_t *md3;


	/* sanity check */
	if ( (pmm::std_size_t) bufSize < ( sizeof( *md3 ) * 2 ) ) {
		return pmm::pmv_error_size;
	}

	/* set as md3 */
	md3 = (const md3_t*) buffer;

	/* check md3 magic */
	if ( *( (const int*) md3->magic ) != *( (const int*) MD3_MAGIC ) ) {
		return pmm::pmv_error_ident;
	}

	/* check md3 version */
	if ( _pico_little_long( md3->version ) != MD3_version ) {
		return pmm::pmv_error_version;
	}

	/* file seems to be a valid md3 */
	return pmm::pmv_ok;
}



/*
   _md3_load()
   loads a quake3 arena md3 model file.
 */

static pmm::model_t *_md3_load( PM_PARAMS_LOAD ){
	int i, j;
	pmm::ub8_t      *bb, *bb0;
	md3_t           *md3;
	md3Surface_t    *surface;
	md3Shader_t     *shader;
	md3TexCoord_t   *texCoord;
	md3Frame_t      *frame;
	md3Triangle_t   *triangle;
	md3Vertex_t     *vertex;
	double lat, lng;

	pmm::model_t     *picoModel;
	pmm::surface_t   *picoSurface;
	pmm::shader_t    *picoShader;
	pmm::vec3_t xyz, normal;
	pmm::vec2_t st;
	pmm::color_t color;


	/* -------------------------------------------------
	   md3 loading
	   ------------------------------------------------- */


	/* set as md3 */
	bb0 = bb = (pmm::ub8_t*) _pico_alloc( bufSize );
	memcpy( bb, buffer, bufSize );
	md3 = (md3_t*) bb;

	/* check ident and version */
	if ( *( (int*) md3->magic ) != *( (int*) MD3_MAGIC ) || _pico_little_long( md3->version ) != MD3_version ) {
		/* not an md3 file (todo: set error) */
		_pico_free( bb0 );
		return nullptr;
	}

	/* swap md3; sea: swaps fixed */
	md3->version = _pico_little_long( md3->version );
	md3->numFrames = _pico_little_long( md3->numFrames );
	md3->numTags = _pico_little_long( md3->numTags );
	md3->num_surfaces = _pico_little_long( md3->num_surfaces );
	md3->numSkins = _pico_little_long( md3->numSkins );
	md3->ofsFrames = _pico_little_long( md3->ofsFrames );
	md3->ofsTags = _pico_little_long( md3->ofsTags );
	md3->ofsSurfaces = _pico_little_long( md3->ofsSurfaces );
	md3->ofsEnd = _pico_little_long( md3->ofsEnd );

	/* do frame check */
	if ( md3->numFrames < 1 ) {
		_pico_printf( pmm::pl_error, "MD3 with 0 frames" );
		_pico_free( bb0 );
		return nullptr;
	}

	if ( frameNum < 0 || frameNum >= md3->numFrames ) {
		_pico_printf( pmm::pl_error, "Invalid or out-of-range MD3 frame specified" );
		_pico_free( bb0 );
		return nullptr;
	}

	/* swap frames */
	frame = (md3Frame_t*) ( bb + md3->ofsFrames );
	for ( i = 0; i < md3->numFrames; i++, frame++ )
	{
		frame->radius = _pico_little_float( frame->radius );
		for ( j = 0; j < 3; j++ )
		{
			frame->bounds[ 0 ][ j ] = _pico_little_float( frame->bounds[ 0 ][ j ] );
			frame->bounds[ 1 ][ j ] = _pico_little_float( frame->bounds[ 1 ][ j ] );
			frame->localOrigin[ j ] = _pico_little_float( frame->localOrigin[ j ] );
		}
	}

	/* swap surfaces */
	surface = (md3Surface_t*) ( bb + md3->ofsSurfaces );
	for ( i = 0; i < md3->num_surfaces; i++ )
	{
		/* swap surface md3; sea: swaps fixed */
		surface->flags = _pico_little_long( surface->flags );
		surface->numFrames = _pico_little_long( surface->numFrames );
		surface->num_shaders = _pico_little_long( surface->num_shaders );
		surface->numTriangles = _pico_little_long( surface->numTriangles );
		surface->ofsTriangles = _pico_little_long( surface->ofsTriangles );
		surface->numVerts = _pico_little_long( surface->numVerts );
		surface->ofsShaders = _pico_little_long( surface->ofsShaders );
		surface->ofsSt = _pico_little_long( surface->ofsSt );
		surface->ofsVertexes = _pico_little_long( surface->ofsVertexes );
		surface->ofsEnd = _pico_little_long( surface->ofsEnd );

		/* swap triangles */
		triangle = (md3Triangle_t*) ( (pmm::ub8_t*) surface + surface->ofsTriangles );
		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			/* sea: swaps fixed */
			triangle->indices[ 0 ] = _pico_little_long( triangle->indices[ 0 ] );
			triangle->indices[ 1 ] = _pico_little_long( triangle->indices[ 1 ] );
			triangle->indices[ 2 ] = _pico_little_long( triangle->indices[ 2 ] );
		}

		/* swap st coords */
		texCoord = (md3TexCoord_t*) ( (pmm::ub8_t*) surface + surface->ofsSt );
		for ( j = 0; j < surface->numVerts; j++, texCoord++ )
		{
			texCoord->st[ 0 ] = _pico_little_float( texCoord->st[ 0 ] );
			texCoord->st[ 1 ] = _pico_little_float( texCoord->st[ 1 ] );
		}

		/* swap xyz/normals */
		vertex = (md3Vertex_t*) ( (pmm::ub8_t*) surface + surface->ofsVertexes );
		for ( j = 0; j < ( surface->numVerts * surface->numFrames ); j++, vertex++ )
		{
			vertex->xyz[ 0 ] = _pico_little_short( vertex->xyz[ 0 ] );
			vertex->xyz[ 1 ] = _pico_little_short( vertex->xyz[ 1 ] );
			vertex->xyz[ 2 ] = _pico_little_short( vertex->xyz[ 2 ] );
			vertex->normal   = _pico_little_short( vertex->normal );
		}

		/* get next surface */
		surface = (md3Surface_t*) ( (pmm::ub8_t*) surface + surface->ofsEnd );
	}

	/* -------------------------------------------------
	   pico model creation
	   ------------------------------------------------- */

	/* create new pico model */
	picoModel = pmm::pp_new_model();
	if ( picoModel == nullptr ) {
		_pico_printf( pmm::pl_error, "Unable to allocate a new model" );
		_pico_free( bb0 );
		return nullptr;
	}

	/* do model setup */
	pmm::pp_set_model_frame_num( picoModel, frameNum );
	pmm::pp_set_model_num_frames( picoModel, md3->numFrames ); /* sea */
	pmm::pp_set_model_name( picoModel, fileName );
	pmm::pp_set_model_file_name( picoModel, fileName );

	/* md3 surfaces become picomodel surfaces */
	surface = (md3Surface_t*) ( bb + md3->ofsSurfaces );

	/* run through md3 surfaces */
	for ( i = 0; i < md3->num_surfaces; i++ )
	{
		/* allocate new pico surface */
		picoSurface = pmm::pp_new_surface( picoModel );
		if ( picoSurface == nullptr ) {
			_pico_printf( pmm::pl_error, "Unable to allocate a new model surface" );
			pmm::pp_free_model( picoModel ); /* sea */
			_pico_free( bb0 );
			return nullptr;
		}

		/* md3 model surfaces are all triangle meshes */
		pmm::pp_set_surface_type( picoSurface, pmm::st_triangles );

		/* set surface name */
		pmm::pp_set_surface_name( picoSurface, surface->name );

		/* create new pico shader -sea */
		picoShader = pmm::pp_new_shader( picoModel );
		if ( picoShader == nullptr ) {
			_pico_printf( pmm::pl_error, "Unable to allocate a new model shader" );
			pmm::pp_free_model( picoModel );
			_pico_free( bb0 );
			return nullptr;
		}

		/* detox and set shader name */
		shader = (md3Shader_t*) ( (pmm::ub8_t*) surface + surface->ofsShaders );
		_pico_setfext( shader->name, "" );
		_pico_unixify( shader->name );
		pmm::pp_set_shader_name( picoShader, shader->name );

		/* associate current surface with newly created shader */
		pmm::pp_set_surface_shader( picoSurface, picoShader );

		/* copy indices */
		triangle = (md3Triangle_t *) ( (pmm::ub8_t*) surface + surface->ofsTriangles );

		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			pmm::pp_set_surface_index( picoSurface, ( j * 3 + 0 ), (pmm::index_t) triangle->indices[ 0 ] );
			pmm::pp_set_surface_index( picoSurface, ( j * 3 + 1 ), (pmm::index_t) triangle->indices[ 1 ] );
			pmm::pp_set_surface_index( picoSurface, ( j * 3 + 2 ), (pmm::index_t) triangle->indices[ 2 ] );
		}

		/* copy vertices */
		texCoord = (md3TexCoord_t*) ( (pmm::ub8_t *) surface + surface->ofsSt );
		vertex = (md3Vertex_t*) ( (pmm::ub8_t*) surface + surface->ofsVertexes + surface->numVerts * frameNum * sizeof( md3Vertex_t ) );
		_pico_set_color( color, 255, 255, 255, 255 );

		for ( j = 0; j < surface->numVerts; j++, texCoord++, vertex++ )
		{
			/* set vertex origin */
			xyz[ 0 ] = MD3_SCALE * vertex->xyz[ 0 ];
			xyz[ 1 ] = MD3_SCALE * vertex->xyz[ 1 ];
			xyz[ 2 ] = MD3_SCALE * vertex->xyz[ 2 ];
			pmm::pp_set_surface_xyz( picoSurface, j, xyz );

			/* decode lat/lng normal to 3 float normal */
			lat = (float) ( ( vertex->normal >> 8 ) & 0xff );
			lng = (float) ( vertex->normal & 0xff );
			lat *= PICO_PI / 128;
			lng *= PICO_PI / 128;
			normal[ 0 ] = (pmm::vec_t) cos( lat ) * (pmm::vec_t) sin( lng );
			normal[ 1 ] = (pmm::vec_t) sin( lat ) * (pmm::vec_t) sin( lng );
			normal[ 2 ] = (pmm::vec_t) cos( lng );
			pmm::pp_set_surface_normal( picoSurface, j, normal );

			/* set st coords */
			st[ 0 ] = texCoord->st[ 0 ];
			st[ 1 ] = texCoord->st[ 1 ];
			pmm::pp_set_surface_st( picoSurface, 0, j, st );

			/* set color */
			pmm::pp_set_surface_color( picoSurface, 0, j, color );
		}

		/* get next surface */
		surface = (md3Surface_t*) ( (pmm::ub8_t*) surface + surface->ofsEnd );
	}

	/* return the new pico model */
	_pico_free( bb0 );
	return picoModel;
}



/* pico file format module definition */
extern const pmm::module_t picoModuleMD3 =
{
	"1.3",                      /* module version string */
	"Quake 3 Arena",            /* module display name */
	"Randy Reddig",             /* author's name */
	"2002 Randy Reddig",        /* module copyright */
	{
		"md3", nullptr, nullptr, nullptr /* default extensions to use */
	},
	_md3_canload,               /* validation routine */
	_md3_load,                  /* load routine */
	nullptr,                       /* save validation routine */
	nullptr                        /* save routine */
};

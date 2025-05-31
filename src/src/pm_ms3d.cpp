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

/* disable warnings */
#if GDEF_COMPILER_MSVC
#pragma warning( disable:4100 )		/* unref param */
#endif

/* remarks:
 * - loader seems stable
 * todo:
 * - fix uv coordinate problem
 * - check for buffer overflows ('bufptr' accesses)
 */
/* uncomment when debugging this module */
 #define DEBUG_PM_MS3D
 #define DEBUG_PM_MS3D_EX

/* plain white */
static pmm::color_t white = { 255,255,255,255 };

/* ms3d limits */
#define MS3D_MAX_VERTS      8192
#define MS3D_MAX_TRIS       16384
#define MS3D_MAX_GROUPS     128
#define MS3D_MAX_MATERIALS  128
#define MS3D_MAX_JOINTS     128
#define MS3D_MAX_KEYFRAMES  216

/* ms3d flags */
#define MS3D_SELECTED       1
#define MS3D_HIDDEN         2
#define MS3D_SELECTED2      4
#define MS3D_DIRTY          8

/* this freaky loader needs byte alignment */
#pragma pack(push, 1)

/* ms3d header */
class TMsHeader
{
public:
	char magic[10];
	int version;
};

/* ms3d vertex */
class TMsVertex
{
public:
	unsigned char flags;                /* sel, sel2, or hidden */
	float xyz[3];
	char boneID;                        /* -1 means 'no bone' */
	unsigned char refCount;
};

/* ms3d triangle */
class TMsTriangle
{
public:
	unsigned short flags;               /* sel, sel2, or hidden */
	unsigned short vertexIndices[3];
	float vertexNormals[3][3];
	float s[3];
	float t[3];
	unsigned char smoothingGroup;       /* 1 - 32 */
	unsigned char groupIndex;
};

/* ms3d material */
class TMsMaterial
{
public:
	char name[32];
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;                    /* range 0..128 */
	float transparency;                 /* range 0..1 */
	unsigned char mode;
	char texture [128];                 /* texture.bmp */
	char alphamap[128];                 /* alpha.bmp */
};

// ms3d group (static part)
// followed by a variable size block (see below)
class TMsGroup
{
public:
	unsigned char flags;                // sel, hidden
	char name[32];
	unsigned short numTriangles;
/*
    unsigned short	triangleIndices[ numTriangles ];
    char			materialIndex;		// -1 means 'no material'
 */
};

// ms3d joint
class TMsJoint
{
public:
	unsigned char flags;
	char name[32];
	char parentName[32];
	float rotation[3];
	float translation[3];
	unsigned short numRotationKeyframes;
	unsigned short numTranslationKeyframes;
};

// ms3d keyframe
class TMsKeyframe
{
public:
	float time;
	float parameter[3];
};

/* restore previous data alignment */
#pragma pack(pop)

/* _ms3d_canload:
 *	validates a milkshape3d model file.
 */
static int _ms3d_canload( PM_PARAMS_CANLOAD ){
	const TMsHeader *hdr;


	/* sanity check */
	if ( (pmm::size_type) bufSize < sizeof( TMsHeader ) ) {
		return pmm::pmv_error_size;
	}

	/* get ms3d header */
	hdr = (const TMsHeader *)buffer;

	/* check ms3d magic */
	if ( strncmp( hdr->magic,"MS3D000000",10 ) != 0 ) {
		return pmm::pmv_error_ident;
	}

	/* check ms3d version */
	if ( _pico_little_long( hdr->version ) < 3 ||
		 _pico_little_long( hdr->version ) > 4 ) {
		_pico_printf( pmm::pl_error,"MS3D file ignored. Only MS3D 1.3 and 1.4 is supported." );
		return pmm::pmv_error_version;
	}
	/* file seems to be a valid ms3d */
	return pmm::pmv_ok;
}

static unsigned char *GetWord( unsigned char *bufptr, int *out ){
	if ( bufptr == nullptr ) {
		return nullptr;
	}
	*out = _pico_little_short( *(unsigned short *)bufptr );
	return( bufptr + 2 );
}

/* _ms3d_load:
 *	loads a milkshape3d model file.
 */
static pmm::model_t *_ms3d_load( PM_PARAMS_LOAD ){
	pmm::model_t    *model;
	unsigned char  *bufptr, *bufptr0;
	int shaderRefs[ MS3D_MAX_GROUPS ];
	int numGroups;
	int numMaterials;
//	unsigned char  *ptrToGroups;
	int numVerts;
	unsigned char  *ptrToVerts;
	int numTris;
	unsigned char  *ptrToTris;
	int i,k,m;

	/* create new pico model */
	model = pmm::pp_new_model();
	if ( model == nullptr ) {
		return nullptr;
	}

	/* do model setup */
	pmm::pp_set_model_frame_num( model, frameNum );
	pmm::pp_set_model_name( model, fileName );
	pmm::pp_set_model_file_name( model, fileName );

	bufptr0 = bufptr = (pmm::ub8_t*) pmm::man.pp_m_new( bufSize );
	memcpy( bufptr, buffer, bufSize );
	/* skip header */
	bufptr += sizeof( TMsHeader );

	/* get number of vertices */
	bufptr = GetWord( bufptr,&numVerts );
	ptrToVerts = bufptr;

#ifdef DEBUG_PM_MS3D
	printf( "NumVertices: %d\n",numVerts );
#endif
	/* swap verts */
	for ( i = 0; i < numVerts; i++ )
	{
		TMsVertex *vertex;
		vertex = (TMsVertex *)bufptr;
		bufptr += sizeof( TMsVertex );

		vertex->xyz[ 0 ] = _pico_little_float( vertex->xyz[ 0 ] );
		vertex->xyz[ 1 ] = _pico_little_float( vertex->xyz[ 1 ] );
		vertex->xyz[ 2 ] = _pico_little_float( vertex->xyz[ 2 ] );

#ifdef DEBUG_PM_MS3D_EX_
		printf( "Vertex: x: %f y: %f z: %f\n",
				msvd[i]->vertex[0],
				msvd[i]->vertex[1],
				msvd[i]->vertex[2] );
#endif
	}
	/* get number of triangles */
	bufptr = GetWord( bufptr,&numTris );
	ptrToTris = bufptr;

#ifdef DEBUG_PM_MS3D
	printf( "NumTriangles: %d\n",numTris );
#endif
	/* swap tris */
	for ( i = 0; i < numTris; i++ )
	{
		TMsTriangle *triangle;
		triangle = (TMsTriangle *)bufptr;
		bufptr += sizeof( TMsTriangle );

		triangle->flags = _pico_little_short( triangle->flags );

		/* run through all tri verts */
		for ( k = 0; k < 3; k++ )
		{
			/* swap tex coords */
			triangle->s[ k ] = _pico_little_float( triangle->s[ k ] );
			triangle->t[ k ] = _pico_little_float( triangle->t[ k ] );

			/* swap fields */
			triangle->vertexIndices[ k ]      = _pico_little_short( triangle->vertexIndices[ k ] );
			triangle->vertexNormals[ 0 ][ k ] = _pico_little_float( triangle->vertexNormals[ 0 ][ k ] );
			triangle->vertexNormals[ 1 ][ k ] = _pico_little_float( triangle->vertexNormals[ 1 ][ k ] );
			triangle->vertexNormals[ 2 ][ k ] = _pico_little_float( triangle->vertexNormals[ 2 ][ k ] );

			/* check for out of range indices */
			if ( triangle->vertexIndices[ k ] >= numVerts ) {
				_pico_printf( pmm::pl_error,"Vertex %d index %d out of range (%d, max %d)",i,k,triangle->vertexIndices[k],numVerts - 1 );
				pmm::pp_free_model( model );
				pmm::man.pp_m_delete( bufptr0 );
				return nullptr; /* yuck */
			}
		}
	}
	/* get number of groups */
	bufptr = GetWord( bufptr,&numGroups );
//	ptrToGroups = bufptr;

#ifdef DEBUG_PM_MS3D
	printf( "NumGroups: %d\n",numGroups );
#endif
	/* run through all groups in model */
	for ( i = 0; i < numGroups && i < MS3D_MAX_GROUPS; i++ )
	{
		pmm::surface_t *surface;
		TMsGroup      *group;

		group = (TMsGroup *)bufptr;
		bufptr += sizeof( TMsGroup );

		/* we ignore hidden groups */
		if ( group->flags & MS3D_HIDDEN ) {
			bufptr += ( group->numTriangles * 2 ) + 1;
			continue;
		}
		/* forced null term of group name */
		group->name[ 31 ] = '\0';

		/* create new pico surface */
		surface = pmm::pp_new_surface( model );
		if ( surface == nullptr ) {
			pmm::pp_free_model( model );
			pmm::man.pp_m_delete( bufptr0 );
			return nullptr;
		}
		/* do surface setup */
		pmm::pp_set_surface_type( surface,pmm::st_triangles );
		pmm::pp_set_surface_name( surface,group->name );

		/* process triangle indices */
		for ( k = 0; k < group->numTriangles; k++ )
		{
			TMsTriangle *triangle;
			unsigned int triangleIndex;

			/* get triangle index */
			bufptr = GetWord( bufptr,(int *)&triangleIndex );

			/* get ptr to triangle data */
			triangle = (TMsTriangle *)( ptrToTris + ( sizeof( TMsTriangle ) * triangleIndex ) );

			/* run through triangle vertices */
			for ( m = 0; m < 3; m++ )
			{
				TMsVertex   *vertex;
				unsigned int vertexIndex;
				pmm::vec2_t texCoord;

				/* get ptr to vertex data */
				vertexIndex = triangle->vertexIndices[ m ];
				vertex = (TMsVertex *)( ptrToVerts + ( sizeof( TMsVertex ) * vertexIndex ) );

				/* store vertex origin */
				pmm::pp_set_surface_xyz( surface,vertexIndex,vertex->xyz );

				/* store vertex color */
				pmm::pp_set_surface_color( surface,0,vertexIndex,white );

				/* store vertex normal */
				pmm::pp_set_surface_normal( surface,vertexIndex,triangle->vertexNormals[ m ] );

				/* store current face vertex index */
				pmm::pp_set_surface_index( surface,( k * 3 + ( 2 - m ) ),(pmm::index_t)vertexIndex );

				/* get texture vertex coord */
				texCoord[ 0 ] = triangle->s[ m ];
				texCoord[ 1 ] = -triangle->t[ m ];  /* flip t */

				/* store texture vertex coord */
				pmm::pp_set_surface_st( surface,0,vertexIndex,texCoord );
			}
		}
		/* store material */
		shaderRefs[ i ] = *bufptr++;

#ifdef DEBUG_PM_MS3D
		printf( "Group %d: '%s' (%d tris)\n",i,group->name,group->numTriangles );
#endif
	}
	/* get number of materials */
	bufptr = GetWord( bufptr,&numMaterials );

#ifdef DEBUG_PM_MS3D
	printf( "NumMaterials: %d\n",numMaterials );
#endif
	/* run through all materials in model */
	for ( i = 0; i < numMaterials; i++ )
	{
		pmm::shader_t *shader;
		pmm::color_t ambient,diffuse,specular;
		TMsMaterial  *material;
		int k;

		material = (TMsMaterial *)bufptr;
		bufptr += sizeof( TMsMaterial );

		/* null term strings */
		material->name    [  31 ] = '\0';
		material->texture [ 127 ] = '\0';
		material->alphamap[ 127 ] = '\0';

		/* ltrim strings */
		_pico_strltrim( material->name );
		_pico_strltrim( material->texture );
		_pico_strltrim( material->alphamap );

		/* rtrim strings */
		_pico_strrtrim( material->name );
		_pico_strrtrim( material->texture );
		_pico_strrtrim( material->alphamap );

		/* create new pico shader */
		shader = pmm::pp_new_shader( model );
		if ( shader == nullptr ) {
			pmm::pp_free_model( model );
			pmm::man.pp_m_delete( bufptr0 );
			return nullptr;
		}
		/* scale shader colors */
		for ( k = 0; k < 4; k++ )
		{
			ambient [ k ] = (pmm::ub8_t) ( material->ambient[ k ] * 255 );
			diffuse [ k ] = (pmm::ub8_t) ( material->diffuse[ k ] * 255 );
			specular[ k ] = (pmm::ub8_t) ( material->specular[ k ] * 255 );
		}
		/* set shader colors */
		pmm::pp_set_shader_ambient_color( shader,ambient );
		pmm::pp_set_shader_diffuse_color( shader,diffuse );
		pmm::pp_set_shader_specular_color( shader,specular );

		/* set shader transparency */
		pmm::pp_set_shader_transparency( shader,material->transparency );

		/* set shader shininess (0..127) */
		pmm::pp_set_shader_shininess( shader,material->shininess );

		/* set shader name */
		pmm::pp_set_shader_name( shader,material->name );

		/* set shader texture map name */
		pmm::pp_set_shader_map_name( shader,material->texture );

#ifdef DEBUG_PM_MS3D
		printf( "Material %d: '%s' ('%s','%s')\n",i,material->name,material->texture,material->alphamap );
#endif
	}
	/* assign shaders to surfaces */
	for ( i = 0; i < numGroups && i < MS3D_MAX_GROUPS; i++ )
	{
		pmm::surface_t *surface;
		pmm::shader_t  *shader;

		/* sanity check */
		if ( shaderRefs[ i ] >= MS3D_MAX_MATERIALS ||
			 shaderRefs[ i ] < 0 ) {
			continue;
		}

		/* get surface */
		surface = pmm::pp_get_model_surface( model,i );
		if ( surface == nullptr ) {
			continue;
		}

		/* get shader */
		shader = pmm::pp_get_model_shader( model,shaderRefs[ i ] );
		if ( shader == nullptr ) {
			continue;
		}

		/* assign shader */
		pmm::pp_set_surface_shader( surface,shader );

#ifdef DEBUG_PM_MS3D
		printf( "Mapped: %d ('%s') to %d (%s)\n",
				shaderRefs[i],shader->name,i,surface->name );
#endif
	}
	/* return allocated pico model */
	pmm::man.pp_m_delete( bufptr0 );
	return model;
//	return nullptr;
}

/* pico file format module definition */
extern const pmm::module_t picoModuleMS3D =
{
	"0.4-a",                    /* module version string */
	"Milkshape 3D",             /* module display name */
	"seaw0lf",                  /* author's name */
	"2002 seaw0lf",             /* module copyright */
	{
		"ms3d",nullptr,nullptr,nullptr   /* default extensions to use */
	},
	_ms3d_canload,              /* validation routine */
	_ms3d_load,                 /* load routine */
	nullptr,                       /* save validation routine */
	nullptr                        /* save routine */
};

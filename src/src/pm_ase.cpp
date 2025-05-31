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
   other aseMaterialList provided with the distribution.

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

/* uncomment when debugging this module */
//#define DEBUG_PM_ASE
//#define DEBUG_PM_ASE_EX


/* dependencies */
#include <pmpmesh/pm_internal.hpp>

#ifdef DEBUG_PM_ASE
#include "time.h"
#endif

/* plain white */
static pmm::color_t white = { 255, 255, 255, 255 };

/* jhefty - multi-subobject material support */

/* Material/SubMaterial management */
/* A material should have 1..n submaterials assigned to it */

class aseSubMaterial_t
{
public:
	aseSubMaterial_t * next;
	int subMtlId;
	pmm::shader_t* shader;

};

class aseMaterial_t
{
public:
	aseMaterial_t * next;
	aseSubMaterial_t * subMtls;
	int mtlId;
};

/* Material/SubMaterial management functions */
static aseMaterial_t* _ase_get_material( aseMaterial_t* list, int mtlIdParent ){
	aseMaterial_t* mtl = list;

	while ( mtl )
	{
		if ( mtlIdParent == mtl->mtlId ) {
			break;
		}
		mtl = mtl->next;
	}
	return mtl;
}

static aseSubMaterial_t* _ase_get_submaterial( aseMaterial_t* list, int mtlIdParent, int subMtlId ){
	aseMaterial_t* parent = _ase_get_material( list, mtlIdParent );
	aseSubMaterial_t* subMtl = nullptr;

	if ( !parent ) {
		_pico_printf( pmm::pl_error, "No ASE material exists with id %i\n", mtlIdParent );
		return nullptr;
	}

	subMtl = parent->subMtls;
	while ( subMtl )
	{
		if ( subMtlId == subMtl->subMtlId ) {
			break;
		}
		subMtl = subMtl->next;
	}
	return subMtl;
}

aseSubMaterial_t* _ase_get_submaterial_or_default( aseMaterial_t* materials, int mtlIdParent, int subMtlId ){
	aseSubMaterial_t* subMtl = _ase_get_submaterial( materials, mtlIdParent, subMtlId );
	if ( subMtl != nullptr ) {
		return subMtl;
	}

	/* ydnar: trying default submaterial */
	subMtl = _ase_get_submaterial( materials, mtlIdParent, 0 );
	if ( subMtl != nullptr ) {
		return subMtl;
	}

	_pico_printf( pmm::pl_error, "Could not find material/submaterial for id %d/%d\n", mtlIdParent, subMtlId );
	return nullptr;
}




static aseMaterial_t* _ase_add_material( aseMaterial_t **list, int mtlIdParent ){
	aseMaterial_t * mtl = reinterpret_cast<decltype(mtl)>(pmm::man.pp_k_new( 1, sizeof( aseMaterial_t ) ));
	mtl->mtlId = mtlIdParent;
	mtl->subMtls = nullptr;
	mtl->next = *list;
	*list = mtl;

	return mtl;
}

static aseSubMaterial_t* _ase_add_submaterial( aseMaterial_t **list, int mtlIdParent, int subMtlId, pmm::shader_t* shader ){
	aseMaterial_t *parent = _ase_get_material( *list,  mtlIdParent );
	aseSubMaterial_t * subMtl = reinterpret_cast<decltype(subMtl)>(pmm::man.pp_k_new( 1, sizeof( aseSubMaterial_t ) ));

	if ( !parent ) {
		parent = _ase_add_material( list, mtlIdParent );
	}

	subMtl->shader = shader;
	subMtl->subMtlId = subMtlId;
	subMtl->next = parent->subMtls;
	parent->subMtls = subMtl;

	return subMtl;
}

static void _ase_free_materials( aseMaterial_t **list ){
	aseMaterial_t* mtl = *list;
	aseSubMaterial_t* subMtl = nullptr;

	aseMaterial_t* mtlTemp = nullptr;
	aseSubMaterial_t* subMtlTemp = nullptr;

	while ( mtl )
	{
		subMtl = mtl->subMtls;
		while ( subMtl )
		{
			subMtlTemp = subMtl->next;
			pmm::man.pp_m_delete( subMtl );
			subMtl = subMtlTemp;
		}
		mtlTemp = mtl->next;
		pmm::man.pp_m_delete( mtl );
		mtl = mtlTemp;
	}
	( *list ) = nullptr;
}

#ifdef DEBUG_PM_ASE
static void _ase_print_materials( aseMaterial_t *list ){
	aseMaterial_t* mtl = list;
	aseSubMaterial_t* subMtl = nullptr;

	while ( mtl )
	{
		_pico_printf( pmm::pl_normal,  "ASE Material %i", mtl->mtlId );
		subMtl = mtl->subMtls;
		while ( subMtl )
		{
			_pico_printf( pmm::pl_normal,  " -- ASE SubMaterial %i - %s\n", subMtl->subMtlId, subMtl->shader->name );
			subMtl = subMtl->next;
		}
		mtl = mtl->next;
	}
}
#endif //DEBUG_PM_ASE

/* todo:
 * - apply material specific uv offsets to uv coordinates
 */

/* _ase_canload:
 *  validates a 3dsmax ase model file.
 */
static int _ase_canload( PM_PARAMS_CANLOAD ){
	picoParser_t *p;


	/* quick data length validation */
	if ( bufSize < 80 ) {
		return pmm::pmv_error_size;
	}

	/* create pico parser */
	p = _pico_new_parser( (const pmm::ub8_t*) buffer, bufSize );
	if ( p == nullptr ) {
		return pmm::pmv_error_memory;
	}

	/* get first token */
	if ( _pico_parse_first( p ) == nullptr ) {
		return pmm::pmv_error_ident;
	}

	/* check first token */
	if ( _pico_stricmp( p->token, "*3dsmax_asciiexport" ) ) {
		_pico_free_parser( p );
		return pmm::pmv_error_ident;
	}

	/* free the pico parser object */
	_pico_free_parser( p );

	/* file seems to be a valid ase file */
	return pmm::pmv_ok;
}

class aseVertex_t
{
public:
	pmm::vec3_t xyz;
	pmm::vec3_t normal;
	pmm::index_t id;
};

class aseTexCoord_t
{
public:
	pmm::vec2_t texcoord;
};

class aseColor_t
{
public:
	pmm::color_t color;
};

class aseFace_t
{
public:
	pmm::index_t indices[9];
	pmm::index_t smoothingGroup;
	pmm::index_t materialId;
	pmm::index_t subMaterialId;
};
using aseFacesIter_t = aseFace_t *;

pmm::surface_t* PicoModelFindOrAddSurface( pmm::model_t *model, pmm::shader_t* shader ){
	/* see if a surface already has the shader */
	int i = 0;
	for ( ; i < model->num_surfaces ; i++ )
	{
		pmm::surface_t* workSurface = model->surface[i];
		if ( workSurface->shader == shader ) {
			return workSurface;
		}
	}

	/* no surface uses this shader yet, so create a new surface */

	{
		/* create a new surface in the model for the unique shader */
		pmm::surface_t* workSurface = pmm::pp_new_surface( model );
		if ( !workSurface ) {
			_pico_printf( pmm::pl_error, "Could not allocate a new surface!\n" );
			return 0;
		}

		/* do surface setup */
		pmm::pp_set_surface_type( workSurface, pmm::st_triangles );
		pmm::pp_set_surface_name( workSurface, shader->name );
		pmm::pp_set_surface_shader( workSurface, shader );

		return workSurface;
	}
}

/* _ase_submit_triangles - jhefty
   use the surface and the current face list to look up material/submaterial IDs
   and submit them to the model for proper processing

   The following still holds from ydnar's _ase_make_surface:
   indices 0 1 2 = vert indices
   indices 3 4 5 = st indices
   indices 6 7 8 = color indices (new)
 */

#if 0
using picoIndexIter_t = pmm::index_t *;

class aseUniqueIndices_t
{
public:
	pmm::index_t* data;
	pmm::index_t* last;

	aseFace_t* faces;
};

pmm::size_type aseUniqueIndices_size( aseUniqueIndices_t* self ){
	return self->last - self->data;
}

void aseUniqueIndices_reserve( aseUniqueIndices_t* self, pmm::index_t size ){
	self->data = self->last = (pmm::index_t*)pmm::man.pp_k_new( size, sizeof( pmm::index_t ) );
}

void aseUniqueIndices_clear( aseUniqueIndices_t* self ){
	pmm::man.pp_m_delete( self->data );
}

void aseUniqueIndices_pushBack( aseUniqueIndices_t* self, pmm::index_t index ){
	*self->last++ = index;
}

pmm::index_t aseFaces_getVertexIndex( aseFace_t* faces, pmm::index_t index ){
	return faces[index / 3].indices[index % 3];
}

pmm::index_t aseFaces_getTexCoordIndex( aseFace_t* faces, pmm::index_t index ){
	return faces[index / 3].indices[( index % 3 ) + 3];
}

pmm::index_t aseFaces_getColorIndex( aseFace_t* faces, pmm::index_t index ){
	return faces[index / 3].indices[( index % 3 ) + 6];
}

int aseUniqueIndex_equal( aseFace_t* faces, pmm::index_t index, pmm::index_t other ){
	return aseFaces_getVertexIndex( faces, index ) == aseFaces_getVertexIndex( faces, other )
		   && aseFaces_getTexCoordIndex( faces, index ) == aseFaces_getTexCoordIndex( faces, other )
		   && aseFaces_getColorIndex( faces, index ) == aseFaces_getColorIndex( faces, other );
}

pmm::index_t aseUniqueIndices_insertUniqueVertex( aseUniqueIndices_t* self, pmm::index_t index ){
	picoIndexIter_t i = self->data;
	for (; i != self->last; ++i )
	{
		pmm::index_t other = (pmm::index_t)( i - self->data );
		if ( aseUniqueIndex_equal( self->faces, index, other ) ) {
			return other;
		}
	}

	aseUniqueIndices_pushBack( self, index );
	return (pmm::index_t)( aseUniqueIndices_size( self ) - 1 );
}

static void _ase_submit_triangles_unshared( pmm::model_t* model, aseMaterial_t* materials, aseVertex_t* vertices, aseTexCoord_t* texcoords, aseColor_t* colors, aseFace_t* faces, int numFaces, int meshHasNormals ){
	aseFacesIter_t i = faces, end = faces + numFaces;

	aseUniqueIndices_t indices;
	aseUniqueIndices_t remap;
	aseUniqueIndices_reserve( &indices, numFaces * 3 );
	aseUniqueIndices_reserve( &remap, numFaces * 3 );
	indices.faces = faces;

	for (; i != end; ++i )
	{
		/* look up the shader for the material/submaterial pair */
		aseSubMaterial_t* subMtl = _ase_get_submaterial_or_default( materials, ( *i ).materialId, ( *i ).subMaterialId );
		if ( subMtl == nullptr ) {
			return;
		}

		{
			pmm::surface_t* surface = PicoModelFindOrAddSurface( model, subMtl->shader );
			int j;
			/* we pull the data from the vertex, color and texcoord arrays using the face index data */
			for ( j = 0 ; j < 3 ; j++ )
			{
				pmm::index_t index = (pmm::index_t)( ( ( i - faces ) * 3 ) + j );
				pmm::index_t size = (pmm::index_t)aseUniqueIndices_size( &indices );
				pmm::index_t unique = aseUniqueIndices_insertUniqueVertex( &indices, index );

				pmm::index_t numVertexes = pmm::pp_get_surface_num_vertices( surface );
				pmm::index_t numIndexes = pmm::pp_get_surface_num_indices( surface );

				aseUniqueIndices_pushBack( &remap, numIndexes );

				pmm::pp_set_surface_index( surface, numIndexes, remap.data[unique] );

				if ( unique == size ) {
					pmm::pp_set_surface_xyz( surface, numVertexes, vertices[( *i ).indices[j]].xyz );
					pmm::pp_set_surface_normal( surface, numVertexes, vertices[( *i ).indices[j]].normal );
					pmm::pp_set_surface_st( surface, 0, numVertexes, texcoords[( *i ).indices[j + 3]].texcoord );

					if ( ( *i ).indices[j + 6] >= 0 ) {
						pmm::pp_set_surface_color( surface, 0, numVertexes, colors[( *i ).indices[j + 6]].color );
					}
					else
					{
						pmm::pp_set_surface_color( surface, 0, numVertexes, white );
					}

					pmm::pp_set_surface_smoothing_group( surface, numVertexes, ( vertices[( *i ).indices[j]].id * ( 1 << 16 ) ) + ( *i ).smoothingGroup );
				}
			}
		}
	}

	aseUniqueIndices_clear( &indices );
	aseUniqueIndices_clear( &remap );
}

#endif

static void _ase_submit_triangles( pmm::model_t* model, aseMaterial_t* materials, aseVertex_t* vertices, aseTexCoord_t* texcoords, aseColor_t* colors, aseFace_t* faces, int numFaces, const char *name ){
	aseFacesIter_t i = faces, end = faces + numFaces;
	for (; i != end; ++i )
	{
		/* look up the shader for the material/submaterial pair */
		aseSubMaterial_t* subMtl = _ase_get_submaterial_or_default( materials, ( *i ).materialId, ( *i ).subMaterialId );
		if ( subMtl == nullptr ) {
			return;
		}

		{
			pmm::vec3_t* xyz[3];
			pmm::vec3_t* normal[3];
			pmm::vec2_t* st[3];
			pmm::color_t* color[3];
			pmm::index_t smooth[3];
			int j;
			/* we pull the data from the vertex, color and texcoord arrays using the face index data */
			for ( j = 0 ; j < 3 ; j++ )
			{
				xyz[j]    = &vertices[( *i ).indices[j]].xyz;
				normal[j] = &vertices[( *i ).indices[j]].normal;
				st[j]     = &texcoords[( *i ).indices[j + 3]].texcoord;

				if ( colors != nullptr && ( *i ).indices[j + 6] >= 0 ) {
					color[j] = &colors[( *i ).indices[j + 6]].color;
				}
				else
				{
					color[j] = &white;
				}

				smooth[j] = ( vertices[( *i ).indices[j]].id * ( 1 << 16 ) ) + ( *i ).smoothingGroup; /* don't merge vertices */

			}

			/* submit the triangle to the model */
			pmm::pp_add_triangle_to_model( model, xyz, normal, 1, st, 1, color, subMtl->shader, name, smooth );
		}
	}
}

static void shadername_convert( char* shaderName ){
	/* unix-style path separators */
	char* s = shaderName;
	for (; *s != '\0'; ++s )
	{
		if ( *s == '\\' ) {
			*s = '/';
		}
	}
}


/* _ase_load:
 *  loads a 3dsmax ase model file.
 */
static pmm::model_t *_ase_load( PM_PARAMS_LOAD ){
	pmm::model_t    *model;
	picoParser_t   *p;
	char lastNodeName[ 1024 ];

	aseVertex_t* vertices = nullptr;
	aseTexCoord_t* texcoords = nullptr;
	aseColor_t* colors = nullptr;
	aseFace_t* faces = nullptr;
	int numVertices = 0;
	int numFaces = 0;
	int numTextureVertices = 0;
	int numTextureVertexFaces = 0;
	int numColorVertices = 0;
	int numColorVertexFaces = 0;
	int vertexId = 0;

	aseMaterial_t* materials = nullptr;

#ifdef DEBUG_PM_ASE
	clock_t start, finish;
	double elapsed;
	start = clock();
#endif

	/* helper */
	#define _ase_error_return( m ) \
	{ \
		_pico_printf( pmm::pl_error,"%s in ASE, line %d.",m,p->curLine ); \
		_pico_free_parser( p );	\
		pmm::pp_free_model( model );	\
		return nullptr; \
	}
	/* create a new pico parser */
	p = _pico_new_parser( (const pmm::ub8_t *)buffer,bufSize );
	if ( p == nullptr ) {
		return nullptr;
	}

	/* create a new pico model */
	model = pmm::pp_new_model();
	if ( model == nullptr ) {
		_pico_free_parser( p );
		return nullptr;
	}
	/* do model setup */
	pmm::pp_set_model_frame_num( model, frameNum );
	pmm::pp_set_model_name( model, fileName );
	pmm::pp_set_model_file_name( model, fileName );

	/* initialize some stuff */
	memset( lastNodeName,0,sizeof( lastNodeName ) );

	/* parse ase model file */
	while ( 1 )
	{
		/* get first token on line */
		if ( _pico_parse_first( p ) == nullptr ) {
			break;
		}

		/* we just skip empty lines */
		if ( p->token == nullptr || !strlen( p->token ) ) {
			continue;
		}

		/* we skip invalid ase statements */
		if ( p->token[0] != '*' && p->token[0] != '{' && p->token[0] != '}' ) {
			_pico_parse_skip_rest( p );
			continue;
		}
		/* remember node name */
		if ( !_pico_stricmp( p->token,"*node_name" ) ) {
			/* read node name */
			char *ptr = _pico_parse( p,0 );
			if ( ptr == nullptr ) {
				_ase_error_return( "Node name parse error" );
			}

			/* remember node name */
			strncpy( lastNodeName,ptr,sizeof( lastNodeName ) );
		}
		/* model mesh (originally contained within geomobject) */
		else if ( !_pico_stricmp( p->token,"*mesh" ) ) {
			/* finish existing surface */
			_ase_submit_triangles( model, materials, vertices, texcoords, colors, faces, numFaces, lastNodeName );
			pmm::man.pp_m_delete( faces );
			pmm::man.pp_m_delete( vertices );
			pmm::man.pp_m_delete( texcoords );
			pmm::man.pp_m_delete( colors );
		}
		else if ( !_pico_stricmp( p->token,"*mesh_numvertex" ) ) {
			if ( !_pico_parse_int( p, &numVertices ) ) {
				_ase_error_return( "Missing MESH_NUMVERTEX value" );
			}

			vertices = reinterpret_cast<decltype(vertices)>(pmm::man.pp_k_new( numVertices, sizeof( aseVertex_t ) ));
		}
		else if ( !_pico_stricmp( p->token,"*mesh_numfaces" ) ) {
			if ( !_pico_parse_int( p, &numFaces ) ) {
				_ase_error_return( "Missing MESH_NUMFACES value" );
			}

			faces = reinterpret_cast<decltype(faces)>(pmm::man.pp_k_new( numFaces, sizeof( aseFace_t ) ));
		}
		else if ( !_pico_stricmp( p->token,"*mesh_numtvertex" ) ) {
			if ( !_pico_parse_int( p, &numTextureVertices ) ) {
				_ase_error_return( "Missing MESH_NUMTVERTEX value" );
			}

			texcoords = reinterpret_cast<decltype(texcoords)>(pmm::man.pp_k_new( numTextureVertices, sizeof( aseTexCoord_t ) ));
		}
		else if ( !_pico_stricmp( p->token,"*mesh_numtvfaces" ) ) {
			if ( !_pico_parse_int( p, &numTextureVertexFaces ) ) {
				_ase_error_return( "Missing MESH_NUMTVFACES value" );
			}
		}
		else if ( !_pico_stricmp( p->token,"*mesh_numcvertex" ) ) {
			if ( !_pico_parse_int( p, &numColorVertices ) ) {
				_ase_error_return( "Missing MESH_NUMCVERTEX value" );
			}

			colors = reinterpret_cast<decltype(colors)>(pmm::man.pp_k_new( numColorVertices, sizeof( aseColor_t ) ));
			memset( colors, 255, numColorVertices * sizeof( aseColor_t ) ); /* ydnar: force colors to white initially */
		}
		else if ( !_pico_stricmp( p->token,"*mesh_numcvfaces" ) ) {
			if ( !_pico_parse_int( p, &numColorVertexFaces ) ) {
				_ase_error_return( "Missing MESH_NUMCVFACES value" );
			}
		}
		/* mesh material reference. this usually comes at the end of */
		/* geomobjects after the mesh blocks. we must assume that the */
		/* new mesh was already created so all we can do here is assign */
		/* the material reference id (shader index) now. */
		else if ( !_pico_stricmp( p->token,"*material_ref" ) ) {
			int mtlId;

			/* get the material ref (0..n) */
			if ( !_pico_parse_int( p,&mtlId ) ) {
				_ase_error_return( "Missing material reference ID" );
			}

			{
				int i = 0;
				/* fix up all of the aseFaceList in the surface to point to the parent material */
				/* we've already saved off their subMtl */
				for (; i < numFaces; ++i )
				{
					faces[i].materialId = mtlId;
				}
			}
		}
		/* model mesh vertex */
		else if ( !_pico_stricmp( p->token,"*mesh_vertex" ) ) {
			int index;

			if ( numVertices == 0 ) {
				_ase_error_return( "Vertex parse error" );
			}

			/* get vertex data (orig: index +y -x +z) */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Vertex parse error" );
			}
			if ( !_pico_parse_vec( p,vertices[index].xyz ) ) {
				_ase_error_return( "Vertex parse error" );
			}

			vertices[index].id = vertexId++;
		}
		/* model mesh vertex normal */
		else if ( !_pico_stricmp( p->token,"*mesh_vertexnormal" ) ) {
			int index;

			if ( numVertices == 0 ) {
				_ase_error_return( "Vertex parse error" );
			}

			/* get vertex data (orig: index +y -x +z) */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Vertex parse error" );
			}
			if ( !_pico_parse_vec( p,vertices[index].normal ) ) {
				_ase_error_return( "Vertex parse error" );
			}
		}
		/* model mesh face */
		else if ( !_pico_stricmp( p->token,"*mesh_face" ) ) {
			pmm::index_t indices[3];
			int index;

			if ( numFaces == 0 ) {
				_ase_error_return( "Face parse error" );
			}

			/* get face index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 1st vertex index */
			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indices[0] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 2nd vertex index */
			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indices[1] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 3rd vertex index */
			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indices[2] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* parse to the subMaterial ID */
			while ( 1 )
			{
				if ( !_pico_parse( p,0 ) ) { /* EOL */
					break;
				}
				if ( !_pico_stricmp( p->token,"*MESH_SMOOTHING" ) ) {
					_pico_parse_int( p, &faces[index].smoothingGroup );
				}
				if ( !_pico_stricmp( p->token,"*MESH_MTLID" ) ) {
					_pico_parse_int( p, &faces[index].subMaterialId );
				}
			}

			faces[index].materialId = 0;
			faces[index].indices[0] = indices[2];
			faces[index].indices[1] = indices[1];
			faces[index].indices[2] = indices[0];
		}
		/* model texture vertex */
		else if ( !_pico_stricmp( p->token,"*mesh_tvert" ) ) {
			int index;

			if ( numVertices == 0 ) {
				_ase_error_return( "Texture Vertex parse error" );
			}

			/* get uv vertex index */
			if ( !_pico_parse_int( p,&index ) || index >= numTextureVertices ) {
				_ase_error_return( "Texture vertex parse error" );
			}

			/* get uv vertex s */
			if ( !_pico_parse_float( p,&texcoords[index].texcoord[0] ) ) {
				_ase_error_return( "Texture vertex parse error" );
			}

			/* get uv vertex t */
			if ( !_pico_parse_float( p,&texcoords[index].texcoord[1] ) ) {
				_ase_error_return( "Texture vertex parse error" );
			}

			/* ydnar: invert t */
			texcoords[index].texcoord[ 1 ] = 1.0f - texcoords[index].texcoord[ 1 ];
		}
		/* ydnar: model mesh texture face */
		else if ( !_pico_stricmp( p->token, "*mesh_tface" ) ) {
			pmm::index_t indices[3];
			int index;

			if ( numFaces == 0 ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get face index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get 1st vertex index */
			if ( !_pico_parse_int( p,&indices[0] ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get 2nd vertex index */
			if ( !_pico_parse_int( p,&indices[1] ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get 3rd vertex index */
			if ( !_pico_parse_int( p,&indices[2] ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			faces[index].indices[3] = indices[2];
			faces[index].indices[4] = indices[1];
			faces[index].indices[5] = indices[0];
		}
		/* model color vertex */
		else if ( !_pico_stricmp( p->token,"*mesh_vertcol" ) ) {
			int index;
			float colorInput;

			if ( numVertices == 0 ) {
				_ase_error_return( "Color Vertex parse error" );
			}

			/* get color vertex index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Color vertex parse error" );
			}

			/* get R component */
			if ( !_pico_parse_float( p,&colorInput ) ) {
				_ase_error_return( "Color vertex parse error" );
			}
			colors[index].color[0] = (pmm::ub8_t)( colorInput * 255 );

			/* get G component */
			if ( !_pico_parse_float( p,&colorInput ) ) {
				_ase_error_return( "Color vertex parse error" );
			}
			colors[index].color[1] = (pmm::ub8_t)( colorInput * 255 );

			/* get B component */
			if ( !_pico_parse_float( p,&colorInput ) ) {
				_ase_error_return( "Color vertex parse error" );
			}
			colors[index].color[2] = (pmm::ub8_t)( colorInput * 255 );

			/* leave alpha alone since we don't get any data from the ASE format */
			colors[index].color[3] = 255;
		}
		/* model color face */
		else if ( !_pico_stricmp( p->token,"*mesh_cface" ) ) {
			pmm::index_t indices[3];
			int index;

			if ( numFaces == 0 ) {
				_ase_error_return( "Face parse error" );
			}

			/* get face index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 1st cvertex index */
			//			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indices[0] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 2nd cvertex index */
			//			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indices[1] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 3rd cvertex index */
			//			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indices[2] ) ) {
				_ase_error_return( "Face parse error" );
			}

			faces[index].indices[6] = indices[2];
			faces[index].indices[7] = indices[1];
			faces[index].indices[8] = indices[0];
		}
		/* model material */
		else if ( !_pico_stricmp( p->token, "*material" ) ) {
			aseSubMaterial_t*   subMaterial = nullptr;
			pmm::shader_t        *shader = nullptr;
			int level = 1, index;
			char materialName[ 1024 ];
			float transValue = 0.0f, shineValue = 1.0f;
			pmm::color_t ambientColor, diffuseColor, specularColor;
			char                *mapname = nullptr;
			int subMtlId, subMaterialLevel = -1;


			/* get material index */
			_pico_parse_int( p,&index );

			/* check brace */
			if ( !_pico_parse_check( p,1,"{" ) ) {
				_ase_error_return( "Material missing opening brace" );
			}

			/* parse material block */
			while ( 1 )
			{
				/* get next token */
				if ( _pico_parse( p,1 ) == nullptr ) {
					break;
				}
				if ( !strlen( p->token ) ) {
					continue;
				}

				/* handle levels */
				if ( p->token[0] == '{' ) {
					level++;
				}
				if ( p->token[0] == '}' ) {
					level--;
				}
				if ( !level ) {
					break;
				}

				if ( level == subMaterialLevel ) {
					/* set material name */
					_pico_first_token( materialName );
					shadername_convert( materialName );
					pmm::pp_set_shader_name( shader, materialName );

					/* set shader's transparency */
					pmm::pp_set_shader_transparency( shader,transValue );

					/* set shader's ambient color */
					pmm::pp_set_shader_ambient_color( shader,ambientColor );

					/* set diffuse alpha to transparency */
					diffuseColor[3] = (pmm::ub8_t)( transValue * 255.0 );

					/* set shader's diffuse color */
					pmm::pp_set_shader_diffuse_color( shader,diffuseColor );

					/* set shader's specular color */
					pmm::pp_set_shader_specular_color( shader,specularColor );

					/* set shader's shininess */
					pmm::pp_set_shader_shininess( shader,shineValue );

					/* set material map name */
					pmm::pp_set_shader_map_name( shader, mapname );

					subMaterial = _ase_add_submaterial( &materials, index, subMtlId, shader );
					subMaterialLevel = -1;
				}

				/* parse submaterial index */
				if ( !_pico_stricmp( p->token,"*submaterial" ) ) {
					/* allocate new pico shader */
					_pico_parse_int( p, &subMtlId );

					shader = pmm::pp_new_shader( model );
					if ( shader == nullptr ) {
						pmm::pp_free_model( model );
						return nullptr;
					}
					subMaterialLevel = level;
				}
				/* parse material name */
				else if ( !_pico_stricmp( p->token,"*material_name" ) ) {
					char* name = _pico_parse( p,0 );
					if ( name == nullptr ) {
						_ase_error_return( "Missing material name" );
					}

					strcpy( materialName, name );
					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse material transparency */
				else if ( !_pico_stricmp( p->token,"*material_transparency" ) ) {
					/* get transparency value from ase */
					if ( !_pico_parse_float( p,&transValue ) ) {
						_ase_error_return( "Material transparency parse error" );
					}

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse material shininess */
				else if ( !_pico_stricmp( p->token,"*material_shine" ) ) {
					/* remark:
					 * - not sure but instead of '*material_shine' i might
					 *   need to use '*material_shinestrength' */

					/* get shine value from ase */
					if ( !_pico_parse_float( p,&shineValue ) ) {
						_ase_error_return( "Material shine parse error" );
					}

					/* scale ase shine range 0..1 to pico range 0..127 */
					shineValue *= 128.0;

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse ambient material color */
				else if ( !_pico_stricmp( p->token,"*material_ambient" ) ) {
					pmm::vec3_t vec;
					/* get r,g,b float values from ase */
					if ( !_pico_parse_vec( p,vec ) ) {
						_ase_error_return( "Material color parse error" );
					}

					/* setup 0..255 range color values */
					ambientColor[ 0 ] = (int)( vec[ 0 ] * 255.0 );
					ambientColor[ 1 ] = (int)( vec[ 1 ] * 255.0 );
					ambientColor[ 2 ] = (int)( vec[ 2 ] * 255.0 );
					ambientColor[ 3 ] = (int)( 255 );

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse diffuse material color */
				else if ( !_pico_stricmp( p->token,"*material_diffuse" ) ) {
					pmm::vec3_t vec;

					/* get r,g,b float values from ase */
					if ( !_pico_parse_vec( p,vec ) ) {
						_ase_error_return( "Material color parse error" );
					}

					/* setup 0..255 range color */
					diffuseColor[ 0 ] = (int)( vec[ 0 ] * 255.0 );
					diffuseColor[ 1 ] = (int)( vec[ 1 ] * 255.0 );
					diffuseColor[ 2 ] = (int)( vec[ 2 ] * 255.0 );
					diffuseColor[ 3 ] = (int)( 255 );

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse specular material color */
				else if ( !_pico_stricmp( p->token,"*material_specular" ) ) {
					pmm::vec3_t vec;

					/* get r,g,b float values from ase */
					if ( !_pico_parse_vec( p,vec ) ) {
						_ase_error_return( "Material color parse error" );
					}

					/* setup 0..255 range color */
					specularColor[ 0 ] = (int)( vec[ 0 ] * 255 );
					specularColor[ 1 ] = (int)( vec[ 1 ] * 255 );
					specularColor[ 2 ] = (int)( vec[ 2 ] * 255 );
					specularColor[ 3 ] = (int)( 255 );

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* material diffuse map */
				else if ( !_pico_stricmp( p->token,"*map_diffuse" ) ) {
					int sublevel = 0;

					/* parse material block */
					while ( 1 )
					{
						/* get next token */
						if ( _pico_parse( p,1 ) == nullptr ) {
							break;
						}
						if ( !strlen( p->token ) ) {
							continue;
						}

						/* handle levels */
						if ( p->token[0] == '{' ) {
							sublevel++;
						}
						if ( p->token[0] == '}' ) {
							sublevel--;
						}
						if ( !sublevel ) {
							break;
						}

						/* parse diffuse map bitmap */
						if ( !_pico_stricmp( p->token,"*bitmap" ) ) {
							char* name = _pico_parse( p,0 );
							if ( name == nullptr ) {
								_ase_error_return( "Missing material map bitmap name" );
							}
							mapname = reinterpret_cast<decltype(mapname)>(pmm::man.pp_m_new( strlen( name ) + 1 ));
							strcpy( mapname, name );
							/* skip rest and continue with next token */
							_pico_parse_skip_rest( p );
							continue;
						}
					}
				}
				/* end map_diffuse block */
			}
			/* end material block */

			if ( subMaterial == nullptr ) {
				/* allocate new pico shader */
				shader = pmm::pp_new_shader( model );
				if ( shader == nullptr ) {
					pmm::pp_free_model( model );
					return nullptr;
				}

				/* set material name */
				shadername_convert( materialName );
				pmm::pp_set_shader_name( shader,materialName );

				/* set shader's transparency */
				pmm::pp_set_shader_transparency( shader,transValue );

				/* set shader's ambient color */
				pmm::pp_set_shader_ambient_color( shader,ambientColor );

				/* set diffuse alpha to transparency */
				diffuseColor[3] = (pmm::ub8_t)( transValue * 255.0 );

				/* set shader's diffuse color */
				pmm::pp_set_shader_diffuse_color( shader,diffuseColor );

				/* set shader's specular color */
				pmm::pp_set_shader_specular_color( shader,specularColor );

				/* set shader's shininess */
				pmm::pp_set_shader_shininess( shader,shineValue );

				/* set material map name */
				pmm::pp_set_shader_map_name( shader, mapname );

				/* extract shadername from bitmap path */
				if ( mapname != nullptr ) {
					char* p = mapname;

					/* convert to shader-name format */
					shadername_convert( mapname );
					{
						/* remove extension */
						char* last_period = strrchr( p, '.' );
						if ( last_period != nullptr ) {
							*last_period = '\0';
						}
					}

					/* find shader path */
					for (; *p != '\0'; ++p )
					{
						if ( _pico_strnicmp( p, "models/", 7 ) == 0 || _pico_strnicmp( p, "textures/", 9 ) == 0 ) {
							break;
						}
					}

					if ( *p != '\0' ) {
						/* set material name */
						pmm::pp_set_shader_name( shader,p );
					}
				}

				/* this is just a material with 1 submaterial */
				subMaterial = _ase_add_submaterial( &materials, index, 0, shader );
			}

			/* ydnar: free mapname */
			if ( mapname != nullptr ) {
				pmm::man.pp_m_delete( mapname );
			}
		}   // !_pico_stricmp ( "*material" )

		/* skip unparsed rest of line and continue */
		_pico_parse_skip_rest( p );
	}

	/* ydnar: finish existing surface */
	_ase_submit_triangles( model, materials, vertices, texcoords, colors, faces, numFaces, lastNodeName );
	pmm::man.pp_m_delete( faces );
	pmm::man.pp_m_delete( vertices );
	pmm::man.pp_m_delete( texcoords );
	pmm::man.pp_m_delete( colors );

#ifdef DEBUG_PM_ASE
	_ase_print_materials( materials );
	finish = clock();
	elapsed = (double)( finish - start ) / CLOCKS_PER_SEC;
	_pico_printf( pmm::pl_normal, "Loaded model in in %-.2f second(s)\n", elapsed );
#endif //DEBUG_PM_ASE

	_ase_free_materials( &materials );

	_pico_free_parser( p );

	/* return allocated pico model */
	return model;
}

/* pico file format module definition */
extern const pmm::module_t picoModuleASE =
{
	"1.0",                  /* module version string */
	"Autodesk 3DSMAX ASCII",    /* module display name */
	"Jared Hefty, seaw0lf",                 /* author's name */
	"2003 Jared Hefty, 2002 seaw0lf",               /* module copyright */
	{
		"ase",nullptr,nullptr,nullptr    /* default extensions to use */
	},
	_ase_canload,               /* validation routine */
	_ase_load,                  /* load routine */
	nullptr,                       /* save validation routine */
	nullptr                        /* save routine */
};

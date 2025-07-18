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

/*
   Nurail: Used pm_md3.c (Randy Reddig) as a template.
 */

#include <pmpmesh/pm_internal.hpp>
#include <sstream>

/* md2 model format */
const char *MD2_MAGIC             = "IDP2";
const int MD2_version             = 8;

#define MD2_NUMVERTEXNORMALS 162

#define MD2_MAX_TRIANGLES       4096
#define MD2_MAX_VERTS           2048
#define MD2_MAX_FRAMES          512
#define MD2_MAX_MD2SKINS        32
#define MD2_MAX_SKINNAME        64

class index_LUT_t
{
public:
	short Vert;
	short ST;
	index_LUT_t *next;

};

class index_DUP_LUT_t
{
public:
	short ST;
	short OldVert;

};

class md2St_t
{
public:
	short s;
	short t;
};

class md2Triangle_t
{
public:
	short index_xyz[3];
	short index_st[3];
};

class md2XyzNormal_t
{
public:
	pmm::ub8_t v[3];                          // scaled byte to fit in frame mins/maxs
	pmm::ub8_t lightnormalindex;
};

class md2Frame_t
{
public:
	float scale[3];                     // multiply byte verts by this
	float translate[3];                 // then add this
	char name[16];                      // frame name from grabbing
	md2XyzNormal_t verts[1];            // variable sized
};


/* md2 model file md2 class */
class md2_t
{
public:
	char magic[ 4 ];
	int version;

	int skinWidth;
	int skinHeight;
	int frameSize;

	int numSkins;
	int numXYZ;
	int numST;
	int numTris;
	int numGLCmds;
	int numFrames;

	int ofsSkins;
	int ofsST;
	int ofsTris;
	int ofsFrames;
	int ofsGLCmds;
	int ofsEnd;
};

float md2_normals[ MD2_NUMVERTEXNORMALS ][ 3 ] =
{
	{ -0.525731f, 0.000000f, 0.850651f },
	{ -0.442863f, 0.238856f, 0.864188f },
	{ -0.295242f, 0.000000f, 0.955423f },
	{ -0.309017f, 0.500000f, 0.809017f },
	{ -0.162460f, 0.262866f, 0.951056f },
	{ 0.000000f, 0.000000f, 1.000000f },
	{ 0.000000f, 0.850651f, 0.525731f },
	{ -0.147621f, 0.716567f, 0.681718f },
	{ 0.147621f, 0.716567f, 0.681718f },
	{ 0.000000f, 0.525731f, 0.850651f },
	{ 0.309017f, 0.500000f, 0.809017f },
	{ 0.525731f, 0.000000f, 0.850651f },
	{ 0.295242f, 0.000000f, 0.955423f },
	{ 0.442863f, 0.238856f, 0.864188f },
	{ 0.162460f, 0.262866f, 0.951056f },
	{ -0.681718f, 0.147621f, 0.716567f },
	{ -0.809017f, 0.309017f, 0.500000f },
	{ -0.587785f, 0.425325f, 0.688191f },
	{ -0.850651f, 0.525731f, 0.000000f },
	{ -0.864188f, 0.442863f, 0.238856f },
	{ -0.716567f, 0.681718f, 0.147621f },
	{ -0.688191f, 0.587785f, 0.425325f },
	{ -0.500000f, 0.809017f, 0.309017f },
	{ -0.238856f, 0.864188f, 0.442863f },
	{ -0.425325f, 0.688191f, 0.587785f },
	{ -0.716567f, 0.681718f, -0.147621f },
	{ -0.500000f, 0.809017f, -0.309017f },
	{ -0.525731f, 0.850651f, 0.000000f },
	{ 0.000000f, 0.850651f, -0.525731f },
	{ -0.238856f, 0.864188f, -0.442863f },
	{ 0.000000f, 0.955423f, -0.295242f },
	{ -0.262866f, 0.951056f, -0.162460f },
	{ 0.000000f, 1.000000f, 0.000000f },
	{ 0.000000f, 0.955423f, 0.295242f },
	{ -0.262866f, 0.951056f, 0.162460f },
	{ 0.238856f, 0.864188f, 0.442863f },
	{ 0.262866f, 0.951056f, 0.162460f },
	{ 0.500000f, 0.809017f, 0.309017f },
	{ 0.238856f, 0.864188f, -0.442863f },
	{ 0.262866f, 0.951056f, -0.162460f },
	{ 0.500000f, 0.809017f, -0.309017f },
	{ 0.850651f, 0.525731f, 0.000000f },
	{ 0.716567f, 0.681718f, 0.147621f },
	{ 0.716567f, 0.681718f, -0.147621f },
	{ 0.525731f, 0.850651f, 0.000000f },
	{ 0.425325f, 0.688191f, 0.587785f },
	{ 0.864188f, 0.442863f, 0.238856f },
	{ 0.688191f, 0.587785f, 0.425325f },
	{ 0.809017f, 0.309017f, 0.500000f },
	{ 0.681718f, 0.147621f, 0.716567f },
	{ 0.587785f, 0.425325f, 0.688191f },
	{ 0.955423f, 0.295242f, 0.000000f },
	{ 1.000000f, 0.000000f, 0.000000f },
	{ 0.951056f, 0.162460f, 0.262866f },
	{ 0.850651f, -0.525731f, 0.000000f },
	{ 0.955423f, -0.295242f, 0.000000f },
	{ 0.864188f, -0.442863f, 0.238856f },
	{ 0.951056f, -0.162460f, 0.262866f },
	{ 0.809017f, -0.309017f, 0.500000f },
	{ 0.681718f, -0.147621f, 0.716567f },
	{ 0.850651f, 0.000000f, 0.525731f },
	{ 0.864188f, 0.442863f, -0.238856f },
	{ 0.809017f, 0.309017f, -0.500000f },
	{ 0.951056f, 0.162460f, -0.262866f },
	{ 0.525731f, 0.000000f, -0.850651f },
	{ 0.681718f, 0.147621f, -0.716567f },
	{ 0.681718f, -0.147621f, -0.716567f },
	{ 0.850651f, 0.000000f, -0.525731f },
	{ 0.809017f, -0.309017f, -0.500000f },
	{ 0.864188f, -0.442863f, -0.238856f },
	{ 0.951056f, -0.162460f, -0.262866f },
	{ 0.147621f, 0.716567f, -0.681718f },
	{ 0.309017f, 0.500000f, -0.809017f },
	{ 0.425325f, 0.688191f, -0.587785f },
	{ 0.442863f, 0.238856f, -0.864188f },
	{ 0.587785f, 0.425325f, -0.688191f },
	{ 0.688191f, 0.587785f, -0.425325f },
	{ -0.147621f, 0.716567f, -0.681718f },
	{ -0.309017f, 0.500000f, -0.809017f },
	{ 0.000000f, 0.525731f, -0.850651f },
	{ -0.525731f, 0.000000f, -0.850651f },
	{ -0.442863f, 0.238856f, -0.864188f },
	{ -0.295242f, 0.000000f, -0.955423f },
	{ -0.162460f, 0.262866f, -0.951056f },
	{ 0.000000f, 0.000000f, -1.000000f },
	{ 0.295242f, 0.000000f, -0.955423f },
	{ 0.162460f, 0.262866f, -0.951056f },
	{ -0.442863f, -0.238856f, -0.864188f },
	{ -0.309017f, -0.500000f, -0.809017f },
	{ -0.162460f, -0.262866f, -0.951056f },
	{ 0.000000f, -0.850651f, -0.525731f },
	{ -0.147621f, -0.716567f, -0.681718f },
	{ 0.147621f, -0.716567f, -0.681718f },
	{ 0.000000f, -0.525731f, -0.850651f },
	{ 0.309017f, -0.500000f, -0.809017f },
	{ 0.442863f, -0.238856f, -0.864188f },
	{ 0.162460f, -0.262866f, -0.951056f },
	{ 0.238856f, -0.864188f, -0.442863f },
	{ 0.500000f, -0.809017f, -0.309017f },
	{ 0.425325f, -0.688191f, -0.587785f },
	{ 0.716567f, -0.681718f, -0.147621f },
	{ 0.688191f, -0.587785f, -0.425325f },
	{ 0.587785f, -0.425325f, -0.688191f },
	{ 0.000000f, -0.955423f, -0.295242f },
	{ 0.000000f, -1.000000f, 0.000000f },
	{ 0.262866f, -0.951056f, -0.162460f },
	{ 0.000000f, -0.850651f, 0.525731f },
	{ 0.000000f, -0.955423f, 0.295242f },
	{ 0.238856f, -0.864188f, 0.442863f },
	{ 0.262866f, -0.951056f, 0.162460f },
	{ 0.500000f, -0.809017f, 0.309017f },
	{ 0.716567f, -0.681718f, 0.147621f },
	{ 0.525731f, -0.850651f, 0.000000f },
	{ -0.238856f, -0.864188f, -0.442863f },
	{ -0.500000f, -0.809017f, -0.309017f },
	{ -0.262866f, -0.951056f, -0.162460f },
	{ -0.850651f, -0.525731f, 0.000000f },
	{ -0.716567f, -0.681718f, -0.147621f },
	{ -0.716567f, -0.681718f, 0.147621f },
	{ -0.525731f, -0.850651f, 0.000000f },
	{ -0.500000f, -0.809017f, 0.309017f },
	{ -0.238856f, -0.864188f, 0.442863f },
	{ -0.262866f, -0.951056f, 0.162460f },
	{ -0.864188f, -0.442863f, 0.238856f },
	{ -0.809017f, -0.309017f, 0.500000f },
	{ -0.688191f, -0.587785f, 0.425325f },
	{ -0.681718f, -0.147621f, 0.716567f },
	{ -0.442863f, -0.238856f, 0.864188f },
	{ -0.587785f, -0.425325f, 0.688191f },
	{ -0.309017f, -0.500000f, 0.809017f },
	{ -0.147621f, -0.716567f, 0.681718f },
	{ -0.425325f, -0.688191f, 0.587785f },
	{ -0.162460f, -0.262866f, 0.951056f },
	{ 0.442863f, -0.238856f, 0.864188f },
	{ 0.162460f, -0.262866f, 0.951056f },
	{ 0.309017f, -0.500000f, 0.809017f },
	{ 0.147621f, -0.716567f, 0.681718f },
	{ 0.000000f, -0.525731f, 0.850651f },
	{ 0.425325f, -0.688191f, 0.587785f },
	{ 0.587785f, -0.425325f, 0.688191f },
	{ 0.688191f, -0.587785f, 0.425325f },
	{ -0.955423f, 0.295242f, 0.000000f },
	{ -0.951056f, 0.162460f, 0.262866f },
	{ -1.000000f, 0.000000f, 0.000000f },
	{ -0.850651f, 0.000000f, 0.525731f },
	{ -0.955423f, -0.295242f, 0.000000f },
	{ -0.951056f, -0.162460f, 0.262866f },
	{ -0.864188f, 0.442863f, -0.238856f },
	{ -0.951056f, 0.162460f, -0.262866f },
	{ -0.809017f, 0.309017f, -0.500000f },
	{ -0.864188f, -0.442863f, -0.238856f },
	{ -0.951056f, -0.162460f, -0.262866f },
	{ -0.809017f, -0.309017f, -0.500000f },
	{ -0.681718f, 0.147621f, -0.716567f },
	{ -0.681718f, -0.147621f, -0.716567f },
	{ -0.850651f, 0.000000f, -0.525731f },
	{ -0.688191f, 0.587785f, -0.425325f },
	{ -0.587785f, 0.425325f, -0.688191f },
	{ -0.425325f, 0.688191f, -0.587785f },
	{ -0.425325f, -0.688191f, -0.587785f },
	{ -0.587785f, -0.425325f, -0.688191f },
	{ -0.688191f, -0.587785f, -0.425325f },
};


// _md2_canload()

static int _md2_canload( PM_PARAMS_CANLOAD ){
	const md2_t *md2;

	/* sanity check */
	if ( (pmm::size_type) bufSize < ( sizeof( *md2 ) * 2 ) ) {
		return pmm::pmv_error_size;
	}

	/* set as md2 */
	md2 = (const md2_t*) buffer;

	/* check md2 magic */
	if ( *( (const int*) md2->magic ) != *( (const int*) MD2_MAGIC ) ) {
		return pmm::pmv_error_ident;
	}

	/* check md2 version */
	if ( _pico_little_long( md2->version ) != MD2_version ) {
		return pmm::pmv_error_version;
	}

	/* file seems to be a valid md2 */
	return pmm::pmv_ok;
}



// _md2_load() loads a quake2 md2 model file.


static pmm::model_t *_md2_load( PM_PARAMS_LOAD ){
	int i, j, dups, dup_index;
	index_LUT_t     *p_index_LUT, *p_index_LUT2, *p_index_LUT3;
	index_DUP_LUT_t *p_index_LUT_DUPS;
	md2Triangle_t   *p_md2Triangle;

	char skinname[ MD2_MAX_SKINNAME ];
	md2_t           *md2;
	md2St_t         *texCoord;
	md2Frame_t      *frame;
	md2Triangle_t   *triangle;
	md2XyzNormal_t  *vertex;

	pmm::ub8_t      *bb, *bb0;
	pmm::model_t     *picoModel;
	pmm::surface_t   *picoSurface;
	pmm::shader_t    *picoShader;
	pmm::vec3_t xyz, normal;
	pmm::vec2_t st;
	pmm::color_t color;


	/* set as md2 */
	bb0 = bb = (pmm::ub8_t*) pmm::man.pp_m_new( bufSize );
	memcpy( bb, buffer, bufSize );
	md2 = (md2_t*) bb;

	/* check ident and version */
	if ( *( (const int*) md2->magic ) != *( (const int*) MD2_MAGIC ) || _pico_little_long( md2->version ) != MD2_version ) {
		/* not an md2 file (todo: set error) */
		pmm::man.pp_print(
			pmm::pl_error,
			(std::ostringstream{} << fileName << " is not an MD2 File!").str()
		);
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// swap md2
	md2->version = _pico_little_long( md2->version );

	md2->skinWidth = _pico_little_long( md2->skinWidth );
	md2->skinHeight = _pico_little_long( md2->skinHeight );
	md2->frameSize = _pico_little_long( md2->frameSize );

	md2->numSkins = _pico_little_long( md2->numSkins );
	md2->numXYZ = _pico_little_long( md2->numXYZ );
	md2->numST = _pico_little_long( md2->numST );
	md2->numTris = _pico_little_long( md2->numTris );
	md2->numGLCmds = _pico_little_long( md2->numGLCmds );
	md2->numFrames = _pico_little_long( md2->numFrames );

	md2->ofsSkins = _pico_little_long( md2->ofsSkins );
	md2->ofsST = _pico_little_long( md2->ofsST );
	md2->ofsTris = _pico_little_long( md2->ofsTris );
	md2->ofsFrames = _pico_little_long( md2->ofsFrames );
	md2->ofsGLCmds = _pico_little_long( md2->ofsGLCmds );
	md2->ofsEnd = _pico_little_long( md2->ofsEnd );

	// do frame check
	if (md2->numFrames < 1)
	{
		pmm::man.pp_print(
			pmm::pl_error,
			(std::ostringstream{} << fileName << " has 0 frames!").str()
		);
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( frameNum < 0 || frameNum >= md2->numFrames ) {
		pmm::man.pp_print(
			pmm::pl_error,
			"Invalid or out-of-range MD2 frame specified"
		);
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// Setup Frame
	frame = (md2Frame_t *) ( bb + md2->ofsFrames + ( sizeof( md2Frame_t ) * frameNum ) );

	// swap frame scale and translation
	for ( i = 0; i < 3; i++ )
	{
		frame->scale[ i ] = _pico_little_float( frame->scale[ i ] );
		frame->translate[ i ] = _pico_little_float( frame->translate[ i ] );
	}

	// swap triangles
	triangle = (md2Triangle_t *) ( (pmm::ub8_t *) ( bb + md2->ofsTris ) );
	for ( i = 0; i < md2->numTris; i++, triangle++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			triangle->index_xyz[ j ] = _pico_little_short( triangle->index_xyz[ j ] );
			triangle->index_st[ j ] = _pico_little_short( triangle->index_st[ j ] );
		}
	}

	// swap st coords
	texCoord = (md2St_t*) ( (pmm::ub8_t *) ( bb + md2->ofsST ) );
	for ( i = 0; i < md2->numST; i++, texCoord++ )
	{
		texCoord->s = _pico_little_short( texCoord->s );
		texCoord->t = _pico_little_short( texCoord->t );
	}

	// set Skin Name
	strncpy( skinname, (const char *) ( bb + md2->ofsSkins ), MD2_MAX_SKINNAME );

	// Print out md2 values
	pmm::man.pp_print(
		pmm::pl_verbose,
		(
			std::ostringstream{}
				<< "Skins: " << md2->numSkins << "  "
				<< "Verts: " << md2->numXYZ << "  "
				<< "STs: " << md2->numST << "  "
				<< "Triangles: " << md2->numTris << "  "
				<< "Frames: " << md2->numFrames << "\n"
				<< "Skin Name \"" << &skinname << "\"\n"
		).str()
	);

	// detox Skin name
	_pico_setfext( skinname, "" );
	_pico_unixify( skinname );

	/* create new pico model */
	picoModel = pmm::pp_new_model();
	if ( picoModel == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	/* do model setup */
	pmm::pp_set_model_frame_num( picoModel, frameNum );
	pmm::pp_set_model_num_frames( picoModel, md2->numFrames ); /* sea */
	pmm::pp_set_model_name( picoModel, fileName );
	pmm::pp_set_model_file_name( picoModel, fileName );

	// allocate new pico surface
	picoSurface = pmm::pp_new_surface( picoModel );
	if ( picoSurface == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model surface");
		pmm::pp_free_model( picoModel );
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}


	pmm::pp_set_surface_type( picoSurface, pmm::st_triangles );
	pmm::pp_set_surface_name( picoSurface, frame->name );
	picoShader = pmm::pp_new_shader( picoModel );
	if ( picoShader == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model shader");
		pmm::pp_free_model( picoModel );
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	pmm::pp_set_shader_name( picoShader, skinname );

	// associate current surface with newly created shader
	pmm::pp_set_surface_shader( picoSurface, picoShader );

	// Init LUT for Verts
	p_index_LUT = (index_LUT_t *)pmm::man.pp_m_new( sizeof( index_LUT_t ) * md2->numXYZ );
	for ( i = 0; i < md2->numXYZ; i++ )
	{
		p_index_LUT[i].Vert = -1;
		p_index_LUT[i].ST = -1;
		p_index_LUT[i].next = nullptr;
	}

	// Fill in Look Up Table, and allocate/fill Linked List from vert array as needed for dup STs per Vert.
	dups = 0;
	for ( i = 0; i < md2->numTris; i++ )
	{
		p_md2Triangle = (md2Triangle_t *) ( bb + md2->ofsTris + ( sizeof( md2Triangle_t ) * i ) );
		for ( j = 0; j < 3; j++ )
		{
			if ( p_index_LUT[p_md2Triangle->index_xyz[j]].ST == -1 ) { // No Main Entry
				p_index_LUT[p_md2Triangle->index_xyz[j]].ST = p_md2Triangle->index_st[j];
			}

			else if ( p_md2Triangle->index_st[j] == p_index_LUT[p_md2Triangle->index_xyz[j]].ST ) { // Equal to Main Entry
				continue;
			}

			else if ( ( p_index_LUT[p_md2Triangle->index_xyz[j]].next == nullptr ) ) { // Not equal to Main entry, and no LL entry
				// Add first entry of LL from Main
				p_index_LUT2 = (index_LUT_t *)pmm::man.pp_m_new( sizeof( index_LUT_t ) );
				if ( p_index_LUT2 == nullptr ) {
					pmm::man.pp_print(pmm::pl_error, " Couldn't allocate memory!\n");
				}
				p_index_LUT[p_md2Triangle->index_xyz[j]].next = (index_LUT_t *)p_index_LUT2;
				p_index_LUT2->Vert = dups;
				p_index_LUT2->ST = p_md2Triangle->index_st[j];
				p_index_LUT2->next = nullptr;
				p_md2Triangle->index_xyz[j] = dups + md2->numXYZ; // Make change in Tri hunk
				dups++;
			}
			else // Try to find in LL from Main Entry
			{
				p_index_LUT3 = p_index_LUT2 = p_index_LUT[p_md2Triangle->index_xyz[j]].next;
				while ( ( p_index_LUT2 != nullptr ) && ( p_md2Triangle->index_xyz[j] != p_index_LUT2->Vert ) ) // Walk down LL
				{
					p_index_LUT3 = p_index_LUT2;
					p_index_LUT2 = p_index_LUT2->next;
				}
				p_index_LUT2 = p_index_LUT3;

				if ( p_md2Triangle->index_st[j] == p_index_LUT2->ST ) { // Found it
					p_md2Triangle->index_xyz[j] = p_index_LUT2->Vert + md2->numXYZ; // Make change in Tri hunk
					continue;
				}

				if ( p_index_LUT2->next == nullptr ) { // Didn't find it. Add entry to LL.
					// Add the Entry
					p_index_LUT3 = (index_LUT_t *)pmm::man.pp_m_new( sizeof( index_LUT_t ) );
					if ( p_index_LUT3 == nullptr ) {
						pmm::man.pp_print(pmm::pl_error, " Couldn't allocate memory!\n");
					}
					p_index_LUT2->next = (index_LUT_t *)p_index_LUT3;
					p_index_LUT3->Vert = p_md2Triangle->index_xyz[j];
					p_index_LUT3->ST = p_md2Triangle->index_st[j];
					p_index_LUT3->next = nullptr;
					p_md2Triangle->index_xyz[j] = dups + md2->numXYZ; // Make change in Tri hunk
					dups++;
				}
			}
		}
	}

	// malloc and build array for Dup STs
	p_index_LUT_DUPS = (index_DUP_LUT_t *)pmm::man.pp_m_new( sizeof( index_DUP_LUT_t ) * dups );
	if ( p_index_LUT_DUPS == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, " Couldn't allocate memory!\n");
	}

	dup_index = 0;
	for ( i = 0; i < md2->numXYZ; i++ )
	{
		p_index_LUT2 = p_index_LUT[i].next;
		while ( p_index_LUT2 != nullptr )
		{
			p_index_LUT_DUPS[p_index_LUT2->Vert].OldVert = i;
			p_index_LUT_DUPS[p_index_LUT2->Vert].ST = p_index_LUT2->ST;
			dup_index++;
			p_index_LUT2 = p_index_LUT2->next;
		}
	}

	// Build Picomodel
	triangle = (md2Triangle_t *) ( (pmm::ub8_t *) ( bb + md2->ofsTris ) );
	texCoord = (md2St_t*) ( (pmm::ub8_t *) ( bb + md2->ofsST ) );
	vertex = (md2XyzNormal_t*) ( (pmm::ub8_t*) ( frame->verts ) );
	for ( j = 0; j < md2->numTris; j++, triangle++ )
	{
		pmm::pp_set_surface_index( picoSurface, j * 3, triangle->index_xyz[0] );
		pmm::pp_set_surface_index( picoSurface, j * 3 + 1, triangle->index_xyz[1] );
		pmm::pp_set_surface_index( picoSurface, j * 3 + 2, triangle->index_xyz[2] );
	}

	for ( i = 0; i < md2->numXYZ; i++, vertex++ )
	{
		/* set vertex origin */
		xyz[ 0 ] = vertex->v[0] * frame->scale[0] + frame->translate[0];
		xyz[ 1 ] = vertex->v[1] * frame->scale[1] + frame->translate[1];
		xyz[ 2 ] = vertex->v[2] * frame->scale[2] + frame->translate[2];
		pmm::pp_set_surface_xyz( picoSurface, i, xyz );

		/* set normal */
		normal[ 0 ] = md2_normals[vertex->lightnormalindex][0];
		normal[ 1 ] = md2_normals[vertex->lightnormalindex][1];
		normal[ 2 ] = md2_normals[vertex->lightnormalindex][2];
		pmm::pp_set_surface_normal( picoSurface, i, normal );

		/* set st coords */
		st[ 0 ] =  ( ( texCoord[p_index_LUT[i].ST].s ) / ( (float)md2->skinWidth ) );
		st[ 1 ] =  ( texCoord[p_index_LUT[i].ST].t / ( (float)md2->skinHeight ) );
		pmm::pp_set_surface_st( picoSurface, 0, i, st );
	}

	if ( dups ) {
		for ( i = 0; i < dups; i++ )
		{
			j = p_index_LUT_DUPS[i].OldVert;
			/* set vertex origin */
			xyz[ 0 ] = frame->verts[j].v[0] * frame->scale[0] + frame->translate[0];
			xyz[ 1 ] = frame->verts[j].v[1] * frame->scale[1] + frame->translate[1];
			xyz[ 2 ] = frame->verts[j].v[2] * frame->scale[2] + frame->translate[2];
			pmm::pp_set_surface_xyz( picoSurface, i + md2->numXYZ, xyz );

			/* set normal */
			normal[ 0 ] = md2_normals[frame->verts[j].lightnormalindex][0];
			normal[ 1 ] = md2_normals[frame->verts[j].lightnormalindex][1];
			normal[ 2 ] = md2_normals[frame->verts[j].lightnormalindex][2];
			pmm::pp_set_surface_normal( picoSurface, i + md2->numXYZ, normal );

			/* set st coords */
			st[ 0 ] =  ( ( texCoord[p_index_LUT_DUPS[i].ST].s ) / ( (float)md2->skinWidth ) );
			st[ 1 ] =  ( texCoord[p_index_LUT_DUPS[i].ST].t / ( (float)md2->skinHeight ) );
			pmm::pp_set_surface_st( picoSurface, 0, i + md2->numXYZ, st );
		}
	}

	/* set color */
	pmm::pp_set_surface_color( picoSurface, 0, 0, color );

	// Free up malloc'ed LL entries
	for ( i = 0; i < md2->numXYZ; i++ )
	{
		if ( p_index_LUT[i].next != nullptr ) {
			p_index_LUT2 = p_index_LUT[i].next;
			do {
				p_index_LUT3 = p_index_LUT2->next;
				pmm::man.pp_m_delete( p_index_LUT2 );
				p_index_LUT2 = p_index_LUT3;
				dups--;
			} while ( p_index_LUT2 != nullptr );
		}
	}

	if ( dups ) {
		pmm::man.pp_print(pmm::pl_warning, " Not all LL mallocs freed\n");
	}

	// Free malloc'ed LUTs
	pmm::man.pp_m_delete( p_index_LUT );
	pmm::man.pp_m_delete( p_index_LUT_DUPS );

	/* return the new pico model */
	pmm::man.pp_m_delete( bb0 );
	return picoModel;

}



/* pico file format module definition */
extern const pmm::module_t picoModuleMD2 =
{
	"0.875",                        /* module version string */
	"Quake 2 MD2",                  /* module display name */
	"Nurail",                       /* author's name */
	"2003 Nurail",                  /* module copyright */
	{
		"md2", nullptr, nullptr, nullptr     /* default extensions to use */
	},
	_md2_canload,                   /* validation routine */
	_md2_load,                      /* load routine */
	nullptr,                           /* save validation routine */
	nullptr                            /* save routine */
};

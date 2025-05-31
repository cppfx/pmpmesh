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

#include <pmpmesh/pm_fm.hpp>
#include <pmpmesh/pmpmesh.hpp>
#include <sstream>
#include <iomanip>

//#define FM_VERBOSE_DBG	0
#undef FM_VERBOSE_DBG
#undef FM_DBG

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

// _fm_canload()
static int _fm_canload( PM_PARAMS_CANLOAD ){
	fm_t fm;
	unsigned char   *bb, *bb0;
	int fm_file_pos;

	bb0 = bb = (pmm::ub8_t*) pmm::man.pp_m_new( bufSize );
	memcpy( bb, buffer, bufSize );

	// Header
	fm.fm_header_hdr = (fm_chunk_header_t *) bb;
	fm_file_pos = sizeof( fm_chunk_header_t ) + fm.fm_header_hdr->size;
#ifdef FM_VERBOSE_DBG
	pmm::man.pp_print(pmm::pl_verbose,
		(std::ostringstream{} << "ident: " << (unsigned char *)fm.fm_header_hdr->ident << "\n").str()
	);
#endif
	if ( ( strcmp( fm.fm_header_hdr->ident, FM_HEADERCHUNKNAME ) )  ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Header Ident incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_ident;
	}

	// check fm
	if ( _pico_little_long( fm.fm_header_hdr->version ) != FM_HEADERCHUNKVER ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Header Version incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_version;
	}

	// Skin
	fm.fm_skin_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_skin_hdr->size;
#ifdef FM_VERBOSE_DBG
	pmm::man.pp_print(pmm::pl_verbose,
		(std::ostringstream{} << "SKIN: " << (unsigned char *) fm.fm_skin_hdr->ident) << "\n").str());
#endif
	if ( ( strcmp( fm.fm_skin_hdr->ident, FM_SKINCHUNKNAME ) ) ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Skin Ident incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_ident;
	}

	// check fm
	if ( _pico_little_long( fm.fm_skin_hdr->version ) != FM_SKINCHUNKVER ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Skin Version incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_version;
	}

	// st
	fm.fm_st_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_st_hdr->size;
#ifdef FM_VERBOSE_DBG
	pmm::man.pp_print(
		pmm::pl_verbose,
		(
			std::ostringstream{} << "ST: " << (unsigned char *) fm.fm_st_hdr->ident << "\n"
		).str()
	);
#endif
	if ( ( strcmp( fm.fm_st_hdr->ident, FM_STCOORDCHUNKNAME ) ) ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM ST Ident incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_ident;
	}

	// check fm
	if ( _pico_little_long( fm.fm_st_hdr->version ) != FM_STCOORDCHUNKVER ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM ST Version incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_version;
	}

	// tri
	fm.fm_tri_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_tri_hdr->size;
#ifdef FM_VERBOSE_DBG
	pmm::man.pp_print(
		pmm::pl_verbose,
		(
			std::ostringstream{}
				<< "TRI: " << (unsigned char *) fm.fm_tri_hdr->ident << "\n"
		).str()
	);
#endif
	if ( ( strcmp( fm.fm_tri_hdr->ident, FM_TRISCHUNKNAME ) ) ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Tri Ident incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_ident;
	}

	// check fm
	if ( _pico_little_long( fm.fm_tri_hdr->version ) != FM_TRISCHUNKVER ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Tri Version incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_version;
	}

	// frame
	fm.fm_frame_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t );
#ifdef FM_VERBOSE_DBG
	pmm::man.pp_print(
		pmm::pl_verbose,
		(
			std::ostringstream{}
				<< "FRAME: " << (unsigned char *) fm.fm_frame_hdr->ident << "\n"
		).str()
	);
#endif
	if ( ( strcmp( fm.fm_frame_hdr->ident, FM_FRAMESCHUNKNAME ) ) ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Frame Ident incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_ident;
	}

	// check fm
	if ( _pico_little_long( fm.fm_frame_hdr->version ) != FM_FRAMESCHUNKVER ) {
#ifdef FM_DBG
		pmm::man.pp_print(pmm::pl_warning, "FM Frame Version incorrect\n");
#endif
		pmm::man.pp_m_delete( bb0 );
		return pmm::pmv_error_version;
	}

	// file seems to be a valid fm
	return pmm::pmv_ok;
}



// _fm_load() loads a Heretic 2 model file.
static pmm::model_t *_fm_load( PM_PARAMS_LOAD ){
	int i, j, dups, dup_index;
	int fm_file_pos;
	index_LUT_t     *p_index_LUT, *p_index_LUT2, *p_index_LUT3;
	index_DUP_LUT_t *p_index_LUT_DUPS;

	fm_vert_normal_t    *vert;

	char skinname[FM_SKINPATHsize];
	fm_t fm;
	fm_header_t     *fm_head;
	fm_st_t         *texCoord;
	fm_xyz_st_t     *tri_verts;
	fm_xyz_st_t     *triangle;
	fm_frame_t      *frame;

	pmm::ub8_t      *bb, *bb0;
	pmm::model_t *picoModel;
	pmm::surface_t   *picoSurface;
	pmm::shader_t    *picoShader;
	pmm::vec3_t xyz, normal;
	pmm::vec2_t st;
	pmm::color_t color;


	bb0 = bb = (pmm::ub8_t*) pmm::man.pp_m_new( bufSize );
	memcpy( bb, buffer, bufSize );

	// Header Header
	fm.fm_header_hdr = (fm_chunk_header_t *) bb;
	fm_file_pos = sizeof( fm_chunk_header_t ) + fm.fm_header_hdr->size;
	if ( ( strcmp( fm.fm_header_hdr->ident, FM_HEADERCHUNKNAME ) )  ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Header Ident incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( _pico_little_long( fm.fm_header_hdr->version ) != FM_HEADERCHUNKVER ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Header Version incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// Skin Header
	fm.fm_skin_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_skin_hdr->size;
	if ( ( strcmp( fm.fm_skin_hdr->ident, FM_SKINCHUNKNAME ) ) ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Skin Ident incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( _pico_little_long( fm.fm_skin_hdr->version ) != FM_SKINCHUNKVER ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Skin Version incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// ST Header
	fm.fm_st_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_st_hdr->size;
	if ( ( strcmp( fm.fm_st_hdr->ident, FM_STCOORDCHUNKNAME ) ) ) {
		pmm::man.pp_print(pmm::pl_warning, "FM ST Ident incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( _pico_little_long( fm.fm_st_hdr->version ) != FM_STCOORDCHUNKVER ) {
		pmm::man.pp_print(pmm::pl_warning, "FM ST Version incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// Tris Header
	fm.fm_tri_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_tri_hdr->size;
	if ( ( strcmp( fm.fm_tri_hdr->ident, FM_TRISCHUNKNAME ) ) ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Tri Ident incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( _pico_little_long( fm.fm_tri_hdr->version ) != FM_TRISCHUNKVER ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Tri Version incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// Frame Header
	fm.fm_frame_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t );
	if ( ( strcmp( fm.fm_frame_hdr->ident, FM_FRAMESCHUNKNAME ) ) ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Frame Ident incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( _pico_little_long( fm.fm_frame_hdr->version ) != FM_FRAMESCHUNKVER ) {
		pmm::man.pp_print(pmm::pl_warning, "FM Frame Version incorrect\n");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// Header
	fm_file_pos = sizeof( fm_chunk_header_t );
	fm_head = fm.fm_header = (fm_header_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_header_hdr->size;

	// Skin
	fm_file_pos += sizeof( fm_chunk_header_t );
	fm.fm_skin = (fm_skinpath_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_skin_hdr->size;

	// ST
	fm_file_pos += sizeof( fm_chunk_header_t );
	texCoord = fm.fm_st = (fm_st_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_st_hdr->size;

	// Tri
	fm_file_pos += sizeof( fm_chunk_header_t );
	tri_verts = fm.fm_tri = (fm_xyz_st_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_tri_hdr->size;

	// Frame
	fm_file_pos += sizeof( fm_chunk_header_t );
	frame = fm.fm_frame = (fm_frame_t *) ( bb + fm_file_pos );

	// do frame check
	if ( fm_head->numFrames < 1 ) {
		pmm::man.pp_print(pmm::pl_error, (std::ostringstream{} << fileName << " has 0 frames!").str());
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( frameNum < 0 || frameNum >= fm_head->numFrames ) {
		pmm::man.pp_print(pmm::pl_error, "Invalid or out-of-range FM frame specified");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	// swap fm
	fm_head->skinWidth = _pico_little_long( fm_head->skinWidth );
	fm_head->skinHeight = _pico_little_long( fm_head->skinHeight );
	fm_head->frameSize = _pico_little_long( fm_head->frameSize );

	fm_head->numSkins = _pico_little_long( fm_head->numSkins );
	fm_head->numXYZ = _pico_little_long( fm_head->numXYZ );
	fm_head->numST = _pico_little_long( fm_head->numST );
	fm_head->numTris = _pico_little_long( fm_head->numTris );
	fm_head->numGLCmds = _pico_little_long( fm_head->numGLCmds );
	fm_head->numFrames = _pico_little_long( fm_head->numFrames );

	// swap frame scale and translation
	for ( i = 0; i < 3; i++ )
	{
		frame->header.scale[ i ] = _pico_little_float( frame->header.scale[ i ] );
		frame->header.translate[ i ] = _pico_little_float( frame->header.translate[ i ] );
	}

	// swap triangles
	triangle = tri_verts;
	for ( i = 0; i < fm_head->numTris; i++, triangle++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			triangle->index_xyz[ j ] = _pico_little_short( triangle->index_xyz[ j ] );
			triangle->index_st[ j ] = _pico_little_short( triangle->index_st[ j ] );
		}
	}

	// swap st coords
	for ( i = 0; i < fm_head->numST; i++ )
	{
		texCoord->s = _pico_little_short( texCoord[i].s );
		texCoord->t = _pico_little_short( texCoord[i].t );
	}
	// set Skin Name
	strncpy( skinname, (const char *) fm.fm_skin, FM_SKINPATHsize );

#ifdef FM_VERBOSE_DBG
	// Print out md2 values
	pmm::man.pp_print(
		pmm::pl_verbose,
		(
			std::ostringstream
				<< "numSkins->" << fm_head->numSkins << "  "
				<< "numXYZ->" << fm_head->numXYZ << "  "
				<< "numST->" << fm_head->numST << "  "
				<< "numTris->" << fm_head->numTris << "  "
				<< "numFrames->" << fm_head->numFrames << "\n"
				<< "Skin Name \"" << &skinname << "\"\n"
		).str()
	);
#endif

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
	pmm::pp_set_model_num_frames( picoModel, fm_head->numFrames ); /* sea */
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
	pmm::pp_set_surface_name( picoSurface, frame->header.name );
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
	p_index_LUT = (index_LUT_t *)pmm::man.pp_m_new( sizeof( index_LUT_t ) * fm_head->numXYZ );
	for ( i = 0; i < fm_head->numXYZ; i++ )
	{
		p_index_LUT[i].Vert = -1;
		p_index_LUT[i].ST = -1;
		p_index_LUT[i].next = nullptr;
	}

	// Fill in Look Up Table, and allocate/fill Linked List from vert array as needed for dup STs per Vert.
	dups = 0;
	triangle = tri_verts;

	for ( i = 0; i < fm_head->numTris; i++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			if ( p_index_LUT[triangle->index_xyz[j]].ST == -1 ) { // No Main Entry
				p_index_LUT[triangle->index_xyz[j]].ST = triangle->index_st[j];
			}

			else if ( triangle->index_st[j] == p_index_LUT[triangle->index_xyz[j]].ST ) { // Equal to Main Entry
#ifdef FM_VERBOSE_DBG
				pmm::man.pp_print(
					pmm::pl_normal,
					(
						std::ostringstream{}
							<< "-> Tri #" << i << ", "
							<< "Vert " << j << ":\t "
							<< "XYZ:" << triangle->index_xyz[j] << "   "
							<< "ST:" << triangle->index_st[j] << "\n"
					).str()
				);
#endif
				continue;
			}
			else if ( ( p_index_LUT[triangle->index_xyz[j]].next == nullptr ) ) { // Not equal to Main entry, and no LL entry
				// Add first entry of LL from Main
				p_index_LUT2 = (index_LUT_t *)pmm::man.pp_m_new( sizeof( index_LUT_t ) );
				if ( p_index_LUT2 == nullptr ) {
					pmm::man.pp_print(pmm::pl_normal, " Couldn't allocate memory!\n");
				}
				p_index_LUT[triangle->index_xyz[j]].next = (index_LUT_t *)p_index_LUT2;
				p_index_LUT2->Vert = dups;
				p_index_LUT2->ST = triangle->index_st[j];
				p_index_LUT2->next = nullptr;
#ifdef FM_VERBOSE_DBG
				pmm::man.pp_print(pmm::pl_normal,
					(
						std::ostringstream{}
							<< " ADDING first LL XYZ:" << triangle->index_xyz[j] << " "
							<< "DUP:" << dups << " "
							<< "ST:" << triangle->index_st[j] << "\n"
					).str()
				);
#endif
				triangle->index_xyz[j] = dups + fm_head->numXYZ; // Make change in Tri hunk
				dups++;
			}
			else // Try to find in LL from Main Entry
			{
				p_index_LUT3 = p_index_LUT2 = p_index_LUT[triangle->index_xyz[j]].next;
				while ( ( p_index_LUT2 != nullptr ) && ( triangle->index_xyz[j] != p_index_LUT2->Vert ) ) // Walk down LL
				{
					p_index_LUT3 = p_index_LUT2;
					p_index_LUT2 = p_index_LUT2->next;
				}
				p_index_LUT2 = p_index_LUT3;

				if ( triangle->index_st[j] == p_index_LUT2->ST ) { // Found it
					triangle->index_xyz[j] = p_index_LUT2->Vert + fm_head->numXYZ; // Make change in Tri hunk
#ifdef FM_VERBOSE_DBG
					pmm::man.pp_print(
						pmm::pl_normal,
						(
							std::ostringstream{}
								<< "--> Tri #" << i << ", "
								<< "Vert " << j << ":\t "
								<< "XYZ:" << triangle->index_xyz[j] << "   "
								<< "ST:" << triangle->index_st[j] << "\n"
						).str()
					);
#endif
					continue;
				}

				if ( p_index_LUT2->next == nullptr ) { // Didn't find it. Add entry to LL.
					// Add the Entry
					p_index_LUT3 = (index_LUT_t *)pmm::man.pp_m_new( sizeof( index_LUT_t ) );
					if ( p_index_LUT3 == nullptr ) {
						pmm::man.pp_print(pmm::pl_normal, " Couldn't allocate memory!\n");
					}
					p_index_LUT2->next = (index_LUT_t *)p_index_LUT3;
					p_index_LUT3->Vert = dups;
					p_index_LUT3->ST = triangle->index_st[j];
					p_index_LUT3->next = nullptr;
#ifdef FM_VERBOSE_DBG
					pmm::man.pp_print(
						pmm::pl_normal,
						(
							std::ostringstream{}
								<< " ADDING additional LL XYZ:" << triangle->index_xyz[j] << " "
								<< "DUP:" << dups << " "
								<< "NewXYZ:" << dups + ( fm_head->numXYZ ) << " "
								<< "ST:" << triangle->index_st[j] << "\n"
						).str()
					);
#endif
					triangle->index_xyz[j] = dups + fm_head->numXYZ; // Make change in Tri hunk
					dups++;
				}
			}
#ifdef FM_VERBOSE_DBG
			pmm::man.pp_print(
				pmm::pl_normal,
				(
					std::ostringstream{}
						<< "---> Tri #" << i << ", "
						<< "Vert " << j << ":\t "
						<< "XYZ:" << triangle->index_xyz[j] << "   "
						<< "ST:" << triangle->index_st[j] << "\n"
				).str()
			);
#endif
		}
		triangle++;
	}

	// malloc and build array for Dup STs
	p_index_LUT_DUPS = (index_DUP_LUT_t *)pmm::man.pp_m_new( sizeof( index_DUP_LUT_t ) * dups );
	if ( p_index_LUT_DUPS == nullptr ) {
		pmm::man.pp_print(pmm::pl_normal, " Couldn't allocate memory!\n");
	}

	dup_index = 0;
	for ( i = 0; i < fm_head->numXYZ; i++ )
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
#ifdef FM_VERBOSE_DBG
	pmm::man.pp_print(pmm::pl_normal, (std::ostringstream{} << " Dups = " << dups << "\n").str());
	pmm::man.pp_print(pmm::pl_normal, (std::ostringstream{} << " Dup Index = " << dup_index << "\n").str());
#endif
	for ( i = 0; i < fm_head->numXYZ; i++ )
	{
#ifdef FM_VERBOSE_DBG
		pmm::man.pp_print(
			pmm::pl_normal,
			(
				std::ostringstream{}
					<< "Vert: "
					<< std::setw(4) << i << "\t"
					<< std::setw(4) << p_index_LUT[i].ST
			).str()
		);
#endif
		if ( p_index_LUT[i].next != nullptr ) {

			p_index_LUT2 = p_index_LUT[i].next;
			do {
#ifdef FM_VERBOSE_DBG
				pmm::man.pp_print(
					pmm::pl_normal,
					(
						std::ostringstream{}
							<< " "
							<< std::setw(4) << p_index_LUT2->Vert
							<< " "
							<< std::setw(4) << p_index_LUT2->ST
					).str()
				);
#endif
				p_index_LUT2 = p_index_LUT2->next;
			} while ( p_index_LUT2 != nullptr );

		}
#ifdef FM_VERBOSE_DBG
		pmm::man.pp_print(pmm::pl_normal, "\n");
#endif
	}


#ifdef FM_VERBOSE_DBG
	for ( i = 0; i < dup_index; i++ )
		pmm::man.pp_print(
			pmm::pl_normal,
			(
				std::ostringstream{}
					<< " Dup Index #" << i << "  "
					<< "OldVert: " << p_index_LUT_DUPS[i].OldVert << "  "
					<< "ST: " << p_index_LUT_DUPS[i].ST << "\n"
			).str()
		);

	triangle = tri_verts;
	for ( i = 0; i < fm_head->numTris; i++ )
	{
		for ( j = 0; j < 3; j++ )
			pmm::man.pp_print(
				pmm::pl_normal,
				(
					std::ostringstream{}
						<< "Tri #" << i << ", "
						<< "Vert " << j << ":\t "
						<< "XYZ:" << triangle->index_xyz[j] << "   "
						<< "ST:" << triangle->index_st[j] << "\n"
				).str()
			);
		pmm::man.pp_print(pmm::pl_normal, "\n");
		triangle++;
	}
#endif
	// Build Picomodel
	triangle = tri_verts;
	for ( j = 0; j < fm_head->numTris; j++, triangle++ )
	{
		pmm::pp_set_surface_index( picoSurface, j * 3, triangle->index_xyz[0] );
		pmm::pp_set_surface_index( picoSurface, j * 3 + 1, triangle->index_xyz[1] );
		pmm::pp_set_surface_index( picoSurface, j * 3 + 2, triangle->index_xyz[2] );
	}

	vert = (fm_vert_normal_t*) ( (pmm::ub8_t*) ( frame->verts ) );
	for ( i = 0; i < fm_head->numXYZ; i++, vert++ )
	{
		/* set vertex origin */
		xyz[ 0 ] = vert->v[0] * frame->header.scale[0] + frame->header.translate[0];
		xyz[ 1 ] = vert->v[1] * frame->header.scale[1] + frame->header.translate[1];
		xyz[ 2 ] = vert->v[2] * frame->header.scale[2] + frame->header.translate[2];
		pmm::pp_set_surface_xyz( picoSurface, i, xyz );

		/* set normal */
		normal[ 0 ] = fm_normals[vert->lightnormalindex][0];
		normal[ 1 ] = fm_normals[vert->lightnormalindex][1];
		normal[ 2 ] = fm_normals[vert->lightnormalindex][2];
		pmm::pp_set_surface_normal( picoSurface, i, normal );

		/* set st coords */
		st[ 0 ] =  ( ( texCoord[p_index_LUT[i].ST].s ) / ( (float)fm_head->skinWidth ) );
		st[ 1 ] =  ( texCoord[p_index_LUT[i].ST].t / ( (float)fm_head->skinHeight ) );
		pmm::pp_set_surface_st( picoSurface, 0, i, st );
	}

	if ( dups ) {
		for ( i = 0; i < dups; i++ )
		{
			j = p_index_LUT_DUPS[i].OldVert;
			/* set vertex origin */
			xyz[ 0 ] = frame->verts[j].v[0] * frame->header.scale[0] + frame->header.translate[0];
			xyz[ 1 ] = frame->verts[j].v[1] * frame->header.scale[1] + frame->header.translate[1];
			xyz[ 2 ] = frame->verts[j].v[2] * frame->header.scale[2] + frame->header.translate[2];
			pmm::pp_set_surface_xyz( picoSurface, i + fm_head->numXYZ, xyz );

			/* set normal */
			normal[ 0 ] = fm_normals[frame->verts[j].lightnormalindex][0];
			normal[ 1 ] = fm_normals[frame->verts[j].lightnormalindex][1];
			normal[ 2 ] = fm_normals[frame->verts[j].lightnormalindex][2];
			pmm::pp_set_surface_normal( picoSurface, i + fm_head->numXYZ, normal );

			/* set st coords */
			st[ 0 ] =  ( ( texCoord[p_index_LUT_DUPS[i].ST].s ) / ( (float)fm_head->skinWidth ) );
			st[ 1 ] =  ( texCoord[p_index_LUT_DUPS[i].ST].t / ( (float)fm_head->skinHeight ) );
			pmm::pp_set_surface_st( picoSurface, 0, i + fm_head->numXYZ, st );
		}
	}

	/* set color */
	pmm::pp_set_surface_color( picoSurface, 0, 0, color );

	// Free up malloc'ed LL entries
	for ( i = 0; i < fm_head->numXYZ; i++ )
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
extern const pmm::module_t picoModuleFM =
{
	"0.85",                     /* module version string */
	"Heretic 2 FM",             /* module display name */
	"Nurail",                   /* author's name */
	"2003 Nurail",              /* module copyright */
	{
		"fm", nullptr, nullptr, nullptr  /* default extensions to use */
	},
	_fm_canload,                /* validation routine */
	_fm_load,                   /* load routine */
	nullptr,                       /* save validation routine */
	nullptr                        /* save routine */
};

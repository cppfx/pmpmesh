/* -----------------------------------------------------------------------------

   PicoModel Library

   Copyright (c) 2003, Randy Reddig & seaw0lf
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

#include <pmpmesh/pm_internal.hpp>
#include <pmpmesh/pmpmesh.hpp>
#include <sstream>

class tga_t
{
public:
	unsigned char id_length, colormap_type, image_type;
	unsigned short colormap_index, colormap_length;
	unsigned char colormap_size;
	unsigned short x_origin, y_origin, width, height;
	unsigned char pixel_size, attributes;
};



/*
   _terrain_load_tga_buffer()
   loads a tga image into a newly allocated image buffer
   fixme: replace/clean this function
 */

void _terrain_load_tga_buffer( unsigned char *buffer, unsigned char **pic, int *width, int *height ) {
	int row, column;
	int columns, rows, numPixels;
	unsigned char   *pixbuf;
	unsigned char   *buf_p;
	tga_t targa_header;
	unsigned char   *targa_rgba;


	*pic = nullptr;

	if ( buffer == nullptr ) {
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;

	targa_header.colormap_index = _pico_little_short( *(short*)buf_p );
	buf_p += 2;
	targa_header.colormap_length = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.y_origin = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.width = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.height = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if ( targa_header.image_type != 2 && targa_header.image_type != 10 && targa_header.image_type != 3 ) {
		pmm::man.pp_print(pmm::pl_error, "Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n");
		pic = nullptr;
		return;
	}

	if ( targa_header.colormap_type != 0 ) {
		pmm::man.pp_print(pmm::pl_error, "Indexed color TGA images not supported\n");
		return;
	}

	if ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 && targa_header.image_type != 3 ) {
		pmm::man.pp_print(pmm::pl_error, "Only 32 or 24 bit TGA images supported (not indexed color)\n");
		pic = nullptr;
		return;
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	targa_rgba = reinterpret_cast<decltype(targa_rgba)>(pmm::man.pp_m_new( numPixels * 4 ));
	*pic = targa_rgba;

	if ( targa_header.id_length != 0 ) {
		buf_p += targa_header.id_length;  // skip TARGA image comment

	}
	if ( targa_header.image_type == 2 || targa_header.image_type == 3 ) {
		// Uncompressed RGB or gray scale image
		for ( row = rows - 1; row >= 0; row-- )
		{
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; column++ )
			{
				unsigned char red,green,blue,alphabyte;
				switch ( targa_header.pixel_size )
				{

				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					break;
				}
			}
		}
	}

	/* rle encoded pixels */
	else if ( targa_header.image_type == 10 ) {
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for ( row = rows - 1; row >= 0; row-- ) {
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; ) {
				packetHeader = *buf_p++;
				packetSize = 1 + ( packetHeader & 0x7f );
				if ( packetHeader & 0x80 ) {        // run-length packet
					switch ( targa_header.pixel_size ) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					default:
						//Error("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
						break;
					}

					for ( j = 0; j < packetSize; j++ ) {
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if ( column == columns ) { // run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							}
							else{
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
				else {                            // non run-length packet
					for ( j = 0; j < packetSize; j++ ) {
						switch ( targa_header.pixel_size ) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default:
							//Sysprintf("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
							break;
						}
						column++;
						if ( column == columns ) { // pixel packet run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							}
							else{
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
			}
breakOut:;
		}
	}

	/* fix vertically flipped image */
	if ( ( targa_header.attributes & ( 1 << 5 ) ) ) {
		int flip;
		for ( row = 0; row < .5f * rows; row++ )
		{
			for ( column = 0; column < columns; column++ )
			{
				flip = *( (int*)targa_rgba + row * columns + column );
				*( (int*)targa_rgba + row * columns + column ) = *( (int*)targa_rgba + ( ( rows - 1 ) - row ) * columns + column );
				*( (int*)targa_rgba + ( ( rows - 1 ) - row ) * columns + column ) = flip;
			}
		}
	}
}



/*
   _terrain_canload()
   validates a picoterrain file
 */

static int _terrain_canload( PM_PARAMS_CANLOAD ) {
	picoParser_t    *p;


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
	if ( _pico_stricmp( p->token, "picoterrain" ) ) {
		_pico_free_parser( p );
		return pmm::pmv_error_ident;
	}

	/* free the pico parser object */
	_pico_free_parser( p );

	/* file seems to be a valid picoterrain file */
	return pmm::pmv_ok;
}



/*
   _terrain_load()
   loads a picoterrain file
 */

static pmm::model_t *_terrain_load( PM_PARAMS_LOAD ) {
	int i, j, v, pw[ 5 ], r;
	picoParser_t    *p;

	char            *shader, *heightmapFile, *colormapFile;
	pmm::vec3_t scale, origin;

	unsigned char   *imageBuffer;
	int imageBufSize, w, h, cw, ch;
	unsigned char   *heightmap, *colormap, *heightPixel, *colorPixel;

	pmm::model_t     *picoModel;
	pmm::surface_t   *picoSurface;
	pmm::shader_t    *picoShader;
	pmm::vec3_t xyz, normal;
	pmm::vec2_t st;
	pmm::color_t color;


	/* create pico parser */
	p = _pico_new_parser( (const pmm::ub8_t*) buffer, bufSize );
	if ( p == nullptr ) {
		return nullptr;
	}

	/* get first token */
	if ( _pico_parse_first( p ) == nullptr ) {
		return nullptr;
	}

	/* check first token */
	if ( _pico_stricmp( p->token, "picoterrain" ) ) {
		pmm::man.pp_print(pmm::pl_error, "Invalid PicoTerrain model");
		_pico_free_parser( p );
		return nullptr;
	}

	/* setup */
	shader = heightmapFile = colormapFile = nullptr;
	_pico_set_vec( scale, 512, 512, 32 );

	/* parse ase model file */
	while ( 1 )
	{
		/* get first token on line */
		if ( !_pico_parse_first( p ) ) {
			break;
		}

		/* skip empty lines */
		if ( !p->token || !p->token[ 0 ] ) {
			continue;
		}

		/* shader */
		if ( !_pico_stricmp( p->token, "shader" ) ) {
			if ( _pico_parse( p, 0 ) && p->token[ 0 ] ) {
				if ( shader != nullptr ) {
					pmm::man.pp_m_delete( shader );
				}
				shader = _pico_clone_alloc( p->token );
			}
		}

		/* heightmap */
		else if ( !_pico_stricmp( p->token, "heightmap" ) ) {
			if ( _pico_parse( p, 0 ) && p->token[ 0 ] ) {
				if ( heightmapFile != nullptr ) {
					pmm::man.pp_m_delete( heightmapFile );
				}
				heightmapFile = _pico_clone_alloc( p->token );
			}
		}

		/* colormap */
		else if ( !_pico_stricmp( p->token, "colormap" ) ) {
			if ( _pico_parse( p, 0 ) && p->token[ 0 ] ) {
				if ( colormapFile != nullptr ) {
					pmm::man.pp_m_delete( colormapFile );
				}
				colormapFile = _pico_clone_alloc( p->token );
			}
		}

		/* scale */
		else if ( !_pico_stricmp( p->token, "scale" ) ) {
			_pico_parse_vec( p, scale );
		}

		/* skip unparsed rest of line and continue */
		_pico_parse_skip_rest( p );
	}

	/* ----------------------------------------------------------------- */

	/* load heightmap */
	heightmap = imageBuffer = nullptr;
	imageBufSize = pmm::man.pp_load_file(heightmapFile, &imageBuffer);
	_terrain_load_tga_buffer( imageBuffer, &heightmap, &w, &h );
	pmm::man.pp_m_delete( heightmapFile );
	pmm::man.pp_f_delete(imageBuffer);

	if ( heightmap == nullptr || w < 2 || h < 2 ) {
		pmm::man.pp_print(pmm::pl_error, "PicoTerrain model with invalid heightmap");
		if ( shader != nullptr ) {
			pmm::man.pp_m_delete( shader );
		}
		if ( colormapFile != nullptr ) {
			pmm::man.pp_m_delete( colormapFile );
		}
		_pico_free_parser( p );
		return nullptr;
	}

	/* set origin (bottom lowest corner of terrain mesh) */
	_pico_set_vec( origin, ( w / -2 ) * scale[ 0 ], ( h / -2 ) * scale[ 1 ], -128 * scale[ 2 ] );

	/* load colormap */
	colormap = imageBuffer = nullptr;
	imageBufSize = pmm::man.pp_load_file(colormapFile, &imageBuffer);
	_terrain_load_tga_buffer( imageBuffer, &colormap, &cw, &ch );
	pmm::man.pp_m_delete( colormapFile );
	pmm::man.pp_f_delete(imageBuffer);

	if ( cw != w || ch != h ) {
		pmm::man.pp_print(pmm::pl_warning, "PicoTerrain colormap/heightmap size mismatch");
		pmm::man.pp_m_delete( colormap );
		colormap = nullptr;
	}

	/* ----------------------------------------------------------------- */

	/* create new pico model */
	picoModel = pmm::pp_new_model();
	if ( picoModel == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model");
		return nullptr;
	}

	/* do model setup */
	pmm::pp_set_model_frame_num( picoModel, frameNum );
	pmm::pp_set_model_num_frames( picoModel, 1 ); /* sea */
	pmm::pp_set_model_name( picoModel, fileName );
	pmm::pp_set_model_file_name( picoModel, fileName );

	/* allocate new pico surface */
	picoSurface = pmm::pp_new_surface( picoModel );
	if ( picoSurface == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model surface");
		pmm::pp_free_model( picoModel ); /* sea */
		return nullptr;
	}

	/* terrain surfaces are triangle meshes */
	pmm::pp_set_surface_type( picoSurface, pmm::st_triangles );

	/* set surface name */
	pmm::pp_set_surface_name( picoSurface, "picoterrain" );

	/* create new pico shader */
	picoShader = pmm::pp_new_shader( picoModel );
	if ( picoShader == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model shader");
		pmm::pp_free_model( picoModel );
		pmm::man.pp_m_delete( shader );
		return nullptr;
	}

	/* detox and set shader name */
	_pico_setfext( shader, "" );
	_pico_unixify( shader );
	pmm::pp_set_shader_name( picoShader, shader );
	pmm::man.pp_m_delete( shader );

	/* associate current surface with newly created shader */
	pmm::pp_set_surface_shader( picoSurface, picoShader );

	/* make bogus normal */
	_pico_set_vec( normal, 0.0f, 0.0f, 0.0f );

	/* create mesh */
	for ( j = 0; j < h; j++ )
	{
		for ( i = 0; i < w; i++ )
		{
			/* get pointers */
			v = i + ( j * w );
			heightPixel = heightmap + v * 4;
			colorPixel = colormap
						 ? colormap + v * 4
						 : nullptr;

			/* set xyz */
			_pico_set_vec( xyz, origin[ 0 ] + scale[ 0 ] * i,
						   origin[ 1 ] + scale[ 1 ] * j,
						   origin[ 2 ] + scale[ 2 ] * heightPixel[ 0 ] );
			pmm::pp_set_surface_xyz( picoSurface, v, xyz );

			/* set normal */
			pmm::pp_set_surface_normal( picoSurface, v, normal );

			/* set st */
			st[ 0 ] = (float) i;
			st[ 1 ] = (float) j;
			pmm::pp_set_surface_st( picoSurface, 0, v, st );

			/* set color */
			if ( colorPixel != nullptr ) {
				_pico_set_color( color, colorPixel[ 0 ], colorPixel[ 1 ], colorPixel[ 2 ], colorPixel[ 3 ] );
			}
			else{
				_pico_set_color( color, 255, 255, 255, 255 );
			}
			pmm::pp_set_surface_color( picoSurface, 0, v, color );

			/* set triangles (zero alpha in heightmap suppresses this quad) */
			if ( i < ( w - 1 ) && j < ( h - 1 ) && heightPixel[ 3 ] >= 128 ) {
				/* set indices */
				pw[ 0 ] = i + ( j * w );
				pw[ 1 ] = i + ( ( j + 1 ) * w );
				pw[ 2 ] = i + 1 + ( ( j + 1 ) * w );
				pw[ 3 ] = i + 1 + ( j * w );
				pw[ 4 ] = i + ( j * w );  /* same as pw[ 0 ] */

				/* set radix */
				r = ( i + j ) & 1;

				/* make first triangle */
				pmm::pp_set_surface_index( picoSurface, ( v * 6 + 0 ), (pmm::index_t) pw[ r + 0 ] );
				pmm::pp_set_surface_index( picoSurface, ( v * 6 + 1 ), (pmm::index_t) pw[ r + 1 ] );
				pmm::pp_set_surface_index( picoSurface, ( v * 6 + 2 ), (pmm::index_t) pw[ r + 2 ] );

				/* make second triangle */
				pmm::pp_set_surface_index( picoSurface, ( v * 6 + 3 ), (pmm::index_t) pw[ r + 0 ] );
				pmm::pp_set_surface_index( picoSurface, ( v * 6 + 4 ), (pmm::index_t) pw[ r + 2 ] );
				pmm::pp_set_surface_index( picoSurface, ( v * 6 + 5 ), (pmm::index_t) pw[ r + 3 ] );
			}
		}
	}

	/* free stuff */
	_pico_free_parser( p );
	pmm::man.pp_m_delete( heightmap );
	pmm::man.pp_m_delete( colormap );

	/* return the new pico model */
	return picoModel;
}



/* pico file format module definition */
extern const pmm::module_t picoModuleTerrain =
{
	"1.3",                      /* module version string */
	"PicoTerrain",              /* module display name */
	"Randy Reddig",             /* author's name */
	"2003 Randy Reddig",        /* module copyright */
	{
		"picoterrain", nullptr, nullptr, nullptr /* default extensions to use */
	},
	_terrain_canload,           /* validation routine */
	_terrain_load,              /* load routine */
	nullptr,                       /* save validation routine */
	nullptr                        /* save routine */
};

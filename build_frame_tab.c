/*
  build_frame_tab.c -- build table of frame lengths
  Copyright (C) 2000 Dieter Baron

  This file is part of malint, an MPEG Audio stream validator.
  The author can be contacted at <dillo@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <string.h>

#include "mpeg.h"

short _mp3_flen_tab[4096];

int _mp3_samp_tab[4][3] = { /* sampling frequency in Hz */
    { 44100, 48000, 32000 },  /* MPEG 1.0 */
    { 22050, 24000, 16000 },  /* MPEG 2.0 */
    {     0,     0,     0 },  /* not used */
    { 11025, 12000,  8000 }   /* MPEG 2.5 */
};

int _mp3_bit_tab[2][16][3] = { /* bit rate in 1000 bit/s */
    { /* MPEG 2.0 / 2.5 */
	{   0,   0,   0 },
	{  32,   8,   8 },
	{  48,  16,  16 },
	{  56,  24,  24 },
	{  64,  32,  32 },
	{  80,  40,  40 },
	{  96,  48,  48 },
	{ 112,  56,  56 },
	{ 128,  64,  64 },
	{ 144,  80,  80 },
	{ 160,  96,  96 },
	{ 176, 112, 112 },
	{ 192, 128, 128 },
	{ 224, 144, 144 },
	{ 256, 160, 160 },
	{   0,   0,   0 },
    },
    { /* MPEG 1.0 */
	{   0,   0,   0 },
	{  32,  32,  32 },
	{  64,  48,  40 },
	{  96,  56,  48 },
	{ 128,  64,  56 },
	{ 160,  80,  64 },
	{ 192,  96,  80 },
	{ 224, 112,  96 },
	{ 256, 128, 112 },
	{ 288, 160, 128 },
	{ 320, 192, 160 },
	{ 352, 224, 192 },
	{ 384, 256, 224 },
	{ 416, 320, 256 },
	{ 448, 384, 320 },
	{   0,   0,   0 }
    }
};



void
build_length_table(void)
{
    int i, l;
    int version, layer, bitrate, samprate;

    memset(_mp3_flen_tab, 0, sizeof(_mp3_flen_tab));

    for (version=0; version<4; version++) {
	if (version == 2)
	    continue;
	for (layer=1; layer<4; layer++) {
	    for (bitrate=1; bitrate<15; bitrate++) {
		for (samprate=0; samprate<3; samprate++) {
		    if (layer == 1)
			l = ((version==0 ? 12000 : 6000)
			     * _mp3_bit_tab[version==0][bitrate][0]
			     / _mp3_samp_tab[version][samprate]) * 4;
		    else
			l = (version==0 ? 144000 : 72000)
			    * _mp3_bit_tab[version==0][bitrate][layer-1]
			    / _mp3_samp_tab[version][samprate];
		    i = (samprate<<1) | (bitrate<<3) | ((4-layer)<<8)
			| ((3-version)<<10);
		    _mp3_flen_tab[i] = _mp3_flen_tab[i|(1<<7)] = l;
		    _mp3_flen_tab[i|1] = _mp3_flen_tab[i|(1<<7)|1]
			= l + (layer==1 ? 4 : 1) ;
		}
	    }
	}
    }
}

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



int _mp3_samp_tab[2][3] = {
    {22050, 24000, 16000},
    {44100, 48000, 32000}
};

int _mp3_bit_tab[2][16][3] = {
    {
	{0, 0, 0},
	{ 32,  8,  8},
	{ 48, 16, 16},
	{ 56, 24, 24},
	{ 64, 32, 32},
	{ 80, 40, 40},
	{ 96, 48, 48},
	{112, 56, 56},
	{128, 64, 64},
	{144, 80, 80},
	{160, 96, 96},
	{176,112,112},
	{192,128,128},
	{224,144,144},
	{256,160,160},
	{0, 0, 0},
    },
    {
	{0, 0, 0},
	{32, 32, 32},
	{64, 48, 40},
	{96, 56, 48},
	{128, 64, 56},
	{160, 80, 64},
	{192, 96, 80},
	{224, 112, 96},
	{256, 128, 112},
	{288, 160, 128},
	{320, 192, 160},
	{352, 224, 192},
	{384, 256, 224},
	{416, 320, 256},
	{448, 384, 320},
	{0, 0, 0}
    }
};



void
build_length_table(int *table)
{
    int i, l;
    int version, layer, bitrate, samprate;

    memset(table, 0, sizeof(int)*2048);

    for (version=0; version<2; version++) {
	for (layer=1; layer<4; layer++) {
	    for (bitrate=1; bitrate<15; bitrate++) {
		for (samprate=0; samprate<3; samprate++) {
		    l = (version
			 ? (layer==1 ? 48000:144000)
			 : (layer==1 ? 24000: 72000))
			* _mp3_bit_tab[version][bitrate][layer-1]
			/ _mp3_samp_tab[version][samprate];
		    i = (samprate<<1) | (bitrate<<3) | ((4-layer)<<8)
			| (version<<10);
		    table[i] = table[i|(1<<7)] = (layer==1) ? l-3 : l;
		    table[i|1] = table[i|(1<<7)|1] = l+1;
		}
	    }
	}
    }
}

/*
  mpeg.c -- MPEG tables

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




#include "mpeg.h"

int _mp3_jsb_tab[3][4] = {
    { 4, 8, 12, 16 },
    { 4, 8, 12, 16},
    { 0, 4,  8, 16}
};

/* number of samples per frame */
int _mp3_nsamp_tab[3] = { 384, 1152, 1152 };

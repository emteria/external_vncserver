/*
droid vnc server - Android VNC server
Copyright (C) 2009 Jose Pereira <onaips@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef FLINGER_H
#define FLINGER_H

#include <stdint.h>

typedef struct _screenFormat
{
  uint16_t width;
  uint16_t height;

  uint8_t bitsPerPixel;

  uint16_t redMax;
  uint16_t greenMax;
  uint16_t blueMax;
  uint16_t alphaMax;

  uint8_t redShift;
  uint8_t greenShift;
  uint8_t blueShift;
  uint8_t alphaShift;

  uint32_t size;

  uint32_t pad;
} screenFormat;

int initFlinger(void);
unsigned int* readBuffer(void);
void closeFlinger(void);

#endif

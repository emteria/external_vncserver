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

#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <errno.h>
#include <cstring>
#include <error.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <cutils/properties.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h> /* for makedev() */

#include <linux/fb.h>
#include <linux/input.h>

#include <android/log.h>
#define L(...) do { __android_log_print(ANDROID_LOG_DEBUG, "vncd", __VA_ARGS__); printf(__VA_ARGS__); } while (0)

void setIdle(int i);

#endif

/* ---------------------------------------------------------------------- *
 * common.h
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * ---------------------------------------------------------------------- */
#ifndef __common_h__
#define __common_h__

#ifdef LC_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "lin-city.h"
#include "lcx11.h"

#elif defined WIN32
#include <windows.h>
#include "lcwin32.h"

#elif defined SVGALIB
#include <vga.h>
#include <vgagl.h>
#include <vgamouse.h>
#include "lcsvga.h"

#endif

#endif	/* __common_h__ */

/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id: js.h 2067 2006-01-30 07:36:01Z bram $
*/

#ifndef __INCLUDED_JS_H__
#define __INCLUDED_JS_H__ 1
#define JS_NEW

// #include "ul.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // -dw- for memcpy

#define _JS_MAX_AXES 16
#define _JS_MAX_BUTTONS 32
#define _JS_MAX_HATS 4

#define JS_TRUE  1
#define JS_FALSE 0


class jsJoystick
{
  int          id ;
protected:
  struct os_specific_s *os ;
  friend struct os_specific_s ;
  int          error        ;
  char         name [ 128 ] ;
  int          num_axes     ;
  int          num_buttons  ;

  float dead_band [ _JS_MAX_AXES ] ;
  float saturate  [ _JS_MAX_AXES ] ;
  float center    [ _JS_MAX_AXES ] ;
  float max       [ _JS_MAX_AXES ] ;
  float min       [ _JS_MAX_AXES ] ;

  void open () ;
  void close () ;

  float fudge_axis ( float value, int axis ) const ;

public:

  jsJoystick ( int ident = 0 ) ;
  ~jsJoystick () { close () ; }

  const char* getName () const { return name ;     }
  int   getNumAxes    () const { return num_axes ; }
  int   getNumButtons () const { return num_buttons; }
  int   notWorking    () const { return error ;    }
  void  setError      () { error = JS_TRUE ; }

  float getDeadBand ( int axis ) const       { return dead_band [ axis ] ; }
  void  setDeadBand ( int axis, float db )   { dead_band [ axis ] = db   ; }

  float getSaturation ( int axis ) const     { return saturate [ axis ]  ; }
  void  setSaturation ( int axis, float st ) { saturate [ axis ] = st    ; }

  void setMinRange ( float *axes ) { memcpy ( min   , axes, num_axes * sizeof(float) ) ; }
  void setMaxRange ( float *axes ) { memcpy ( max   , axes, num_axes * sizeof(float) ) ; }
  void setCenter   ( float *axes ) { memcpy ( center, axes, num_axes * sizeof(float) ) ; }

  void getMinRange ( float *axes ) const { memcpy ( axes, min   , num_axes * sizeof(float) ) ; }
  void getMaxRange ( float *axes ) const { memcpy ( axes, max   , num_axes * sizeof(float) ) ; }
  void getCenter   ( float *axes ) const { memcpy ( axes, center, num_axes * sizeof(float) ) ; }

  void read    ( int *buttons, float *axes ) ;
  void rawRead ( int *buttons, float *axes ) ;
  // bool SetForceFeedBack ( int axe, float force );
} ;

extern void jsInit () ;

#endif



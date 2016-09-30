//  glv - OpenGL viewer command line tool
/*****************************************************************************
 * Copyright (C) 2003 Patrick Lagace <bl4cklight@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
*****************************************************************************/

#ifndef LIMITS_GLV
#define LIMITS_GLV

// Auto-enable limits for visual c++
#ifdef _MSC_VER
//#define GLV_MISSING_LIMITS_HEADER_FILE
//#define GLV_MISSING_STD_MIN_MAX
#endif

#ifdef GLV_MISSING_STD_MIN_MAX
#undef min
#undef max

namespace std {
	template <class T>
		T min(T a,T b) {
			if(a < b)
				return a;
			return b;
		};
	template <class T>
		T max(T a,T b) {
			if(a > b)
				return a;
			return b;
		};
};

#endif //GLV_MISSING_STD_MIN_MAX

#ifdef GLV_MISSING_LIMITS_HEADER_FILE

#ifdef GLV_MISSING_CFLOAT_HEADER_FILE
#include <float.h>
#else // #ifdef GLV_MISSING_CFLOAT_HEADER_FILE
#include <cfloat>
#endif // #ifdef GLV_MISSING_CFLOAT_HEADER_FILE

namespace std {

  template <class T>
  struct numeric_limits {};

  template <>
  struct numeric_limits<float>
  {
    static double min() throw() {
      return FLT_MIN;
    }
    static double max() throw() {
      return FLT_MAX;
    }
  };
}

#else // #ifdef GLV_MISSING_LIMITS_HEADER_FILE

#include <limits>

#endif // #ifdef GLV_MISSING_LIMITS_HEADER_FILE

#endif // LIMITS_GLV


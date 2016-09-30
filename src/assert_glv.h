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

#ifndef ASSERT_GLV
#define ASSERT_GLV

#include <iostream>

#ifdef GLV_DEBUG

#include <signal.h>

// Extremely basic assertion mechanism
#define GLV_ASSERT(pCondition) \
{ \
  bool lCondition = pCondition; \
  if (!lCondition) { \
    std::cerr << "GLV_ASSERTION " << std::endl; \
    std::cerr << "    Condition   : " << #pCondition << std::endl; \
    std::cerr << "    Filename    : " << __FILE__ << std::endl; \
    std::cerr << "    Line number : " << __LINE__ << std::endl; \
    ::raise(SIGSEGV); \
  } \
}

#else // #ifdef GLV_DEBUG

#define GLV_ASSERT(pCondition)

#endif // #ifdef GLV_DEBUG

#endif // ASSERT_GLV


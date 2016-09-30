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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <map>

bool skipBlanks(const char** pStrPtr);
bool readIntegerTuple(const char** pStrPtr,int pSz,int* pData);
bool readFloatTuple(const char** pStrPtr,int pSz,float* pData);

int findFirstOf(const char* pSourceString,const char* pSet,int pStartPos=0);
int findFirstNotOf(const char* pSourceString,const char* pSet,int pStartPos=0);

bool extractParameterDictionaryFromLine(const std::string& pString,std::map<std::string,std::string>& pDictionary,int& pEndOfLine);

int         countWords(const std::string& pString);
std::string trimString(const std::string& pString,
                       const std::string& pToTrim);

#endif // STRING_UTILS_H


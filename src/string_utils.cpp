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

#include <cstring>
#include "string_utils.h"
#include "assert_glv.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

bool skipBlanks(const char** pStrPtr)
{
  const char* lStrPos = *pStrPtr;
  while(*lStrPos != '\0') {
    char c = *lStrPos;
    if(c != ' ' && c != '\t' && c != '\n') {
      *pStrPtr = lStrPos;
      return false;
    }
    lStrPos++;
  }
  *pStrPtr = lStrPos;
  return true;
}

// Pseudo-equivalent of the system function "strtod".
//  Recoded because of the dumb behavior of the strtod under win32
//   (under windows, strtod will compute the strlen of the string!
//    this is quite bad, for very long strings. (few megabytes... this
//    multiply the load time of a big raw mesh by 1000 ! ) ).
float readFloat(char* str,char** after)
{
  int i = 0;

  // Read while blank
  char c = str[i];
  while( c == ' ' || c == '\t' || c == '\n') {
    i++;
    c = str[i];
  }

  int lFirstNonBlank = i;
  // Read until a non-acceptable character
  while( (c >= '0' && c <= '9') || c == '-' || c == '+' || c == 'e' || c == '.') {
    i++;
    c = str[i];
  }

  char buffer[32];
  for(int j=0;j<i;j++) {
    buffer[j] = str[lFirstNonBlank+j];
  }
  buffer[i] = '\0';

  float val;
  if(sscanf(buffer,"%f",&val)) {
    *after = &str[i];
    return val;
  }
  *after = 0;
  return 0;
}

bool readFloatTuple(const char** pStrPtr,int pSz,float* pData)
{
  const char* lNewStrPos = *pStrPtr;

  for(int i=0;i<pSz;i++) {
    *pStrPtr = lNewStrPos;
    pData[i] = readFloat((char*)(*pStrPtr),(char**)&lNewStrPos);
  }
  if(*pStrPtr == lNewStrPos) {
    return false;
  }
  *pStrPtr = lNewStrPos;
  return true;
}

bool readIntegerTuple(const char** pStrPtr,int pSz,int* pData)
{
  const char* lNewStrPos = *pStrPtr;

  for(int i=0;i<pSz;i++) {
    *pStrPtr = lNewStrPos;
    pData[i] = strtol((char*)*pStrPtr,(char**)&lNewStrPos,10);
  }
  if(*pStrPtr == lNewStrPos) {
    return false;
  }
  *pStrPtr = lNewStrPos;
  return true;
}

int findFirstOf(const char* pSourceString,const char* pSet,int pStartPos)
{
  int lSzStr = strlen(pSourceString);
  int lSzSet = strlen(pSet);

  for(int i=pStartPos;i<lSzStr;i++) {
    for(int j=0;j<lSzSet;j++) {
      if(pSourceString[i] == pSet[j]) {
        return i;
      }
    }
  }
  return -1;
}

int findFirstNotOf(const char* pSourceString,const char* pSet,int pStartPos)
{
  int lSzStr = strlen(pSourceString);
  int lSzSet = strlen(pSet);

  for(int i=pStartPos;i<lSzStr;i++) {
    bool lNotIn = true;
    for(int j=0;j<lSzSet;j++) {
      if(pSourceString[i] == pSet[j]) {
        lNotIn = false;
      }
    }
    if(lNotIn) {
      return i;
    }
  }
  return -1;
}

bool extractParameterDictionaryFromLine(const std::string& pString,std::map<std::string,std::string>& pDictionary,int& pEndOfLine)
{
  int lParameterCount = 0;

  bool lWordFlag = false;
  bool lStringFlag = false;

  const int lSz = pString.size();
  int lStartWord = 0;
  for(int i=0;i<lSz;i++) {
    char c = pString[i];
    if(!lWordFlag) {
      if(c == '\n') {
        pEndOfLine = i;
        return lParameterCount != 0;
      }
      else if(c != ' ' && c != '\t') {
        lStartWord = i;
        lWordFlag = true;
      }
    }
    else {
      if(lStringFlag) {
        if(c == '\"') {
          lStringFlag = false;
        }
      }
      else {
        if(c == '\"') {
          lStringFlag = true;
        }
        else if(c == ' ' || c == '\t' || c == '\n') {
          std::string lWord = pString.substr(lStartWord,i-lStartWord);

          unsigned int lEqual = lWord.find("=");
          if(lEqual != std::string::npos) {
            pDictionary[lWord.substr(0,lEqual)] = lWord.substr(lEqual+1);
          }
          else {
            pDictionary[lWord] = "1";
          }
          lParameterCount++;
          lWordFlag = false;
          if(c == '\n') {
            std::cout << "END" << std::endl;
            pEndOfLine = i;
            return lParameterCount != 0;
          }
        }
      }
    }
  }
  pEndOfLine = lSz;
  return lParameterCount != 0;
}

int countWords(const std::string& pString)
{
  int  lCount           = 0;
  bool lStartWhiteSpace = false;

  std::string::size_type lLen = pString.size();

  for(std::string::size_type i=0; i<lLen; ++i) {

    if(pString[i] == ' ') {
      lStartWhiteSpace = false;
    }
    else {
      if(!lStartWhiteSpace) {
        ++lCount;
      }
      lStartWhiteSpace = true;
    }
  }
  return lCount;
}

std::string trimString(const std::string& pString,
                       const std::string& pToTrim)
{
  std::string lTrimmed;

  if (!pString.empty()) {
    std::string::size_type lBegin = pString.find_first_not_of(pToTrim);
    std::string::size_type lEnd   = pString.find_last_not_of (pToTrim);

    if (lBegin != std::string::npos && lEnd != std::string::npos) {
      lTrimmed = pString.substr(lBegin, (lEnd+1)-lBegin);
    }
    else {
      GLV_ASSERT(lBegin == std::string::npos && lEnd == std::string::npos);
    }
  }

  return lTrimmed;
}

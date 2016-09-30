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

#include "glinclude.h"
#include <string>
#include <vector>

void drawText(const std::string& pText,float pXMin,float pYMin,float pXMax,float pYMax,int pFontSize,void* pCurrentFont,bool pAutoLineBreak,bool pFlagCentered)
{
	// We get the viewport size, to be able afterwise to transform pixel
	// sizes in "global viewport" referential.
	int lViewportStats[4];
	glGetIntegerv(GL_VIEWPORT,lViewportStats);
	const float lLineSize = (float)pFontSize/lViewportStats[3];

	// If enabled, we execute end-of-line cutted word processing
	std::string lWordCutContent = pText;

	std::vector<float> lVectLineWidths;

		//Word cut
		std::vector<float> lCharSizes;
		{for(unsigned int i=0;i<pText.size();i++) {
			lCharSizes.push_back(glutBitmapWidth(pCurrentFont,lWordCutContent[i])/(float)lViewportStats[2]);
		}}
		float lMaxWidth = pXMax-pXMin;
		float lCurrentLineWidth=0;
		float lCurrentWordWidth=0;
		int lStartOfWord=0;
		{for(unsigned int i=0;i<pText.size();i++) {
			if(pText[i] == ' ') {
				lStartOfWord = i;
				lCurrentLineWidth += lCurrentWordWidth + lCharSizes[i];
				lCurrentWordWidth=0;			
			}
			else if(pText[i] == '\n') {
				lStartOfWord = i;
				lVectLineWidths.push_back(lCurrentLineWidth+lCurrentWordWidth);
				lCurrentLineWidth=0;
				lCurrentWordWidth=0;			
			}
			else {
				lCurrentWordWidth += lCharSizes[i];	
				if(pAutoLineBreak) {
					if(lCurrentLineWidth + lCurrentWordWidth > lMaxWidth) {				
						lWordCutContent[lStartOfWord] = '\n';
						lCurrentLineWidth = 0;
					}
				}
			}
		}}
		lVectLineWidths.push_back(lCurrentLineWidth+lCurrentWordWidth);
	

	glRasterPos2f(pXMin,pYMax-lLineSize);	
	
	glPushAttrib(GL_ENABLE_BIT);
	// Text drawing

	glDisable(GL_DEPTH_TEST);

	int lCurrentLine = 1;

	bool aCenteredMode = pFlagCentered;
	if(!aCenteredMode) {
		glRasterPos2f(pXMin,pYMax-lLineSize);
	}
	else {
		glRasterPos2f((pXMax+pXMin)/2-lVectLineWidths[lCurrentLine-1]/2,pYMax-lLineSize);
	}
	float lXPos = pXMin;
	
	for(unsigned int i=0;i < lWordCutContent.size();i++) {
		lXPos += glutBitmapWidth(pCurrentFont,lWordCutContent[i])/(float)lViewportStats[2];
		if(lXPos > pXMax || lWordCutContent[i] == '\n') {
			lCurrentLine++;
			if(!aCenteredMode) {
				glRasterPos2f(pXMin,pYMax-lLineSize*lCurrentLine);
			}
			else {
				glRasterPos2f((pXMax+pXMin)/2-lVectLineWidths[lCurrentLine-1]/2,pYMax-lLineSize*lCurrentLine);
			}
			lXPos = pXMin;
			if(pYMin+lLineSize*lCurrentLine > pYMax) 
				break;			
		}
		else {
			glutBitmapCharacter(pCurrentFont,lWordCutContent[i]);
		}
	}

	glPopAttrib();

}

void drawText3D(const std::string& pText,float pX,float pY,float pZ,void* pCurrentFont)
{
	glRasterPos3f(pX,pY,pZ);	
	
	glPushAttrib(GL_ENABLE_BIT);
	// Text drawing

	glDisable(GL_DEPTH_TEST);

	
	for(unsigned int i=0;i < pText.size();i++) {
	  glutBitmapCharacter(pCurrentFont,pText[i]);
	}

	glPopAttrib();

}



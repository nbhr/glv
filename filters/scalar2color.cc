#include <stdio.h>
#include <set>
#include <vector>
#include <string>
#include <map>
#include <math.h>

void writeColorValue(float pVal);
void readColorValue(const std::string& pStringValue,float* pTargetRGB);
void hsv2rgb (float h, float s, float v, float * r, float * g, float * b);

bool gFlagAutoMinMax=false;
bool gFlagHue=true;
float gMin=0;
float gMax=1;
float gColorMin[3] = {0,0,1};
float gColorMax[3] = {1,0,0};

int main(int argc,char** argv)
{
  // Command line processing
  std::set<std::string> lSetSwitchs;
  std::map<std::string,std::string> lIndexSwitchValues;
  std::vector<std::string> lVectFilenames;
  for(int i=1;i<argc;i++) {
    if(argv[i][0] == '-') {
      std::string lStringArg(argv[i]);
      int lEqual = lStringArg.find("=");
      if(lEqual == std::string::npos) {
        lSetSwitchs.insert(argv[i]);
      }
      else {
        lIndexSwitchValues[lStringArg.substr(0,lEqual)] = lStringArg.substr(lEqual+1);
      }
    }
    else {
      lVectFilenames.push_back(argv[i]);
    }
  }

  bool lInvertSearch=false;

  std::string lSearch;
  if(lSetSwitchs.find("-h") != lSetSwitchs.end()) {
    printf("scalar2color filters \"scalar_v\" primitives from a GL stream; and convert them to useable\n");
    printf("  \"color\" primitives.\n");
    printf("  Default behavior: scalars between 0 and 1  are converted to a red/blue hue range\n");
    printf("  Options:\n");
    printf("   -autominmax : In raw_scalar_v blocks, compute the min/max of the color range from the values\n");
    printf("   -min=VALUE : [Default = 0] Manually set the minimum range of the values color table\n");
    printf("   -max=VALUE : [Default = 1] Manually set the maximum range of the values color table\n");
    printf("   -colormin=COLOR :  Either a RGB tuple   r,g,b   or a color name.\n");
    printf("   -colormax=COLOR :  Either a RGB tuple   r,g,b   or a color name.\n");
    printf("   colormin/max disable the hue mode, and color table is built from interpolation between\n");
    printf("    the two values\n");
    return 0;
  }
  if(lSetSwitchs.find("-autominmax") != lSetSwitchs.end()) {
    gFlagAutoMinMax = true;
  }
  if(lIndexSwitchValues.find("-min") != lIndexSwitchValues.end()) {
    sscanf(lIndexSwitchValues["-min"].c_str(),"%f",&gMin);
  }
  if(lIndexSwitchValues.find("-max") != lIndexSwitchValues.end()) {
    sscanf(lIndexSwitchValues["-max"].c_str(),"%f",&gMax);
  }
  if(lIndexSwitchValues.find("-colormin") != lIndexSwitchValues.end()) {
    gFlagHue = false;
    std::string lColorStr = lIndexSwitchValues["-colormin"];
    readColorValue(lColorStr,gColorMin);
  }
  if(lIndexSwitchValues.find("-colormax") != lIndexSwitchValues.end()) {
    gFlagHue = false;
    std::string lColorStr = lIndexSwitchValues["-colormax"];
    readColorValue(lColorStr,gColorMax);
  }

  // Open file/stdin
  FILE* f=0;
  if(lVectFilenames.size() == 0) {
    f = stdin;
  }
  else {
    f = fopen(lVectFilenames[0].c_str(),"r");
    if(!f) {
      fprintf(stderr,"Cannot open file: %s",lVectFilenames[1].c_str());
      return 1;
    }
  }


  char lBuffer[1024];
  fgets(lBuffer,1024,f);
  while(!feof(f)) {
    bool lMatch = strstr(lBuffer,lSearch.c_str());
    if(strstr(lBuffer,"raw_scalar_v") == lBuffer) {
      std::vector<float> lVectRawValues;
      float lMin = 99e+99;
      float lMax = -99e+99;

      fgets(lBuffer,1024,f);
      while(!feof(f) && strstr(lBuffer,"raw_end") != lBuffer) {
        float lVal=0;
        sscanf(lBuffer,"%f",&lVal);
        if(lVal < lMin) {
          lMin = lVal;
        }
        else if(lVal > lMax) {
          lMax = lVal;
        }
        lVectRawValues.push_back(lVal);
        fgets(lBuffer,1024,f);
      }

      if(gFlagAutoMinMax) {
        gMin = lMin;
        gMax = lMax;
      }
      printf("raw_color_v\n",lBuffer);
      for(int i=0;i<lVectRawValues.size();i++) { 
        writeColorValue(lVectRawValues[i]); 
      }
      if(strstr(lBuffer,"raw_end")) {
        printf("%s",lBuffer);
      }
    }
    else if(strstr(lBuffer,"scalar_v") == lBuffer) {
      float lVal=0;
      sscanf(lBuffer+6,"%f",&lVal);
      writeColorValue(lVal);
    }
    else {
      printf("%s",lBuffer);
    }
    fgets(lBuffer,1024,f);
  }
}

void writeColorValue(float pVal)
{
  float lFactor = (pVal-gMin)/(gMax-gMin);
  if(!gFlagHue) {
    float lFactorR = gColorMax[0]-gColorMin[0];
    float lFactorG = gColorMax[1]-gColorMin[1];
    float lFactorB = gColorMax[2]-gColorMin[2];
    float r = lFactor*lFactorR + gColorMin[0];
    float g = lFactor*lFactorG + gColorMin[1];
    float b = lFactor*lFactorB + gColorMin[2];
    printf("%f %f %f\n",r,g,b);
  }
  else {
    float r,g,b;
    hsv2rgb(lFactor*(2/3.),1,1,&r,&g,&b);  
    printf("%f %f %f\n",r,g,b);
  }
}

void readColorValue(const std::string& pStringValue,float* pTargetRGB)
{
  if(!pStringValue.empty()) {
    if(pStringValue[0] >= '0' && pStringValue[0] <= '9') {
      // RGB value
      sscanf(pStringValue.c_str(),"%f,%f,%f",&pTargetRGB[0],&pTargetRGB[1],&pTargetRGB[2]);
    }
    else {
      // String value
      if(pStringValue == "blue") {
        pTargetRGB[0] = 0;
        pTargetRGB[1] = 0;
        pTargetRGB[2] = 1;
      }
      else if(pStringValue == "red") {
        pTargetRGB[0] = 1;
        pTargetRGB[1] = 0;
        pTargetRGB[2] = 0;
      }
      else if(pStringValue == "green") {
        pTargetRGB[0] = 0;
        pTargetRGB[1] = 1;
        pTargetRGB[2] = 0;
      }
      else if(pStringValue == "white") {
        pTargetRGB[0] = 0;
        pTargetRGB[1] = 0;
        pTargetRGB[2] = 0;
      }
      else if(pStringValue == "black") {
        pTargetRGB[0] = 1;
        pTargetRGB[1] = 1;
        pTargetRGB[2] = 1;
      }
    }
  }
}

void hsv2rgb (float h, float s, float v, float * r, float * g, float * b)
{
  int i;
  float aa, bb, cc, f;

  if(s == 0)
    *r = *g = *b = v;
  else {
    if (h == 1.0)
      h = 0;
    h *= 6.0;
    i = (int)floor (h);
    f = h - i;
    aa = v * (1 - s);
    bb = v * (1 - (s * f));
    cc = v * (1 - (s * (1 - f)));
    switch (i) {
            case 0: *r = v;  *g = cc; *b = aa; break;
            case 1: *r = bb; *g = v;  *b = aa; break;
            case 2: *r = aa; *g = v;  *b = cc; break;
            case 3: *r = aa; *g = bb; *b = v;  break;
            case 4: *r = cc; *g = aa; *b = v;  break;
            case 5: *r = v;  *g = aa; *b = bb; break;
    }
  }
}

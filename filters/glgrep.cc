#include <stdio.h>
#include <set>
#include <vector>
#include <string>

int main(int argc,char** argv)
{
  // Command line processing
  std::set<std::string> lSetSwitchs;
  std::vector<std::string> lVectFilenames;
  for(int i=1;i<argc;i++) {
    if(argv[i][0] == '-') {
      lSetSwitchs.insert(argv[i]);
    }
    else {
      lVectFilenames.push_back(argv[i]);
    }
  }

  bool lInvertSearch=false;
  if(lSetSwitchs.find("-v") != lSetSwitchs.end()) {
    lInvertSearch = true;
  }
  else if(lSetSwitchs.find("-h") != lSetSwitchs.end() || lSetSwitchs.find("-help") != lSetSwitchs.end()) {
    printf("Usage:  glgrep [-v] PATTERN [FILENAME]\n");
    printf("  -v = Invert search ");
    printf("  PATTERN = Pattern to search for; output only lines/raw_sections with pattern\n");
    printf("  FILENAME = File to parse, if not given; using stdin\n"); 
  }
  std::string lSearch;
  if(lVectFilenames.empty()) {
    fprintf(stderr,"Error - Must specify a pattern to search for\n");
    return 1;
  }
  lSearch = lVectFilenames[0];

  // Open file/stdin
  FILE* f=0;
  if(lVectFilenames.size() == 1) {
    f = stdin;
  }
  else {
    f = fopen(lVectFilenames[1].c_str(),"r");
    if(!f) {
      fprintf(stderr,"Cannot open file: %s",lVectFilenames[1].c_str());
      return 1;
    }
  }

  char lBuffer[1024];
  fgets(lBuffer,1024,f);
  while(!feof(f)) {
    bool lMatch = strstr(lBuffer,lSearch.c_str());
    if(strstr(lBuffer,"raw_") == lBuffer) {
      if((lMatch && !lInvertSearch) || (!lMatch && lInvertSearch)) {
        printf("%s",lBuffer);
        fgets(lBuffer,1024,f);
        while(!feof(f) && strstr(lBuffer,"raw_end") != lBuffer) {
          printf("%s",lBuffer);
          fgets(lBuffer,1024,f);
        } 
        if(strstr(lBuffer,"raw_end")) {
          printf("%s",lBuffer);
        }
      }
      else {
        fgets(lBuffer,1024,f);
        while(!feof(f) && strstr(lBuffer,"raw_end") != lBuffer) {
          fgets(lBuffer,1024,f);
        } 
      }
    }
    else {
      lMatch = strstr(lBuffer,lSearch.c_str());
      if((lMatch && !lInvertSearch) || (!lMatch && lInvertSearch)) {
        printf("%s",lBuffer);
      }
    }
    fgets(lBuffer,1024,f);
  }
}

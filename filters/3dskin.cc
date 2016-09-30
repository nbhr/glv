#include <stdio.h>
#include <string>
#include <vector>
#include <map>

class Cell3D {
public:
  int aSz;
  int aConn[8];
  Cell3D(const int pConnSz,const int* pConn) {
    aSz = pConnSz;
    for(int i=0;i<pConnSz;i++) {
      aConn[i] = pConn[i];
    }
  }
};

class Facet {
public:
  int c[4];
  int sz;
  Facet()
  {
    c[0] = 0;
    c[1] = 0;
    c[2] = 0;
    c[3] = 0;
    sz = 3;
  }
  Facet(int n1,int n2,int n3)
  {
    if(n1 < n2) {
      if(n2 < n3) {
        c[0] = n1;
        c[1] = n2;
        c[2] = n3;
      }
      else {
        if(n1 < n3) {
          c[0] = n1;
          c[1] = n3;
          c[2] = n2;
        }

        else {
          c[0] = n3;
          c[1] = n1;
          c[2] = n2;
        }
      }
    }
    else {
      if(n1 < n3) {
        c[0] = n2;
        c[1] = n1;
        c[2] = n3;
      }
      else {
        if(n2 < n3) {
          c[0] = n2;
          c[1] = n3;
          c[2] = n1;
        }
        else {
          c[0] = n3;
          c[1] = n2;
          c[2] = n1;
        }
      }
    }
    c[3] = 0;

    sz = 3;
  }

  Facet(int n1,int n2,int n3,int n4)
  {
    if(n1 < n2) {
      if(n2 < n3) {
        c[0] = n1;
        c[1] = n2;
        c[2] = n3;
      }
      else {
        if(n1 < n3) {
          c[0] = n1;
          c[1] = n3;
          c[2] = n2;
        }
        else {
          c[0] = n3;
          c[1] = n1;
          c[2] = n2;
        }
      }
    }
    else {
      if(n1 < n3) {
        c[0] = n2;
        c[1] = n1;
        c[2] = n3;
      }

      else {
        if(n2 < n3) {
          c[0] = n2;
          c[1] = n3;
          c[2] = n1;
        }
        else {
          c[0] = n3;
          c[1] = n2;
          c[2] = n1;
        }
      }
    }
    int i=0;
    for(i=0;i<3;i++) {
      if(c[i] > n4) {
        for(int j=2;j>=i;j--) {
          c[j+1] = c[j];
        }
        c[i] = n4;
        break;
      }
    }
    if(i==3) {
      c[3] = n4;
    }

    sz = 4;
  }

  Facet(const Facet& ref)
  {
    sz = ref.sz;
    c[0] = ref.c[0];
    c[1] = ref.c[1];
    c[2] = ref.c[2];
    c[3] = ref.c[3];
  }

  Facet& operator=(const Facet& ref)
  {
    sz = ref.sz;
    c[0] = ref.c[0];
    c[1] = ref.c[1];
    c[2] = ref.c[2];
    c[3] = ref.c[3];
    return *this;
  }

  bool operator==(const Facet& ref) const
  {
    if(sz == ref.sz) {
      if(sz == 3) {
        return (c[0] == ref.c[0]) &&
                (c[1] == ref.c[1]) &&
                (c[2] == ref.c[2]);
      }
      return (c[0] == ref.c[0]) &&
              (c[1] == ref.c[1]) &&
              (c[2] == ref.c[2]) &&
              (c[3] == ref.c[3]);
    }
    return false;
  }

  bool operator<(const Facet& ref) const
  {
    if(sz < ref.sz) {
      return true;
    }
    else if(sz > ref.sz) {
      return false;
    }
    if(c[0] < ref.c[0]) {
      return true;
    }

    else if(c[0] == ref.c[0]) {
      if(c[1] < ref.c[1]) {
        return true;
      }
      else if(c[1] == ref.c[1]) {
        if(c[2] < ref.c[2]) {
          return true;
        }
        else if(c[2] == ref.c[2]) {
          if(sz == 3) {
            return false;
          }
          if(c[3] < ref.c[3]) {
            return true;
          }
        }
      }
    }
    return false;
  }

};


int gLineNo=0;
std::vector<Facet> gVectFacets;
std::map<Facet,int> gCountFacets;

void readRawCell(FILE* f,int pConnSize);
void flushCellsToFacets();

int main(int argc,char** argv)
{
  if(argc == 2 && strstr(argv[1],"-h") == argv[1]) {
    printf("Usage:  Piped data or 1 argument filename.\n  Transform any tetra_v prism_v hexa_v or pyra_v primitives to triangle_v/quad_v\n");
    return 0;
  }
  char lBuffer[1024];
  
  // Open file/stdin
  FILE* f=0;
  if(argc == 1) {
    f = stdin; 
  }
  else {
    f = fopen(argv[1],"r");
    if(!f) {
      fprintf(stderr,"Cannot open file: %s",argv[1]);
      return 1;
    }
  }

  fgets(lBuffer,1024,f);
  gLineNo++;
  while(!feof(f)) {
    if(strstr(lBuffer,"raw_tetra_v") == lBuffer) {
      readRawCell(f,4);
    }
    else if(strstr(lBuffer,"raw_pyra_v") == lBuffer) {
      readRawCell(f,5);
    }
    else if(strstr(lBuffer,"raw_prism_v") == lBuffer) {
      readRawCell(f,6);
    }
    else if(strstr(lBuffer,"raw_hexa_v") == lBuffer) {
      readRawCell(f,8);
    }
    else if(strstr(lBuffer,"tetra_v") == lBuffer) {
      int conn[8];
      int nb = sscanf(lBuffer+strlen("tetra_v"),"%d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3]);
      if(nb != 4) {
        fprintf(stderr,"Error reading tetra connectivity\n");
        return 1;
      }
      gVectFacets.push_back(Facet(conn[0],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[0],conn[3]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(strstr(lBuffer,"pyra_v") == lBuffer) {
      int conn[8];
      int nb = sscanf(lBuffer+strlen("pyra_v"),"%d %d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3],&conn[4]);
      if(nb != 5) {
        fprintf(stderr,"Error reading pyra connectivity\n");
        return 1;
      }
      gVectFacets.push_back(Facet(conn[0],conn[3],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[3],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[3],conn[0],conn[4]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(strstr(lBuffer,"prism_v") == lBuffer) {
      int conn[8];
      int nb = sscanf(lBuffer+strlen("prism_v"),"%d %d %d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3],&conn[4],&conn[5]);
      if(nb != 6) {
        fprintf(stderr,"Error reading prism connectivity\n");
        return 1;
      }
      gVectFacets.push_back(Facet(conn[0],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[4],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[5],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[0],conn[3],conn[5]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[3],conn[4],conn[5]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(strstr(lBuffer,"hexa_v") == lBuffer) {
      int conn[8];
      int nb = sscanf(lBuffer+strlen("hexa_v"),"%d %d %d %d %d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3],&conn[4],&conn[5],&conn[6],&conn[7]);
      if(nb != 6) {
        fprintf(stderr,"Error reading hexa connectivity\n");
        return 1;
      }
      gVectFacets.push_back(Facet(conn[0],conn[3],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[5],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[6],conn[5]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[3],conn[7],conn[6]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[3],conn[0],conn[4],conn[7]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[4],conn[5],conn[6],conn[7]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(strstr(lBuffer,"object_end")) {
      flushCellsToFacets(); 
      printf("%s",lBuffer);
    }
    else {
      printf("%s",lBuffer);
    }
    fgets(lBuffer,1024,f);
    gLineNo++;
  }

  fclose(f);

  flushCellsToFacets();

  return 0;  
}

void flushCellsToFacets()
{
  const int lNbFacet = gVectFacets.size();
  for(int i=0;i<lNbFacet;i++) {
    Facet& face = gVectFacets[i];
    if(gCountFacets[face] == 1) {
      if(face.sz == 3) {
        printf("triangle_v %d %d %d\n",face.c[0],face.c[1],face.c[2]);
      }
      else {
        printf("quad_v %d %d %d %d\n",face.c[0],face.c[1],face.c[2],face.c[3]);
      }
    }
  }
  gVectFacets.clear();
  gCountFacets.clear();
}

void readRawCell(FILE* f,int pConnSize)
{
  char lBuffer[1024];
  int conn[8];

  fgets(lBuffer,1024,f);
  gLineNo++;
  while(strstr(lBuffer,"raw_end") != lBuffer && !feof(f)) {
    int nb = 0;
    if(pConnSize == 4) {
      nb = sscanf(lBuffer,"%d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3]);
      gVectFacets.push_back(Facet(conn[0],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[0],conn[3]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(pConnSize == 5) {
      nb = sscanf(lBuffer,"%d %d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3],&conn[4]);
      gVectFacets.push_back(Facet(conn[0],conn[3],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[3],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[3],conn[0],conn[4]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(pConnSize == 6) {
      nb = sscanf(lBuffer,"%d %d %d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3],&conn[4],&conn[5]);
      gVectFacets.push_back(Facet(conn[0],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[4],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[5],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[0],conn[3],conn[5]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[3],conn[4],conn[5]));
      gCountFacets[gVectFacets.back()]++;
    }
    else if(pConnSize == 8) {
      nb = sscanf(lBuffer,"%d %d %d %d %d %d %d %d",&conn[0],&conn[1],&conn[2],&conn[3],&conn[4],&conn[5],&conn[6],&conn[7]);
      gVectFacets.push_back(Facet(conn[0],conn[2],conn[1]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[0],conn[1],conn[4],conn[3]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[1],conn[2],conn[5],conn[4]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[2],conn[0],conn[3],conn[5]));
      gCountFacets[gVectFacets.back()]++;
      gVectFacets.push_back(Facet(conn[3],conn[4],conn[5]));
      gCountFacets[gVectFacets.back()]++;
    }
    if(nb != pConnSize) {
      fprintf(stderr,"Error reading %d connectivity\n",pConnSize);
      exit(1);
    }
    fgets(lBuffer,1024,f);
    gLineNo++;
  }
  if(feof(f)) {
    fprintf(stderr,"Error in raw_vertex - no raw_end\n");
    exit(1);
  }
}

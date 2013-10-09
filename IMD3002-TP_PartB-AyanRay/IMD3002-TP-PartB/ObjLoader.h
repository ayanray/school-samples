//By Karl Berg
//Last update Oct 18, 2003
//ModelType.h
//Loads, stores and displays .obj files on command

#ifndef MODELTYPE_H
#define MODELTYPE_H

#define objMaxNumVerts 75000
#define objMaxNumMats  64

#include <stdlib.h>
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


typedef struct
{
   GLuint	texID;
   GLuint	bpp;
   GLuint	width;
   GLuint	height;
} TextureImage;

typedef struct
{
	GLfloat x, y, z;
} OBJVertex;

typedef struct
{
	GLfloat u, v, w;
} OBJTexCoord;

struct OBJEdge
{
   int			lEdge;
   int			gEdge;

   int			lEdgeText;
   int			gEdgeText;

   OBJVertex	lEdgeNorm;
   OBJVertex	gEdgeNorm;

   OBJEdge	  *smlNext;
   OBJEdge	  *gtrNext;
};

struct OBJFace
{
   int v1, v2, v3, v4;
   int t1, t2, t3, t4;

   int n1, n2, n3, n4;
   OBJVertex faceNorm;

   int NormType;

   OBJFace  *next;
};

typedef struct
{
   char      Name[32];

   OBJVertex aCol;
   OBJVertex dCol;
   OBJVertex sCol;

   float     Reflect;
   float     Opacity;
   
   char      aTextName[255];
   char      dTextName[255];
   char      sTextName[255];

   int       NormType;
   int       SmthType;
} OBJMtrl;

typedef struct
{
   OBJFace  *objTngls;
   OBJFace  *objQuads;
   OBJEdge	*objEdges;

   int       matID;
} OBJGroup;

typedef struct
{
   OBJVertex    objVerts[objMaxNumVerts];
   OBJTexCoord  objTexts[objMaxNumVerts];
   OBJVertex    objNorms[objMaxNumVerts];
   OBJGroup     group[objMaxNumMats];
   OBJMtrl      mtrl[objMaxNumMats];

   int NumVerts;
   int NumTexts;
   int NumNorms;
   int NumTngls;
   int NumQuads;
} OBJModel;

typedef struct
{
   GLuint       DispList;
   GLuint       EdgeList;
   TextureImage TextMap;
   TextureImage GlowMap;
   TextureImage SpecMap;

   bool         hasEdges;
   bool         hasTexts;
   bool         hasGlows;
   bool         hasSpecs;

   GLfloat      dMat[4];
   GLfloat      aMat[4];
   GLfloat      sMat[4];
   GLfloat      SpecQty;
} OBJCModel;

extern bool useSmooth;
extern bool useLights;

// Thanks to LongJumper of the iDevGames forums for the following function
char* better_fgets(char *line, int len, FILE *in_file);

class ModelType
{
   public:

       ModelType();
      ~ModelType();

      void Draw();

      bool LoadObj(char *filename, float Scale);

      void DeleteModel();

      OBJVertex maxVerts;
      OBJVertex minVerts;

      float     myRadius;
      float     myScale;

   private:

      void     Normalise_Vector(OBJVertex &vec);
      void     Calc_Norm_Vector(const OBJVertex& p1, const OBJVertex& p2, const OBJVertex& p3, OBJVertex &normal);

      void     Delete_Edge(OBJEdge *currPtr);
      bool     Insert_Edge(OBJEdge *currPtr, OBJEdge *newEdge);
      OBJEdge* Add_New_Edge(OBJEdge *headPtr, int v1, int t1, const OBJVertex& n1, int v2, int t2, const OBJVertex& n2);
      void     Build_Edge_List();
      void     Delete_Face(OBJFace *headPtr);
      void     Insert_Face(OBJFace *headPtr, OBJFace *newFace);

// TGA Loader code heavily based off nehe tutorial code (nehe.gamedev.net)
      bool     LoadTGA_CMP(FILE *in_file, TextureImage &texture, GLubyte *imageData);
      bool     LoadTGA_RAW(FILE *in_file, TextureImage &texture, GLubyte *imageData);
      bool     LoadTGA(TextureImage &texture, char *filename, int UseMipMaps);
      void     DeleteTexture(TextureImage &texture);

      bool     ParseMtl(char *filename);
      bool     ParseObj(char *filename);

      void     DrawEdges(OBJEdge *theEdge);
      void     GenEdgeObj();

      void     NormDispObj();
      void     GenDispObj();

      OBJModel  *tDispObj;
      OBJCModel *cDispObj;

      int		  numObjects;
      bool       FastSmooth;
};

#endif



//By Karl Berg
//Last update Oct 17, 2003
//ModelType.cpp
//Loads, stores and displays .obj files on command

#include "ObjLoader.h"

bool useSmooth = 1;
bool useLights = 1;

ModelType::ModelType()
{
	maxVerts.x = 0;
	maxVerts.y = 0;
	maxVerts.z = 0;

	minVerts.x = 0;
	minVerts.y = 0;
	minVerts.z = 0;

   myRadius = 0.0f;
   myScale  = 1.0f;

   numObjects = 0;
   tDispObj = NULL;
   cDispObj = NULL;

   FastSmooth = true;
}

ModelType::~ModelType()
{
   DeleteModel();
}

void ModelType::Draw()
{
   GLfloat  nullMat[] = { 0.00f, 0.00f, 0.00f, 1.00f };

   glPushAttrib(GL_LINE_SMOOTH | GL_POLYGON_OFFSET_FILL | GL_TEXTURE_2D | GL_BLEND);
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glEnable(GL_POLYGON_OFFSET_LINE);
   glEnable(GL_BLEND);
   glEnable(GL_TEXTURE_2D);

   for (int i = 0; i < numObjects; i++)
   {
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cDispObj[i].SpecQty);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, nullMat);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cDispObj[i].dMat);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cDispObj[i].aMat);
      glPolygonOffset(1.0, 2.0);
      glDepthMask(GL_TRUE);

      if (cDispObj[i].hasTexts)
      {
         glBindTexture(GL_TEXTURE_2D, cDispObj[i].TextMap.texID);
         glCallList(cDispObj[i].DispList);
         glDepthMask(GL_FALSE);
         if (cDispObj[i].hasEdges && useSmooth)
         {
            glPolygonOffset(1.0, 3.0);
            if (FastSmooth)
               glCallList(cDispObj[i].EdgeList);
            else
            {
               glPolygonMode(GL_FRONT, GL_LINE);
               glCallList(cDispObj[i].DispList);
               glPolygonMode(GL_FRONT, GL_FILL);
            }
         }
      }
      else
      {
         glDisable(GL_TEXTURE_2D);
         glCallList(cDispObj[i].DispList);
         glDepthMask(GL_FALSE);
         if (cDispObj[i].hasEdges && useSmooth)
         {
            glPolygonOffset(1.0, 3.0);
            if (FastSmooth)
                glCallList(cDispObj[i].EdgeList);
            else
            {
               glPolygonMode(GL_FRONT, GL_LINE);
               glCallList(cDispObj[i].DispList);
               glPolygonMode(GL_FRONT, GL_FILL);
            }
         }
         glEnable(GL_TEXTURE_2D);
      }

      if (useLights)
      {
         glBlendFunc(GL_SRC_ALPHA, GL_ONE);
         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cDispObj[i].sMat);
         glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, nullMat);
         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, nullMat);
         glPolygonOffset(1.0, 1.0);
         if (cDispObj[i].hasSpecs)
         {
            glBindTexture(GL_TEXTURE_2D, cDispObj[i].SpecMap.texID);
            glCallList(cDispObj[i].DispList);
         }
         if (cDispObj[i].hasGlows)
         {
            glDisable(GL_LIGHTING);
            glPolygonOffset(0.0, 0.0);
            glBindTexture(GL_TEXTURE_2D, cDispObj[i].GlowMap.texID);
            glCallList(cDispObj[i].DispList);
            glEnable(GL_LIGHTING);
         }
      }
   }
   glPopAttrib();
}

bool ModelType::LoadObj(char *filename, float Scale)
{
   myScale = Scale;

   const GLubyte *strRenderer;
   strRenderer = glGetString (GL_RENDERER);
   if ((strncmp((char*)strRenderer, "ATi Rage 128", 12) == 0) || (strncmp((char*)strRenderer, "ATI Rage 128", 12) == 0))
      FastSmooth = false;

   if (!ParseObj(filename))
      return false;

   NormDispObj();

   if (FastSmooth)
      Build_Edge_List();

   cDispObj = (OBJCModel*)malloc(sizeof(OBJCModel) * numObjects);

   GenDispObj();
   printf("Completed loading model %s.\n\n", filename);

   if (FastSmooth)
   {
      GenEdgeObj();
      for (int i = 0; i < numObjects; i++)
      {
         if (tDispObj->group[i].objEdges != NULL)
         {
            Delete_Edge(tDispObj->group[i].objEdges);
            tDispObj->group[i].objEdges = NULL;
         }
      }
   }

   for (int i = 0; i < numObjects; i++)
   {
      Delete_Face(tDispObj->group[i].objTngls);
      Delete_Face(tDispObj->group[i].objQuads);
      tDispObj->group[i].objTngls = NULL;
      tDispObj->group[i].objQuads = NULL;
   }

   free(tDispObj);
   tDispObj = NULL;

   return true;
}

void ModelType::DeleteModel()
{
   for (int i = 0; i < numObjects; i++)
   {
      glDeleteLists(cDispObj[i].DispList, 1);
      glDeleteLists(cDispObj[i].EdgeList, 1);

      DeleteTexture(cDispObj[i].TextMap);
      DeleteTexture(cDispObj[i].GlowMap);
      DeleteTexture(cDispObj[i].SpecMap);
   }
}

void ModelType::Normalise_Vector(OBJVertex &vec)
{
   float length;

   length = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));

   if (length == 0.0f)
		length = 1.0f;

   vec.x /= length;
   vec.y /= length;
   vec.z /= length;
}

void ModelType::Calc_Norm_Vector(const OBJVertex& p1, const OBJVertex& p2, const OBJVertex& p3, OBJVertex &normal)
{
   OBJVertex v1, v2;

   v1.x = p1.x - p2.x;
   v1.y = p1.y - p2.y;
   v1.z = p1.z - p2.z;

   v2.x = p2.x - p3.x;
   v2.y = p2.y - p3.y;
   v2.z = p2.z - p3.z;

   normal.x = v1.y * v2.z - v1.z * v2.y;
   normal.y = v1.z * v2.x - v1.x * v2.z;
   normal.z = v1.x * v2.y - v1.y * v2.x;

   Normalise_Vector(normal);
}

void ModelType::Delete_Edge(OBJEdge *currPtr)
{
   if (currPtr->smlNext != NULL)
      Delete_Edge(currPtr->smlNext);
   if (currPtr->gtrNext != NULL)
      Delete_Edge(currPtr->gtrNext);
   currPtr->smlNext = NULL;
   currPtr->gtrNext = NULL;
}

bool ModelType::Insert_Edge(OBJEdge *currPtr, OBJEdge *newEdge)
{
   if (currPtr->gEdge < newEdge->gEdge)
   {
      if (currPtr->smlNext == NULL)
      {
         currPtr->smlNext = newEdge;
         return true;
      }
      return Insert_Edge(currPtr->smlNext, newEdge);
   }

   if (currPtr->gEdge > newEdge->gEdge)
   {
      if (currPtr->gtrNext == NULL)
      {
         currPtr->gtrNext = newEdge;
         return true;
      }
      return Insert_Edge(currPtr->gtrNext, newEdge);
   }

   if  (currPtr->lEdge < newEdge->lEdge)
   {
      if (currPtr->smlNext == NULL)
      {
         currPtr->smlNext = newEdge;
         return true;
      }
      return Insert_Edge(currPtr->smlNext, newEdge);
   }

   if (currPtr->lEdge > newEdge->lEdge)
   {
      if (currPtr->gtrNext == NULL)
      {
         currPtr->gtrNext = newEdge;
         return true;
      }
      return Insert_Edge(currPtr->gtrNext, newEdge);
   }

   currPtr->lEdgeNorm.x += newEdge->lEdgeNorm.x;
   currPtr->lEdgeNorm.y += newEdge->lEdgeNorm.y;
   currPtr->lEdgeNorm.z += newEdge->lEdgeNorm.z;
   currPtr->gEdgeNorm.x += newEdge->gEdgeNorm.x;
   currPtr->gEdgeNorm.y += newEdge->gEdgeNorm.y;
   currPtr->gEdgeNorm.z += newEdge->gEdgeNorm.z;
   return false;
}

OBJEdge* ModelType::Add_New_Edge(OBJEdge *headPtr, int v1, int t1, const OBJVertex& n1, int v2, int t2, const OBJVertex& n2)
{
   if (v1 == v2)
      return headPtr;

   OBJEdge *newEdge = (OBJEdge*)malloc(sizeof(OBJEdge));
   if (v1 < v2)
   {
      newEdge->lEdge = v1;
      newEdge->lEdgeText = t1;
      newEdge->lEdgeNorm.x = n1.x;
      newEdge->lEdgeNorm.y = n1.y;
      newEdge->lEdgeNorm.z = n1.z;
      newEdge->gEdge = v2;
      newEdge->gEdgeText = t2;
      newEdge->gEdgeNorm.x = n2.x;
      newEdge->gEdgeNorm.y = n2.y;
      newEdge->gEdgeNorm.z = n2.z;
   }
   else
   {
      newEdge->lEdge = v2;
      newEdge->lEdgeText = t2;
      newEdge->lEdgeNorm.x = n2.x;
      newEdge->lEdgeNorm.y = n2.y;
      newEdge->lEdgeNorm.z = n2.z;
      newEdge->gEdge = v1;
      newEdge->gEdgeText = t1;
      newEdge->gEdgeNorm.x = n1.x;
      newEdge->gEdgeNorm.y = n1.y;
      newEdge->gEdgeNorm.z = n1.z;
   }
   newEdge->smlNext = NULL;
   newEdge->gtrNext = NULL;

   if (headPtr == NULL)
      return newEdge;

   if (!Insert_Edge(headPtr, newEdge))
      free(newEdge);

   return headPtr;
}

void ModelType::Build_Edge_List()
{
   for (int i = 0; i < numObjects; i++)
   {
      OBJFace *currPtr;

      currPtr = tDispObj->group[i].objTngls;
      while (currPtr != NULL)
      {
         if (currPtr->NormType)
         {
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v1, currPtr->t1, tDispObj->objNorms[currPtr->n1],
                                                                                    currPtr->v2, currPtr->t2, tDispObj->objNorms[currPtr->n2]);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v2, currPtr->t2, tDispObj->objNorms[currPtr->n2],
                                                                                    currPtr->v3, currPtr->t3, tDispObj->objNorms[currPtr->n3]);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v3, currPtr->t3, tDispObj->objNorms[currPtr->n3],
                                                                                    currPtr->v1, currPtr->t1, tDispObj->objNorms[currPtr->n1]);
         }
         else
         {
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v1, currPtr->t1, currPtr->faceNorm,
                                                                                    currPtr->v2, currPtr->t2, currPtr->faceNorm);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v2, currPtr->t2, currPtr->faceNorm,
                                                                                    currPtr->v3, currPtr->t3, currPtr->faceNorm);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v3, currPtr->t3, currPtr->faceNorm,
                                                                                    currPtr->v1, currPtr->t1, currPtr->faceNorm);
         }
         currPtr = currPtr->next;
      }

      currPtr = tDispObj->group[i].objQuads;
      while (currPtr != NULL)
      {
         if (currPtr->NormType)
         {
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v1, currPtr->t1, tDispObj->objNorms[currPtr->n1],
                                                                                    currPtr->v2, currPtr->t2, tDispObj->objNorms[currPtr->n2]);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v2, currPtr->t2, tDispObj->objNorms[currPtr->n2],
                                                                                    currPtr->v3, currPtr->t3, tDispObj->objNorms[currPtr->n3]);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v3, currPtr->t3, tDispObj->objNorms[currPtr->n3],
                                                                                    currPtr->v4, currPtr->t4, tDispObj->objNorms[currPtr->n4]);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v4, currPtr->t4, tDispObj->objNorms[currPtr->n4],
                                                                                    currPtr->v1, currPtr->t1, tDispObj->objNorms[currPtr->n1]);
         }
         else
         {
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v1, currPtr->t1, currPtr->faceNorm,
                                                                                    currPtr->v2, currPtr->t2, currPtr->faceNorm);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v2, currPtr->t2, currPtr->faceNorm,
                                                                                    currPtr->v3, currPtr->t3, currPtr->faceNorm);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v3, currPtr->t3, currPtr->faceNorm,
                                                                                    currPtr->v4, currPtr->t4, currPtr->faceNorm);
            tDispObj->group[i].objEdges = Add_New_Edge(tDispObj->group[i].objEdges, currPtr->v4, currPtr->t4, currPtr->faceNorm,
                                                                                    currPtr->v1, currPtr->t1, currPtr->faceNorm);
         }
         currPtr = currPtr->next;
      }
   }
}

void ModelType::Delete_Face(OBJFace *headPtr)
{
   OBJFace *currPtr = headPtr, *prevPtr;
   while (currPtr != NULL)
   {
      prevPtr = currPtr;
      currPtr = currPtr->next;
      free(prevPtr);
   }
}

void ModelType::Insert_Face(OBJFace *headPtr, OBJFace *newFace)
{
   OBJFace *currPtr = headPtr;
   while (currPtr->next != NULL)
      currPtr = currPtr->next;

   currPtr->next = newFace;
}

bool ModelType::LoadTGA_CMP(FILE *in_file, TextureImage &texture, GLubyte *imageData)
{
   GLuint   bytesPerPixel = texture.bpp / 8;
   GLuint   pixelcount    = texture.height * texture.width;
   GLuint   currentpixel  = 0;
   GLuint   currentbyte   = 0;
   GLubyte *colorbuffer   = (GLubyte*)malloc(bytesPerPixel);

   if (colorbuffer == NULL)
   {
      printf("error, unable to allocate memory!\n");
      return false;
   }

   do
   {
      GLubyte chunkheader = 0;
      if (fread(&chunkheader, sizeof(GLubyte), 1, in_file) == 0)
      {
         printf("error, could not read RLE header.\n");
         free(colorbuffer);
         return false;
      }

      if (chunkheader < 128)
      {
         short counter;
         chunkheader++;
         for (counter = 0; counter < chunkheader; counter++)
         {
            if (fread(colorbuffer, 1, bytesPerPixel, in_file) != bytesPerPixel)
            {
               printf("error, could not read image data.\n");
               free(colorbuffer);
               return false;
            }

            imageData[currentbyte    ] = colorbuffer[2];
			   imageData[currentbyte + 1] = colorbuffer[1];
			   imageData[currentbyte + 2] = colorbuffer[0];

            if (bytesPerPixel == 4)
               imageData[currentbyte + 3] = colorbuffer[3];

            currentbyte += bytesPerPixel;
            currentpixel++;

				if (currentpixel > pixelcount)
				{
					printf("error, too many pixels read.\n");
               free(colorbuffer);
               return false;
				}
			}
		}
		else
		{
         short counter;
			chunkheader -= 127;
			if (fread(colorbuffer, 1, bytesPerPixel, in_file) != bytesPerPixel)
			{
				printf("error, could not read from file.\n");
            free(colorbuffer);
            return false;
         }

         for (counter = 0; counter < chunkheader; counter++)
         {
            imageData[currentbyte    ] = colorbuffer[2];
            imageData[currentbyte + 1] = colorbuffer[1];
            imageData[currentbyte + 2] = colorbuffer[0];

            if (bytesPerPixel == 4)
               imageData[currentbyte + 3] = colorbuffer[3];

            currentbyte += bytesPerPixel;
            currentpixel++;

            if(currentpixel > pixelcount)
            {
               printf("error, too many pixels read.\n");
               free(colorbuffer);
               return false;
            }
         }
      }
   } while (currentpixel < pixelcount);
   free(colorbuffer);

   return true;
}

bool ModelType::LoadTGA_RAW(FILE *in_file, TextureImage &texture, GLubyte *imageData)
{
   GLuint bytesPerPixel = texture.bpp / 8;
   GLuint imageSize = texture.width * texture.height * bytesPerPixel;
   GLuint temp;

   if (fread(imageData, 1, imageSize, in_file) != imageSize)
   {
      printf("error, file appears to be damaged!\n");
      return false;
   }

   for(int i = 0; i < int(imageSize); i += bytesPerPixel)
   {
      temp = imageData[i];
      imageData[i] = imageData[i + 2];
      imageData[i + 2] = temp;
   }

   return true;
}

bool ModelType::LoadTGA(TextureImage &texture, char *filename, int UseMipMaps)
{
   GLubyte  RAW_Header[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   GLubyte  CMP_Header[12] = { 0, 0,10, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   GLubyte  TGAcompare[12];
   GLubyte  header[6];
   GLuint   type = GL_RGBA;
   GLubyte *imageData;

   int fType = 0;

   FILE *in_file = fopen(filename, "rb");

   printf("Loading texture %s: ", filename);

   if (in_file == NULL)
   {
      printf("error, file not found!\n");
      return NULL;
   }

   if (fread(TGAcompare, 1, sizeof(TGAcompare), in_file) != sizeof(TGAcompare))
   {
      printf("error, invalid format!\n");
      fclose(in_file);
      return NULL;
   }

   if (memcmp(RAW_Header, TGAcompare, sizeof(TGAcompare)) == 0)
      fType = 0;
   else if (memcmp(CMP_Header, TGAcompare, sizeof(TGAcompare)) == 0)
      fType = 1;
   else
   {
      printf("error, invalid format!\n");
      fclose(in_file);
      return false;
   }

   if (fread(header, 1, sizeof(header), in_file) != sizeof(header))
   {
      printf("error, invalid format!\n");
      fclose(in_file);
      return false;
   }

   texture.width  = header[1] * 256 + header[0];
   texture.height = header[3] * 256 + header[2];
   texture.bpp    = header[4];
   if (texture.width <= 0 || texture.height <= 0 || (texture.bpp != 24 && texture.bpp != 32))
   {
      printf("error, invalid format!\n");
      fclose(in_file);
      return false;
   }

   imageData = (GLubyte*)malloc(texture.width * texture.height * texture.bpp);
   if (imageData == NULL)
   {
      printf("error, unable to allocate memory!\n");
      fclose(in_file);
      return false;
   }

   if (fType)
   {
      if (!LoadTGA_CMP(in_file, texture, imageData))
      {
         fclose(in_file);
         free(imageData);
         return false;
      }
   }
   else
   {
      if (!LoadTGA_RAW(in_file, texture, imageData))
      {
         fclose(in_file);
         free(imageData);
         return false;
      }
   }
   fclose(in_file);

   glGenTextures(1, &texture.texID);
   if (texture.bpp == 24)
      type = GL_RGB;

   glBindTexture(GL_TEXTURE_2D, texture.texID);
   if (UseMipMaps)
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width, texture.height, type, GL_UNSIGNED_BYTE, (GLvoid *)imageData);
   }
   else
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, type, texture.width, texture.height, 0, type, GL_UNSIGNED_BYTE, (GLvoid *)imageData);
   }

   free(imageData);
   imageData = NULL;

   printf("done.\n");
   if (fType)
      printf("-> Format of texture:    RLE Compressed.\n");
   else
      printf("-> Format of texture:    Uncompressed.\n");
   printf("-> Width of texture:     %d\n", (int)texture.width);
   printf("-> Height of texture:    %d\n", (int)texture.height);
   printf("-> Bitdepth of texture:  %d\n", (int)texture.bpp);
   return true;
}

void ModelType::DeleteTexture(TextureImage &texture)
{
	glDeleteTextures(1, &texture.texID);
}

bool ModelType::ParseMtl(char *filename)
{
   FILE	*in_file;
   char	 oneline[255];
   int    currMtrl = -1;
   float  cVal1, cVal2, cVal3;
   int    bVal;

   printf("Loading material library %s: ", filename);
   in_file = fopen(filename, "rt");
   if (in_file == NULL)
   {
      printf("error, file not found!\n");
      return false;
   }

   while (!feof(in_file))
   {
      if (better_fgets(oneline, 255, in_file) == NULL)
         break;
      
      if (strncmp(oneline, "newmtl", 6) == 0)
      {
         currMtrl++;
         sscanf(oneline, "newmtl %s", tDispObj->mtrl[currMtrl].Name);
      }
      if (strncmp(oneline, "Ka", 2) == 0)
      {
         sscanf(oneline, "Ka %f %f %f", &cVal1, &cVal2, &cVal3);
         tDispObj->mtrl[currMtrl].aCol.x = cVal1;
         tDispObj->mtrl[currMtrl].aCol.y = cVal2;
         tDispObj->mtrl[currMtrl].aCol.z = cVal3;
      }
      if (strncmp(oneline, "Kd", 2) == 0)
      {
         sscanf(oneline, "Kd %f %f %f", &cVal1, &cVal2, &cVal3);
         tDispObj->mtrl[currMtrl].dCol.x = cVal1;
         tDispObj->mtrl[currMtrl].dCol.y = cVal2;
         tDispObj->mtrl[currMtrl].dCol.z = cVal3;
      }
      if (strncmp(oneline, "Ks", 2) == 0)
      {
         sscanf(oneline, "Ks %f %f %f", &cVal1, &cVal2, &cVal3);
         tDispObj->mtrl[currMtrl].sCol.x = cVal1;
         tDispObj->mtrl[currMtrl].sCol.y = cVal2;
         tDispObj->mtrl[currMtrl].sCol.z = cVal3;
      }
      if (strncmp(oneline, "Tr", 2) == 0)
      {
         sscanf(oneline, "Tr %f", &cVal1);
         tDispObj->mtrl[currMtrl].Opacity = cVal1;
      }
      if (strncmp(oneline, "Ns", 2) == 0)
      {
         sscanf(oneline, "Ns %f", &cVal1);
         tDispObj->mtrl[currMtrl].Reflect = cVal1;
      }
      else if (strncmp(oneline, "map_Ka", 6) == 0)
         sscanf(oneline, "map_Ka %s", tDispObj->mtrl[currMtrl].aTextName);
      else if (strncmp(oneline, "map_Kd", 6) == 0)
         sscanf(oneline, "map_Kd %s", tDispObj->mtrl[currMtrl].dTextName);
      else if (strncmp(oneline, "map_Ks", 6) == 0)
         sscanf(oneline, "map_Ks %s", tDispObj->mtrl[currMtrl].sTextName);
      else if (sscanf(oneline, "Smooth %d", &bVal) == 1)
         tDispObj->mtrl[currMtrl].SmthType = bVal;
      else if (sscanf(oneline, "Normal %d", &bVal) == 1)
         tDispObj->mtrl[currMtrl].NormType = bVal;
   }

   printf("done.\n");
   printf("-> Number of materials:  %d\n", ++currMtrl);
   numObjects = currMtrl;
   return true;
}

bool ModelType::ParseObj(char *filename)
{
   FILE  *in_file;
   char	 oneline[255];
   char   mtlname[255];
   int    currGroup = -1;
   float  x, y, z;
   int    iV1, iV2, iV3, iV4, iT1, iT2, iT3, iT4, iN1, iN2, iN3, iN4;

   printf("Loading model %s: ", filename);
   in_file = fopen(filename, "rt");
   if (in_file == NULL)
   {
      printf("error, file not found!\n");
      return false;
   }
   else
      printf("done.\n");

   tDispObj = (OBJModel*)malloc(sizeof(OBJModel));
   tDispObj->NumVerts = 0;
   tDispObj->NumTexts = 0;
   tDispObj->NumNorms = 0;
   tDispObj->NumTngls = 0;
   tDispObj->NumQuads = 0;

   for (int i = 0; i < objMaxNumMats; i++)
   {
      tDispObj->group[i].objTngls = NULL;
      tDispObj->group[i].objQuads = NULL;
      tDispObj->group[i].objEdges = NULL;
      tDispObj->group[i].matID = -1;

      tDispObj->mtrl[i].Name[0] = '\0';
      tDispObj->mtrl[i].aTextName[0] = '\0';
      tDispObj->mtrl[i].dTextName[0] = '\0';
      tDispObj->mtrl[i].sTextName[0] = '\0';
      tDispObj->mtrl[i].NormType = 0;
      tDispObj->mtrl[i].SmthType = 0;
      tDispObj->mtrl[i].aCol.x = 1.0f;
      tDispObj->mtrl[i].aCol.y = 1.0f;
      tDispObj->mtrl[i].aCol.z = 1.0f;
      tDispObj->mtrl[i].dCol.x = 1.0f;
      tDispObj->mtrl[i].dCol.y = 1.0f;
      tDispObj->mtrl[i].dCol.z = 1.0f;
      tDispObj->mtrl[i].sCol.x = 1.0f;
      tDispObj->mtrl[i].sCol.y = 1.0f;
      tDispObj->mtrl[i].sCol.z = 1.0f;
      tDispObj->mtrl[i].Reflect = 1.0f;
      tDispObj->mtrl[i].Opacity = 1.0f;
   }

   while (!feof(in_file))
   {
      if (better_fgets(oneline, 255, in_file) == NULL)
         break;

      if (strncmp(oneline, "mtllib", 6) == 0)
      {
         if (sscanf(oneline, "mtllib %s", mtlname) == 1)
            ParseMtl(mtlname);
      }

      if (strncmp(oneline, "usemtl", 6) == 0)
      {
         int matID = 0;
            
         if ((sscanf(oneline, "usemtl %s", mtlname) == 1) && (numObjects != 0))
         {
            while (strncmp(mtlname, tDispObj->mtrl[matID].Name, 32) != 0)
            {
               matID++;
               if (matID == numObjects)
                  break;
            }
         }
         currGroup = matID;
         tDispObj->group[currGroup].matID = matID;
      }

      if (sscanf(oneline, "v %f %f %f", &x, &y, &z) == 3)
      {
         x *= myScale;
         y *= myScale;
         z *= myScale;
         tDispObj->objVerts[tDispObj->NumVerts].x = x;
         tDispObj->objVerts[tDispObj->NumVerts].y = y;
         tDispObj->objVerts[tDispObj->NumVerts].z = z;
         tDispObj->NumVerts++;
         if (x > maxVerts.x)
            maxVerts.x = x;
         if (y > maxVerts.y)
            maxVerts.y = y;
         if (z > maxVerts.z)
            maxVerts.z = z;
         if (x < minVerts.x)
            minVerts.x = x;
         if (y < minVerts.y)
            minVerts.y = y;
         if (z < minVerts.z)
            minVerts.z = z;
      }
      else if (sscanf(oneline, "vt %f %f %f", &x, &y, &z) == 3)
      {
         tDispObj->objTexts[tDispObj->NumTexts].u = x;
         tDispObj->objTexts[tDispObj->NumTexts].v = y;
         tDispObj->objTexts[tDispObj->NumTexts].w = z;
         tDispObj->NumTexts++;
      }
      else if (sscanf(oneline, "vt %f %f", &x, &y) == 2)
      {
         tDispObj->objTexts[tDispObj->NumTexts].u = x;
         tDispObj->objTexts[tDispObj->NumTexts].v = y;
         tDispObj->objTexts[tDispObj->NumTexts].w = 0.0f;
         tDispObj->NumTexts++;
      }
      else if (sscanf(oneline, "vn %f %f %f", &x, &y, &z) == 3)
      {
         tDispObj->objNorms[tDispObj->NumNorms].x = x;
         tDispObj->objNorms[tDispObj->NumNorms].y = y;
         tDispObj->objNorms[tDispObj->NumNorms].z = z;
         tDispObj->NumNorms++;
      }
      else if (sscanf(oneline, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &iV1, &iT1, &iN1, &iV2, &iT2, &iN2, &iV3, &iT3, &iN3, &iV4, &iT4, &iN4) == 12)
      {
         tDispObj->NumQuads++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = (iV4 < 0) ? (tDispObj->NumVerts + iV4) : --iV4;

         newFace->t1 = (iT1 < 0) ? (tDispObj->NumTexts + iT1) : --iT1;
         newFace->t2 = (iT2 < 0) ? (tDispObj->NumTexts + iT2) : --iT2;
         newFace->t3 = (iT3 < 0) ? (tDispObj->NumTexts + iT3) : --iT3;
         newFace->t4 = (iT4 < 0) ? (tDispObj->NumTexts + iT4) : --iT4;

         newFace->n1 = (iN1 < 0) ? (tDispObj->NumNorms + iN1) : --iN1;
         newFace->n2 = (iN2 < 0) ? (tDispObj->NumNorms + iN2) : --iN2;
         newFace->n3 = (iN3 < 0) ? (tDispObj->NumNorms + iN3) : --iN3;
         newFace->n4 = (iN4 < 0) ? (tDispObj->NumNorms + iN4) : --iN4;

         newFace->NormType = 1;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objQuads == NULL)
            tDispObj->group[currGroup].objQuads = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objQuads, newFace);
      }
      else if (sscanf(oneline, "f %d/%d/%d %d/%d/%d %d/%d/%d", &iV1, &iT1, &iN1, &iV2, &iT2, &iN2, &iV3, &iT3, &iN3) == 9)
      {
         tDispObj->NumTngls++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = -1;

         newFace->t1 = (iT1 < 0) ? (tDispObj->NumTexts + iT1) : --iT1;
         newFace->t2 = (iT2 < 0) ? (tDispObj->NumTexts + iT2) : --iT2;
         newFace->t3 = (iT3 < 0) ? (tDispObj->NumTexts + iT3) : --iT3;
         newFace->t4 = -1;

         newFace->n1 = (iN1 < 0) ? (tDispObj->NumNorms + iN1) : --iN1;
         newFace->n2 = (iN2 < 0) ? (tDispObj->NumNorms + iN2) : --iN2;
         newFace->n3 = (iN3 < 0) ? (tDispObj->NumNorms + iN3) : --iN3;
         newFace->n4 = -1;

         newFace->NormType = 1;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objTngls == NULL)
            tDispObj->group[currGroup].objTngls = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objTngls, newFace);
      }
      else if (sscanf(oneline, "f %d//%d %d//%d %d//%d %d//%d", &iV1, &iN1, &iV2, &iN2, &iV3, &iN3, &iV4, &iN4) == 8)
      {
         tDispObj->NumQuads++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = (iV4 < 0) ? (tDispObj->NumVerts + iV4) : --iV4;

         newFace->t1 = -1;
         newFace->t2 = -1;
         newFace->t3 = -1;
         newFace->t4 = -1;

         newFace->n1 = (iN1 < 0) ? (tDispObj->NumNorms + iN1) : --iN1;
         newFace->n2 = (iN2 < 0) ? (tDispObj->NumNorms + iN2) : --iN2;
         newFace->n3 = (iN3 < 0) ? (tDispObj->NumNorms + iN3) : --iN3;
         newFace->n4 = (iN4 < 0) ? (tDispObj->NumNorms + iN4) : --iN4;

         newFace->NormType = 1;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objQuads == NULL)
            tDispObj->group[currGroup].objQuads = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objQuads, newFace);
      }
      else if (sscanf(oneline, "f %d/%d %d/%d %d/%d %d/%d", &iV1, &iT1, &iV2, &iT2, &iV3, &iT3, &iV4, &iT4) == 8)
      {
         tDispObj->NumQuads++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = (iV4 < 0) ? (tDispObj->NumVerts + iV4) : --iV4;

         newFace->t1 = (iT1 < 0) ? (tDispObj->NumTexts + iT1) : --iT1;
         newFace->t2 = (iT2 < 0) ? (tDispObj->NumTexts + iT2) : --iT2;
         newFace->t3 = (iT3 < 0) ? (tDispObj->NumTexts + iT3) : --iT3;
         newFace->t4 = (iT4 < 0) ? (tDispObj->NumTexts + iT4) : --iT4;

         newFace->n1 = -1;
         newFace->n2 = -1;
         newFace->n3 = -1;
         newFace->n4 = -1;

         newFace->NormType = 0;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objQuads == NULL)
            tDispObj->group[currGroup].objQuads = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objQuads, newFace);
      }
      else if (sscanf(oneline, "f %d//%d %d//%d %d//%d", &iV1, &iN1, &iV2, &iN2, &iV3, &iN3) == 6)
      {
         tDispObj->NumTngls++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = -1;

         newFace->t1 = -1;
         newFace->t2 = -1;
         newFace->t3 = -1;
         newFace->t4 = -1;

         newFace->n1 = (iN1 < 0) ? (tDispObj->NumNorms + iN1) : --iN1;
         newFace->n2 = (iN2 < 0) ? (tDispObj->NumNorms + iN2) : --iN2;
         newFace->n3 = (iN3 < 0) ? (tDispObj->NumNorms + iN3) : --iN3;
         newFace->n4 = -1;

         newFace->NormType = 1;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objTngls == NULL)
            tDispObj->group[currGroup].objTngls = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objTngls, newFace);
      }
      else if (sscanf(oneline, "f %d/%d %d/%d %d/%d", &iV1, &iT1, &iV2, &iT2, &iV3, &iT3) == 6)
      {
         tDispObj->NumTngls++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = -1;

         newFace->t1 = (iT1 < 0) ? (tDispObj->NumTexts + iT1) : --iT1;
         newFace->t2 = (iT2 < 0) ? (tDispObj->NumTexts + iT2) : --iT2;
         newFace->t3 = (iT3 < 0) ? (tDispObj->NumTexts + iT3) : --iT3;
         newFace->t4 = -1;

         newFace->n1 = -1;
         newFace->n2 = -1;
         newFace->n3 = -1;
         newFace->n4 = -1;

         newFace->NormType = 0;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objTngls == NULL)
            tDispObj->group[currGroup].objTngls = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objTngls, newFace);
      }
      else if (sscanf(oneline, "f %d %d %d %d", &iV1, &iV2, &iV3, &iV4) == 4)
      {
         tDispObj->NumQuads++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = (iV4 < 0) ? (tDispObj->NumVerts + iV4) : --iV4;

         newFace->t1 = -1;
         newFace->t2 = -1;
         newFace->t3 = -1;
         newFace->t4 = -1;

         newFace->n1 = -1;
         newFace->n2 = -1;
         newFace->n3 = -1;
         newFace->n4 = -1;

         newFace->NormType = 0;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objQuads == NULL)
            tDispObj->group[currGroup].objQuads = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objQuads, newFace);
      }
      else if (sscanf(oneline, "f %d %d %d", &iV1, &iV2, &iV3) == 3)
      {
         tDispObj->NumTngls++;
         OBJFace *newFace = (OBJFace*)malloc(sizeof(OBJFace));
         newFace->next = NULL;

         newFace->v1 = (iV1 < 0) ? (tDispObj->NumVerts + iV1) : --iV1;
         newFace->v2 = (iV2 < 0) ? (tDispObj->NumVerts + iV2) : --iV2;
         newFace->v3 = (iV3 < 0) ? (tDispObj->NumVerts + iV3) : --iV3;
         newFace->v4 = -1;

         newFace->t1 = -1;
         newFace->t2 = -1;
         newFace->t3 = -1;
         newFace->t4 = -1;

         newFace->n1 = -1;
         newFace->n2 = -1;
         newFace->n3 = -1;
         newFace->n4 = -1;

         newFace->NormType = 0;
         if (currGroup == -1)
            currGroup = numObjects;
         if (tDispObj->group[currGroup].objTngls == NULL)
            tDispObj->group[currGroup].objTngls = newFace;
         else
            Insert_Face(tDispObj->group[currGroup].objTngls, newFace);
      }
   }
   fclose(in_file);

   if (numObjects == 0)
     numObjects++;

   if (fabs(maxVerts.x) > myRadius)
		myRadius = fabs(maxVerts.x);
	if (fabs(maxVerts.y) > myRadius)
		myRadius = fabs(maxVerts.y);
	if (fabs(maxVerts.z) > myRadius)
		myRadius = fabs(maxVerts.z);
   if (fabs(minVerts.x) > myRadius)
      myRadius = fabs(maxVerts.x);
	if (fabs(minVerts.y) > myRadius)
		myRadius = fabs(maxVerts.y);
	if (fabs(minVerts.z) > myRadius)
		myRadius = fabs(maxVerts.z);

   printf("-> Number of vertices:   %d\n", tDispObj->NumVerts);
   printf("-> Number of normals:    %d\n", tDispObj->NumNorms);
   printf("-> Number of tex coords: %d\n", tDispObj->NumTexts);
   printf("-> Number of triangles:  %d\n", tDispObj->NumTngls);
   printf("-> Number of quads:      %d\n", tDispObj->NumQuads);
   return true;
}

void ModelType::DrawEdges(OBJEdge *theEdge)
{
   if (theEdge == NULL)
      return;

   Normalise_Vector(theEdge->lEdgeNorm);
   Normalise_Vector(theEdge->gEdgeNorm);

   glNormal3f(theEdge->lEdgeNorm.x, theEdge->lEdgeNorm.y, theEdge->lEdgeNorm.z);
   if (theEdge->lEdgeText >= 0)
      glTexCoord3f(tDispObj->objTexts[theEdge->lEdgeText].u, tDispObj->objTexts[theEdge->lEdgeText].v, tDispObj->objTexts[theEdge->lEdgeText].w);
   glVertex3d(tDispObj->objVerts[theEdge->lEdge].x, tDispObj->objVerts[theEdge->lEdge].y, tDispObj->objVerts[theEdge->lEdge].z);

   glNormal3f(theEdge->gEdgeNorm.x, theEdge->gEdgeNorm.y, theEdge->gEdgeNorm.z);
   if (theEdge->gEdgeText >= 0)
      glTexCoord3f(tDispObj->objTexts[theEdge->gEdgeText].u, tDispObj->objTexts[theEdge->gEdgeText].v, tDispObj->objTexts[theEdge->gEdgeText].w);
   glVertex3d(tDispObj->objVerts[theEdge->gEdge].x, tDispObj->objVerts[theEdge->gEdge].y, tDispObj->objVerts[theEdge->gEdge].z);

   DrawEdges(theEdge->smlNext);
   DrawEdges(theEdge->gtrNext);
}

void ModelType::GenEdgeObj()
{
   for (int i = 0; i < numObjects; i++)
   {
      if (!tDispObj->mtrl[i].SmthType)
         break;

      cDispObj[i].hasEdges = true;
      cDispObj[i].EdgeList = glGenLists(1);
      glNewList(cDispObj[i].EdgeList, GL_COMPILE);

      glBegin(GL_LINES);
      {
         DrawEdges(tDispObj->group[i].objEdges);
      }
      glEnd();

      glEndList();
   }
}

void ModelType::NormDispObj()
{
   OBJVertex myNormal;

   for (int i = 0; i < objMaxNumVerts; i++)
   {
      tDispObj->objNorms[i].x = 0.0f;
      tDispObj->objNorms[i].y = 0.0f;
      tDispObj->objNorms[i].z = 0.0f;
   }

   for (int i = 0; i < numObjects; i++)
   {
      int      currMat = tDispObj->group[i].matID;
      OBJFace *currPtr;

      currPtr = tDispObj->group[i].objTngls;
      while (currPtr != NULL)
      {
         if (!currPtr->NormType)
         {
            if (tDispObj->mtrl[currMat].NormType)
            {
               Calc_Norm_Vector(tDispObj->objVerts[currPtr->v1], tDispObj->objVerts[currPtr->v2], tDispObj->objVerts[currPtr->v3], myNormal);
               tDispObj->objNorms[currPtr->v1].x += myNormal.x;
               tDispObj->objNorms[currPtr->v1].y += myNormal.y;
               tDispObj->objNorms[currPtr->v1].z += myNormal.z;
               tDispObj->objNorms[currPtr->v2].x += myNormal.x;
               tDispObj->objNorms[currPtr->v2].y += myNormal.y;
               tDispObj->objNorms[currPtr->v2].z += myNormal.z;
               tDispObj->objNorms[currPtr->v3].x += myNormal.x;
               tDispObj->objNorms[currPtr->v3].y += myNormal.y;
               tDispObj->objNorms[currPtr->v3].z += myNormal.z;
               currPtr->n1 = currPtr->v1;
               currPtr->n2 = currPtr->v2;
               currPtr->n3 = currPtr->v3;
               currPtr->NormType = 1;
            }
            else
            {
               Calc_Norm_Vector(tDispObj->objVerts[currPtr->v1], tDispObj->objVerts[currPtr->v2], tDispObj->objVerts[currPtr->v3], currPtr->faceNorm);
            }
         }
         currPtr = currPtr->next;
      }

      currPtr = tDispObj->group[i].objQuads;
      while (currPtr != NULL)
      {
         if (!currPtr->NormType)
         {
            if (tDispObj->mtrl[currMat].NormType)
            {
               Calc_Norm_Vector(tDispObj->objVerts[currPtr->v1], tDispObj->objVerts[currPtr->v2], tDispObj->objVerts[currPtr->v3], myNormal);
               tDispObj->objNorms[currPtr->v1].x += myNormal.x;
               tDispObj->objNorms[currPtr->v1].y += myNormal.y;
               tDispObj->objNorms[currPtr->v1].z += myNormal.z;
               tDispObj->objNorms[currPtr->v2].x += myNormal.x;
               tDispObj->objNorms[currPtr->v2].y += myNormal.y;
               tDispObj->objNorms[currPtr->v2].z += myNormal.z;
               tDispObj->objNorms[currPtr->v3].x += myNormal.x;
               tDispObj->objNorms[currPtr->v3].y += myNormal.y;
               tDispObj->objNorms[currPtr->v3].z += myNormal.z;
               tDispObj->objNorms[currPtr->v4].x += myNormal.x;
               tDispObj->objNorms[currPtr->v4].y += myNormal.y;
               tDispObj->objNorms[currPtr->v4].z += myNormal.z;
               currPtr->n1 = currPtr->v1;
               currPtr->n2 = currPtr->v2;
               currPtr->n3 = currPtr->v3;
               currPtr->n4 = currPtr->v4;
               currPtr->NormType = 1;
            }
            else
            {
               Calc_Norm_Vector(tDispObj->objVerts[currPtr->v1], tDispObj->objVerts[currPtr->v2], tDispObj->objVerts[currPtr->v3], currPtr->faceNorm);
            }
         }
         currPtr = currPtr->next;
      }
   }

   for (int i = 0; i < objMaxNumVerts; i++)
   {
      Normalise_Vector(tDispObj->objNorms[i]);
   }
}

void ModelType::GenDispObj()
{
   for (int i = 0; i < numObjects; i++)
   {
      OBJFace *currPtr;

      if (tDispObj->mtrl[i].dTextName[0] != '\0')
         cDispObj[i].hasTexts = LoadTGA(cDispObj[i].TextMap, tDispObj->mtrl[i].dTextName, 1);
      else
         cDispObj[i].hasTexts = false;
      if (tDispObj->mtrl[i].sTextName[0] != '\0')
         cDispObj[i].hasSpecs = LoadTGA(cDispObj[i].SpecMap, tDispObj->mtrl[i].sTextName, 1);
      else
         cDispObj[i].hasSpecs = false;
      if (tDispObj->mtrl[i].aTextName[0] != '\0')
         cDispObj[i].hasGlows = LoadTGA(cDispObj[i].GlowMap, tDispObj->mtrl[i].aTextName, 1);
      else
         cDispObj[i].hasGlows = false;

      cDispObj[i].SpecQty = tDispObj->mtrl[i].Reflect;
      cDispObj[i].dMat[0] = tDispObj->mtrl[i].dCol.x;
      cDispObj[i].dMat[1] = tDispObj->mtrl[i].dCol.y;
      cDispObj[i].dMat[2] = tDispObj->mtrl[i].dCol.z;
      cDispObj[i].dMat[3] = tDispObj->mtrl[i].Opacity;
      cDispObj[i].aMat[0] = tDispObj->mtrl[i].aCol.x;
      cDispObj[i].aMat[1] = tDispObj->mtrl[i].aCol.y;
      cDispObj[i].aMat[2] = tDispObj->mtrl[i].aCol.z;
      cDispObj[i].aMat[3] = tDispObj->mtrl[i].Opacity;
      cDispObj[i].sMat[0] = tDispObj->mtrl[i].sCol.x;
      cDispObj[i].sMat[1] = tDispObj->mtrl[i].sCol.y;
      cDispObj[i].sMat[2] = tDispObj->mtrl[i].sCol.z;
      cDispObj[i].sMat[3] = tDispObj->mtrl[i].Opacity;

      cDispObj[i].DispList = glGenLists(1);
      glNewList(cDispObj[i].DispList, GL_COMPILE);

      glBegin(GL_TRIANGLES);
      currPtr = tDispObj->group[i].objTngls;
      while (currPtr != NULL)
      {
         if (!currPtr->NormType)
            glNormal3f(currPtr->faceNorm.x, currPtr->faceNorm.y, currPtr->faceNorm.z);
         else
            glNormal3f(tDispObj->objNorms[currPtr->n1].x, tDispObj->objNorms[currPtr->n1].y, tDispObj->objNorms[currPtr->n1].z);
         if (currPtr->t1 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t1].u, tDispObj->objTexts[currPtr->t1].v, tDispObj->objTexts[currPtr->t1].w);
         glVertex3f(tDispObj->objVerts[currPtr->v1].x, tDispObj->objVerts[currPtr->v1].y, tDispObj->objVerts[currPtr->v1].z);

         if (currPtr->NormType)
            glNormal3f(tDispObj->objNorms[currPtr->n2].x, tDispObj->objNorms[currPtr->n2].y, tDispObj->objNorms[currPtr->n2].z);
         if (currPtr->t2 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t2].u, tDispObj->objTexts[currPtr->t2].v, tDispObj->objTexts[currPtr->t2].w);
         glVertex3f(tDispObj->objVerts[currPtr->v2].x, tDispObj->objVerts[currPtr->v2].y, tDispObj->objVerts[currPtr->v2].z);

         if (currPtr->NormType)
            glNormal3f(tDispObj->objNorms[currPtr->n3].x, tDispObj->objNorms[currPtr->n3].y, tDispObj->objNorms[currPtr->n3].z);
         if (currPtr->t3 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t3].u, tDispObj->objTexts[currPtr->t3].v, tDispObj->objTexts[currPtr->t3].w);
         glVertex3f(tDispObj->objVerts[currPtr->v3].x, tDispObj->objVerts[currPtr->v3].y, tDispObj->objVerts[currPtr->v3].z);

         currPtr = currPtr->next;
      }
      glEnd();

      glBegin(GL_QUADS);
      currPtr = tDispObj->group[i].objQuads;
      while (currPtr != NULL)
      {
         if (!currPtr->NormType)
            glNormal3f(currPtr->faceNorm.x, currPtr->faceNorm.y, currPtr->faceNorm.z);
         else
            glNormal3f(tDispObj->objNorms[currPtr->n1].x, tDispObj->objNorms[currPtr->n1].y, tDispObj->objNorms[currPtr->n1].z);
         if (currPtr->t1 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t1].u, tDispObj->objTexts[currPtr->t1].v, tDispObj->objTexts[currPtr->t1].w);
         glVertex3f(tDispObj->objVerts[currPtr->v1].x, tDispObj->objVerts[currPtr->v1].y, tDispObj->objVerts[currPtr->v1].z);

         if (currPtr->NormType)
            glNormal3f(tDispObj->objNorms[currPtr->n2].x, tDispObj->objNorms[currPtr->n2].y, tDispObj->objNorms[currPtr->n2].z);
         if (currPtr->t2 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t2].u, tDispObj->objTexts[currPtr->t2].v, tDispObj->objTexts[currPtr->t2].w);
         glVertex3f(tDispObj->objVerts[currPtr->v2].x, tDispObj->objVerts[currPtr->v2].y, tDispObj->objVerts[currPtr->v2].z);

         if (currPtr->NormType)
            glNormal3f(tDispObj->objNorms[currPtr->n3].x, tDispObj->objNorms[currPtr->n3].y, tDispObj->objNorms[currPtr->n3].z);
         if (currPtr->t3 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t3].u, tDispObj->objTexts[currPtr->t3].v, tDispObj->objTexts[currPtr->t3].w);
         glVertex3f(tDispObj->objVerts[currPtr->v3].x, tDispObj->objVerts[currPtr->v3].y, tDispObj->objVerts[currPtr->v3].z);

         if (currPtr->NormType)
            glNormal3f(tDispObj->objNorms[currPtr->n4].x, tDispObj->objNorms[currPtr->n4].y, tDispObj->objNorms[currPtr->n4].z);
         if (currPtr->t4 > 0)
            glTexCoord3f(tDispObj->objTexts[currPtr->t4].u, tDispObj->objTexts[currPtr->t4].v, tDispObj->objTexts[currPtr->t4].w);
         glVertex3f(tDispObj->objVerts[currPtr->v4].x, tDispObj->objVerts[currPtr->v4].y, tDispObj->objVerts[currPtr->v4].z);

         currPtr = currPtr->next;
      }
      glEnd();

      glEndList();
   }
}

char* better_fgets(char *line, int len, FILE *in_file)
{
   char *temp = line;
   int   val;

   if (--len < 0)
      return NULL;

   if (len)
   {
      do
      {
         val = getc(in_file);

         if (val == EOF)
         {
            if (feof(in_file) && temp != line)
               break;
            else
            {
               line = NULL;
               return NULL;
            }
         }
         *temp++ = val;
      } while (val != '\r' && val != '\n' && --len);
   }
   *temp = '\0';

   return line;
}


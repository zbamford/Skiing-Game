/*
 * GlGeomCone.h - Version 0.9 - November 18, 2022
*
* C++ class for rendering cones in Modern OpenGL.
*   A GlGeomCone object encapsulates a VAO, a VBO, and an EBO,
*   which can be used to render a cone.
*   The number of slices and stacks and rings can be varied.
*
* Author: Sam Buss
*
* Software accompanying POSSIBLE SECOND EDITION TO the book
*		3D Computer Graphics: A Mathematical Introduction with OpenGL,
*		by S. Buss, Cambridge University Press, 2003.
*
* Software is "as-is" and carries no warranty.  It may be used without
*   restriction, but if you modify it, please change the filenames to
*   prevent confusion between different versions.
* Bug reports: Sam Buss, sbuss@ucsd.edu.
* Web page: http://math.ucsd.edu/~sbuss/MathCG2
*/

#ifndef GLGEOM_CONE_H
#define GLGEOM_CONE_H

#include "GlGeomBase.h"
#include <limits.h>

// GlGeomCone
//     Generates vertices, normals, and texture coodinates for a cone.
//     The cone is formed of "slices" and "stacks" and "rings"
//     The cone has base radius 1 and height 1, it is centered on
//          y-axis; the tip (apex) is at (0,1,0) and the center of
//          its circular base is at the origin.
// Supports:
//    (1) Allocating and loading a VAO, VBO, and EBO
//    (2) Rendering the cone with OpenGL.
// How to use:
//          First call either the constructor or Remesh() to set the numbers 
//          of slices, stacks and rings.
//          These values can be changed later by calling Remesh()
//    * Then call InitializeAttribLocations() to
//            give locations in the VBO buffer for the shader program.
//            This loads all the vertex data into the VBO and EBO.
//    * Call Render() - to render the cone.  This gives the glDrawElements 
//            commands for the cone using the VAO, VBO and EBO.


class GlGeomCone : public GlGeomBase
{
public:
    GlGeomCone() : GlGeomCone(3, 1, 1) {}
    GlGeomCone(int slices, int stacks=1, int rings=1);

	// Remesh() - Re-mesh to change the number slices and stacks and rings.
    // Can be called either before or after InitializeAttribLocations(), but it is
    //    more efficient if Remesh() is called first, or if the constructor sets the mesh resolution.
    void Remesh(int slices, int stacks, int rings);

	// Allocate the VAO, VBO, and EBO.
	// Set up info about the Vertex Attribute Locations
	// This must be called before Render() is first called.
    // First parameter is the location for the vertex position vector in the shader program.
    // Second parameter is the location for the vertex normal vector in the shader program.
    // Third parameter is the location for the vertex 2D texture coordinates in the shader program.
    // The second and third parameters are optional.
    void InitializeAttribLocations(
		unsigned int pos_loc, unsigned int normal_loc = UINT_MAX, unsigned int texcoords_loc = UINT_MAX);

    void Render();          // Render: renders entire cone
    void RenderBase();
    void RenderSide();

    int GetNumSlices() const { return numSlices; }
    int GetNumStacks() const { return numStacks; }
    int GetNumRings() const { return numRings; }
    
    // Use GetNumElements() and GetNumVerticesTexCoords() and GetNumVerticesNoTexCoords()
    //    to determine the amount of data that will returned by CalcVboAndEbo.
    //    Numbers are different since texture coordinates must be assigned differently
    //        to some vertices depending on which triangle they appear in.
    int GetNumElements() const { return GetNumElementsDisk() + GetNumElementsSide(); }
    int GetNumVerticesTexCoords() const { return GetNumVerticesDisk() + GetNumVerticesSideTexCoords(); }
    int GetNumVerticesNoTexCoords() const { return GetNumVerticesDisk() + GetNumVerticesSideNoTexCoords(); }

    // "Disk" methods are for the circular base.  "Side" for the cone's side
    int GetNumElementsDisk() const { return 3 * (2 * numRings - 1)*numSlices; }
    int GetNumVerticesDisk() const { return 1 + numRings * numSlices; }
    int GetNumElementsSide() const { return 3 * (2*numStacks-1)*numSlices; }
    int GetNumVerticesSideTexCoords() const { return numSlices + numStacks*(numSlices + 1); }
    int GetNumVerticesSideNoTexCoords() const { return numSlices + numStacks*numSlices; }

    // CalcVboAndEbo- return all VBO vertex information, and EBO elements for GL_TRIANGLES drawing.
    // See GlGeomBase.h for additional information
    void CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
        int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset,
        unsigned int stride);

private: 

    // Disable all copy and assignment operators.
    // A GlGeomCone can be allocated as a global or static variable, or with new.
    //     If you need to pass it to/from a function, use references or pointers
    //     and be sure that there are no implicit copy or assignment operations!
    GlGeomCone(const GlGeomCone&) = delete;
    GlGeomCone& operator=(const GlGeomCone&) = delete;
    GlGeomCone(GlGeomCone&&) = delete;
    GlGeomCone& operator=(GlGeomCone&&) = delete;


private:
    int numSlices;          // Number of radial slices (like cake slices
    int numStacks;          // Number of stacks from base to the apex
    int numRings;           // Number of concentric rings on circular base


private: 
    bool VboEboLoaded = false;

    void PreRender();

    void SetBaseVert(float x, float z, int i, int j, float* VBOdataBuffer,
        int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, int stride);
 };

// Constructor
inline GlGeomCone::GlGeomCone(int slices, int stacks, int rings)
{
	numSlices = slices;
	numStacks = stacks;
    numRings = rings;
}

#endif  // GLGEOM_CONE_H

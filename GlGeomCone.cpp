/*
* GlGeomCone.cpp - Version 0.9 - November 18, 2022.
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

// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "GlGeomCone.h"
#include "MathMisc.h"
#include "assert.h"

void GlGeomCone::Remesh(int slices, int stacks, int rings)
{
    if (slices == numSlices && stacks == numStacks && rings == numRings) {
        return;
    }
    numSlices = ClampRange(slices, 3, 255);
    numStacks = ClampRange(stacks, 1, 255);
    numRings = ClampRange(rings, 1, 255);

    VboEboLoaded = false;
}

void GlGeomCone::CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
    int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, unsigned int stride)
{
    assert(vertPosOffset >= 0 && stride > 0);
    bool calcNormals = (vertNormalOffset >= 0);       // Should normals be calculated?
    bool calcTexCoords = (vertTexCoordsOffset >= 0);  // Should texture coordinates be calculated?

    // VBO Data is laid out: base vertices, then side vertices including apex vertices

    // Set base center vertices
    SetBaseVert(0.0, 0.0, 0, 0, VBOdataBuffer, vertPosOffset, vertNormalOffset, vertTexCoordsOffset, stride);
    int stopSlices = calcTexCoords ? numSlices : numSlices - 1;
    for (int i = 0; i <= stopSlices; i++) {
        // Handle a slice of vertices.
        // theta measures from the negative z-axis, counterclockwise viewed from above.
        float theta = ((float)(i % numSlices)) * (float)PI2 / (float)(numSlices);
        float c = -cosf(theta);      // Negated values (start at negative z-axis)
        float s = -sinf(theta);
        if (i < numSlices) {
            // Base vertex position and normal and texture coordinates
            for (int j = 1; j <= numRings; j++) {
                float radius = (float)j / (float)numRings;
                SetBaseVert(s * radius, c * radius, i, j, VBOdataBuffer, vertPosOffset, vertNormalOffset, vertTexCoordsOffset, stride);
            }
        }
        // sidePtr points to vertex data entries
        float* sidePtr = VBOdataBuffer + (GetNumVerticesDisk() + i * (numStacks + 1)) * stride;
        float sCoord = ((float)i) / (float)(numSlices);
        // Side vertices, positions and normals and texture coordinates (except apex)
        // Starts at the base and goes up towards the apex.
        for (int j = 0; j < numStacks; j++, sidePtr += stride) {
            float* vPtr = sidePtr + vertPosOffset;
            float tCoord = (float)j / (float)numStacks;
            float slopeFactor = 1.0f - tCoord;
            *(vPtr++) = s * slopeFactor;
            *(vPtr++) = tCoord;
            *vPtr = c * slopeFactor;
            if (calcNormals) {
                float* nPtr = sidePtr + vertNormalOffset;
                *(nPtr++) = s * sqrtf(2.0f);
                *(nPtr++) = sqrtf(2.0f);
                *nPtr = c * sqrtf(2.0f);
            }
            if (calcTexCoords) {
                float* tcPtr = sidePtr + vertTexCoordsOffset;
                *(tcPtr++) = sCoord;
                *tcPtr = tCoord;
            }
        }
        if ( i <numSlices ) {
            // Apex vertex with position, normal and texture coordinates
            float* vPtr = sidePtr + vertPosOffset;
            *vPtr++ = 0.0f;
            *vPtr++ = 1.0f;  // Apex position
            *vPtr = 0.0f;
            if (calcNormals) {
                float* nPtr = sidePtr + vertNormalOffset;
                // Use theta midway between the two side strips
                float thetaApex = ((float)(2 * i + 1)) * (float)PI2 / (float)(2 * numSlices);
                *(nPtr++) = -sinf(thetaApex) * sqrtf(2.0f);
                *(nPtr++) = sqrtf(2.0f);
                *nPtr = -cosf(thetaApex) * sqrtf(2.0f);
            }
            if (calcTexCoords) {
                float* tcPtr = sidePtr + vertTexCoordsOffset;
                // Use top center of the texture map for the apex text coordinates
                *(tcPtr++) = 0.5f;
                *tcPtr = 1.0f;
            }
        }
    }

    // EBO data is also laid out as base, then side
    unsigned int* eboPtr = EBOdataBuffer;
    // Bsae 
    for (int i = 0; i < numSlices; i++) {
        int r = i * numRings + 1;
        int rightR = ((i + 1) % numSlices) * numRings + 1;
        *(eboPtr++) = 0;
        *(eboPtr++) = rightR;
        *(eboPtr++) = r;
        for (int j = 0; j < numRings - 1; j++) {
            *(eboPtr++) = r + j;
            *(eboPtr++) = rightR + j;
            *(eboPtr++) = rightR + j + 1;

            *(eboPtr++) = r + j;
            *(eboPtr++) = rightR + j + 1;
            *(eboPtr++) = r + j + 1;
        }
    }
    // Side
    for (int i = 0; i < numSlices; i++) {
        int r = i * (numStacks + 1) + GetNumVerticesDisk();
        int ii = calcTexCoords ? (i + 1) : (i + 1) % numSlices;
        int rightR = ii * (numStacks + 1) + GetNumVerticesDisk();
        int j;
        for (j = 0; j < numStacks-1; j++) {
            *(eboPtr++) = rightR + j;
            *(eboPtr++) = r + j + 1;
            *(eboPtr++) = r + j;

            *(eboPtr++) = rightR + j;
            *(eboPtr++) = rightR + j + 1;
            *(eboPtr++) = r + j + 1;
        }
        // The apex vertex that ends this strip of GL_TRIANGLES
        *(eboPtr++) = rightR + j;
        *(eboPtr++) = r + j + 1;
        *(eboPtr++) = r + j;
    }
}

// Set a vertex in the base.
void GlGeomCone::SetBaseVert(float x, float z, int i, int j, float* VBOdataBuffer,
    int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, int stride)
{
    // i is the slice number, j is the ring number.
    // j==0 means the center point.  In this case, i must equal 0. (Not checked)
    float* basePtrBottom = VBOdataBuffer + stride * (i * numRings + j);
    float* vPtrBottom = basePtrBottom + vertPosOffset;
    *(vPtrBottom++) = x;
    *(vPtrBottom++) = 0.0;
    *vPtrBottom = z;
    if (vertNormalOffset >= 0) {
        float* nPtrBottom = basePtrBottom + vertNormalOffset;
        *(nPtrBottom++) = 0.0;
        *(nPtrBottom++) = -1.0;
        *nPtrBottom = 0.0;
    }
    if (vertTexCoordsOffset >= 0) {
        float sCoord = 0.5f * (1.0f - x);
        float tCoord = 0.5f * (1.0f - z);
        float* tcPtrBottom = basePtrBottom + vertTexCoordsOffset;
        *(tcPtrBottom++) = 0.5f * (1.0f - x);      // s - Coordinate
            * tcPtrBottom = 0.5f * (1.0f - z);     // t-Coordinate
    }
}

void GlGeomCone::InitializeAttribLocations(
    unsigned int pos_loc, unsigned int normal_loc, unsigned int texcoords_loc)
{
    // The call to GlGeomBase::InitializeAttribLocations will further call
    //   GlGeomSphere::CalcVboAndEbo()

    GlGeomBase::InitializeAttribLocations(pos_loc, normal_loc, texcoords_loc);
    VboEboLoaded = true;
}


// **********************************************
// These routines do the rendering.
// If the cylinder's VAO, VBO, EBO need to be loaded, it does this first.
// **********************************************

void GlGeomCone::PreRender()
{
    GlGeomBase::PreRender();

    if (!VboEboLoaded) {
        ReInitializeAttribLocations();
    }
}

void GlGeomCone::Render()
{
    PreRender();
    GlGeomBase::Render();
}

void GlGeomCone::RenderBase()
{
    PreRender();

    GlGeomBase::RenderEBO(GL_TRIANGLES, GetNumElementsDisk(), 0);
}

void GlGeomCone::RenderSide()
{
    PreRender();

    GlGeomBase::RenderEBO(GL_TRIANGLES, GetNumElementsSide(), GetNumElementsDisk());
}





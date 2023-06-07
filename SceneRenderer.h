#pragma once

#include <vector>

//
// Function Prototypes
//
void MySetupSurfaces();                // Called once, before rendering begins.
void SetupForTextures();               // Loads textures, sets Phong material

std::vector<std::pair<float, float>> RenderScene(float xPos, float zPos); // Renders the entire scene




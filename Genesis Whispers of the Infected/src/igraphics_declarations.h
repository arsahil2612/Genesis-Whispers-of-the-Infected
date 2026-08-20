#ifndef IGRAPHICS_DECLARATIONS_H
#define IGRAPHICS_DECLARATIONS_H

#include "../iGraphics.h"
#include <string>

// Helper for resolving asset paths across build directories
std::string GetAssetPath(const std::string& relativePath);

#endif // IGRAPHICS_DECLARATIONS_H

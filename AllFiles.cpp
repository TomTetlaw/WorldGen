#define STB_SPRINTF_IMPLEMENTATION 
#define STB_IMAGE_IMPLEMENTATION
#define STB_PERLIN_IMPLEMENTATION
#define STBI_MALLOC(s) ArenaAllocate(&GlobalArena, s)
#define STBI_REALLOC_SIZED(p,os,ns) __ArenaRealloc(&GlobalArena, p, os, ns)
#define STBI_FREE

#include "Includes.h"

#undef max
#undef min

#ifdef DEBUG_BUILD
#define TRACY_ENABLE
#include "Tracy/Tracy.hpp"
#include "Tracy/TracyC.h"
#include "Tracy/TracyOpenGL.hpp"
#define TracyBegin(n) TracyCZoneN(tracy_##n, #n, 1)
#define TracyEnd(n) TracyCZoneEnd(tracy_##n)
#else
#define TracyBegin(n)
#define TracyEnd(n)
#endif

#include "Platform.cpp"
#include "Memory.cpp"
#include "Utils.cpp"
#include "Renderer.cpp"
#include "Game.cpp"
#include "Shader.cpp"
#include "Entity.cpp"
#include "Texture.cpp"

#include "OpenGL.Generated.cpp"

#ifdef WINDOWS_BUILD
#include "Win32Main.cpp"
#endif
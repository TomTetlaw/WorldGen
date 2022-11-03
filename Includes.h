#ifndef _INCLUDES_H
#define _INCLUDES_H

#ifdef WINDOWS_BUILD
#include <Windows.h>
#include "Include/CoreKHR.h"
#include "Include/CoreOpenGL.h"
#include "Include/CoreWGL.h"
#endif

#include "OpenGL.Generated.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Types.h"
#include "Memory.h"
#include "Include/stb_sprintf.h"
#include "Include/stb_image.h"
#include "Include/stb_perlin.h"

#include "Utils.h"
#include "Math.h"
#include "Platform.h"
#include "Renderer.h"
#include "Shader.h"
#include "Game.h"
#include "Texture.h"
#include "Entity.h"

#endif //_INCLUDES_H

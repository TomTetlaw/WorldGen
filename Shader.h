#ifndef _SHADER_H
#define _SHADER_H

u32 LoadShader(s8 *VertexFile, s8 *PixelFile);

void ShaderSetMatrix(u32 Shader, s8 *Name, mat4 Matrix);

#endif //_SHADER_H

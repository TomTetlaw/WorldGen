#ifndef _SHADER_H
#define _SHADER_H

typedef void (*material_func)(shader_instance *Shader);

struct shader_instance {
    u32 ID;
    void *UserData;
    material_func MaterialFunction;
};

u32 LoadShader(s8 *VertexFile, s8 *PixelFile);

void CreateShaderInstance(shader_instance *Instance, material_func Func, void *UserData, u32 Shader);

void ShaderSetMatrix(shader_instance *Shader, s8 *Name, mat4 Matrix);
void ShaderSetInt(shader_instance *Shader, s8 *Name, s32 Value);
void ShaderSetFloat(shader_instance *Shader, s8 *Name, f32 Value);

#endif //_SHADER_H

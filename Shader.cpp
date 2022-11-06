#include "Includes.h"

u32 LoadShader(s8 *VertexFile, s8 *FragmentFile) {
    GlobalPushMark();
    
    file VertexShaderFile = PlatformLoadFile(VertexFile, true, &GlobalArena);
    file FragmentShaderFile = PlatformLoadFile(FragmentFile, true, &GlobalArena);
    
    s8 *VertexSources[] = { "#version 330\n", VertexShaderFile.Data };
    s8 *FragmentSources[] = { "#version 330\n", FragmentShaderFile.Data };
    u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
    u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(VertexShader, 2, VertexSources, 0);
    glCompileShader(VertexShader);
    glShaderSource(FragmentShader, 2, FragmentSources, 0);
    glCompileShader(FragmentShader);
    
    GlobalPopMark();
    
    u32 Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    
    glLinkProgram(Program);
    
    s32 Linked = 0;
    glGetProgramiv(Program, GL_LINK_STATUS, &Linked);
    if(!Linked) {
        s8 VertexLog[1024];
        s8 FragmentLog[1024];
        s8 ProgramLog[1024];
        
        glGetShaderInfoLog(VertexShader, 1024, 0, VertexLog);
        glGetShaderInfoLog(FragmentShader, 1024, 0, FragmentLog);
        glGetProgramInfoLog(Program, 1024, 0, ProgramLog);
        
        PlatformFatalError("Failed to compile shader %s %s:\n VertexLog %s\n FragmentLog %s\n ProgramLog %s", VertexFile, FragmentFile, VertexLog, FragmentLog, ProgramLog);
    }
    
    glDetachShader(Program, VertexShader);
    glDetachShader(Program, FragmentShader);
    
    return Program;
}

void CreateShaderInstance(shader_instance *Instance, material_func Func, void *UserData, u32 Shader) {
    Instance->MaterialFunction = Func;
    Instance->UserData = UserData;
    Instance->ID = Shader;
}

void ShaderSetMatrix(shader_instance *Shader, s8 *Name, mat4 Matrix) {
    u32 Location = glGetUniformLocation(Shader->ID, Name);
    glUniformMatrix4fv(Location, 1, GL_FALSE, Matrix.e);
}

void ShaderSetInt(shader_instance *Shader, s8 *Name, s32 Value) {
    u32 Location = glGetUniformLocation(Shader->ID, Name);
    glUniform1i(Location, Value);
}

void ShaderSetFloat(shader_instance *Shader, s8 *Name, f32 Value) {
    u32 Location = glGetUniformLocation(Shader->ID, Name);
    glUniform1f(Location, Value);
}
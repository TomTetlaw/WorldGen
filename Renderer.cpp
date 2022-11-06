#include "Includes.h"

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

typedef u32 index;

struct vertex {
    vec2 Position;
    vec2 TexCoord;
    colour Colour;
};

struct render_command {
    s32 FirstIndex;
    s32 FirstVertex;
    s32 IndexCount;
    s32 VertexCount;
    index *Indices;
    vertex *Vertices;
    shader_instance *Shader;
    texture *Texture;
};

struct renderer_cache {
    arena Indices;
    arena Vertices;
    arena RenderCommands;
    
    s32 MaxIndices;
    s32 MaxVertices;
    s32 MaxRenderCommands;
    
    u32 VertexBuffer;
    u32 IndexBuffer;
    u32 VertexArray;
    
    shader_instance Basic;
    shader_instance Textured;
};

renderer_cache Cache;

render_command *GetRenderCommand(s32 IndexCount, s32 VertexCount) {
    s32 FirstIndex = Cache.Indices.Used / sizeof(index);
    s32 FirstVertex = Cache.Vertices.Used / sizeof(vertex);
    
    vertex *Vertices = (vertex *)ArenaAllocate(&Cache.Vertices, sizeof(vertex) * VertexCount);
    index *Indices = (index *)ArenaAllocate(&Cache.Indices, sizeof(index) * IndexCount);
    render_command *Command = (render_command *)ArenaAllocate(&Cache.RenderCommands, sizeof(render_command));
    
    Command->FirstIndex = FirstIndex;
    Command->FirstVertex = FirstVertex;
    Command->IndexCount = IndexCount;
    Command->VertexCount = VertexCount;
    Command->Indices = Indices;
    Command->Vertices = Vertices;
    
    return Command;
}

void InitRenderer() {
    Cache.MaxIndices = 500000;
    Cache.MaxVertices = 500000;
    Cache.MaxRenderCommands = 50000;
    
    MakeSubArena(&Cache.Indices, &RendererArena, "Indices", sizeof(index) * Cache.MaxIndices);
    MakeSubArena(&Cache.Vertices, &RendererArena, "Vertices", sizeof(vertex) * Cache.MaxVertices);
    MakeSubArena(&Cache.RenderCommands, &RendererArena, "RenderCommands",  sizeof(render_command) * Cache.MaxRenderCommands);
    
    glGenVertexArrays(1, &Cache.VertexArray);
    glBindVertexArray(Cache.VertexArray);
    
    glGenBuffers(1, &Cache.VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Cache.VertexBuffer);
    glObjectLabel(GL_BUFFER, Cache.VertexBuffer, -1, "VertexBuffer");
    glBufferData(GL_ARRAY_BUFFER, Cache.MaxVertices * sizeof(vertex), 0, GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &Cache.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cache.IndexBuffer);
    glObjectLabel(GL_BUFFER, Cache.IndexBuffer, -1, "IndexBuffer");
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Cache.MaxIndices * sizeof(index), 0, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, Position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(vertex), (const void *)offsetof(vertex, TexCoord));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(vertex), (const void *)offsetof(vertex, Colour));
    
    u32 BasicShader = LoadShader("Shaders/Basic.vs", "Shaders/Basic.fs");
    u32 TexturedShader = LoadShader("Shaders/Textured.vs", "Shaders/Textured.fs");
    CreateShaderInstance(&Cache.Basic, 0, 0, BasicShader);
    CreateShaderInstance(&Cache.Textured, 0, 0, TexturedShader);
    
    PlatformDeleteFile("OpenGL.log");
    
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback((GLDEBUGPROC)DebugCallback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
}

void PushLine(renderer_line *Line) {
    vec2 Diff = Line->B - Line->A;
    vec2 Normal = Normalise(v2(-Diff.y, Diff.x));
    
    vec2 a0 = Line->A;
    vec2 a1 = Line->A + Normal*Line->Thickness;
    vec2 b0 = Line->B;
    vec2 b1 = Line->B + Normal*Line->Thickness;
    
    vertex Vertices[4] = {
        { a0, v2(0.f, 0.f), Line->Colour },
        { a1, v2(1.f, 0.f), Line->Colour },
        { b0, v2(1.f, 1.f), Line->Colour },
        { b1, v2(0.f, 1.f), Line->Colour },
    };
    
    index Indices[6] = {
        0, 1, 2, 0, 2, 3
    };
    
    render_command *Command = GetRenderCommand(6, 4);
    Copy(Command->Vertices, Vertices, sizeof(vertex) * 4);
    Copy(Command->Indices, Indices, sizeof(index) * 6);
    
    Command->Shader = &Cache.Basic;
}

void PushQuad(renderer_quad *Quad) {
    vertex Vertices[4] = {
        { Quad->Position + v2(0.f, 0.f),                   v2(0.f, 0.f), Quad->Colour },
        { Quad->Position + v2(Quad->Size.x, 0.f),          v2(1.f, 0.f), Quad->Colour },
        { Quad->Position + v2(Quad->Size.x, Quad->Size.y), v2(1.f, 1.f), Quad->Colour },
        { Quad->Position + v2(0.f, Quad->Size.y),          v2(0.f, 1.f), Quad->Colour },
    };
    
    index Indices[6] = {
        0, 1, 2, 3, 0, 2
    };
    
    render_command *Command = GetRenderCommand(6, 4);
    Copy(Command->Vertices, Vertices, sizeof(vertex) * 4);
    Copy(Command->Indices, Indices, sizeof(index) * 6);
    
    if(Quad->Shader) Command->Shader = Quad->Shader;
    else Command->Shader = &Cache.Basic;
}

void PushQuads(renderer_quad *Quads, s32 QuadCount) {
    ZoneScopedN("PushQuads");
    
    s32 VertCount = QuadCount * 4;
    s32 IndexCount = QuadCount * 6;
    
    render_command *Command = GetRenderCommand(IndexCount, VertCount);
    vertex *Vertices = Command->Vertices;
    index *Indices = Command->Indices;
    
    s32 Vert = 0;
    s32 Index = 0;
    for(s32 i = 0; i < QuadCount; i++) {
        renderer_quad *Quad = Quads + i;
        
        vertex QuadVertices[4] = {
            { Quad->Position + v2(0.f, 0.f),                   v2(0.f, 0.f), Quad->Colour },
            { Quad->Position + v2(Quad->Size.x, 0.f),          v2(1.f, 0.f), Quad->Colour },
            { Quad->Position + v2(Quad->Size.x, Quad->Size.y), v2(1.f, 1.f), Quad->Colour },
            { Quad->Position + v2(0.f, Quad->Size.y),          v2(0.f, 1.f), Quad->Colour },
        };
        
        index QuadIndices[6] = {
            Vert + 0u, Vert + 1u, Vert + 2u, Vert + 3u, Vert + 0u, Vert + 2u
        };
        
        Copy(&Vertices[Vert], QuadVertices, sizeof(vertex) * 4);
        Copy(&Indices[Index], QuadIndices, sizeof(index) * 6);
        
        Vert += 4;
        Index += 6;
    }
    
    Command->Shader = &Cache.Basic;
}

void PushTexture(renderer_quad *Quad, texture *Texture) {
    vertex Vertices[4] = {
        { Quad->Position + v2(0.f, 0.f),                   v2(0.f, 0.f), Quad->Colour },
        { Quad->Position + v2(Quad->Size.x, 0.f),          v2(1.f, 0.f), Quad->Colour },
        { Quad->Position + v2(Quad->Size.x, Quad->Size.y), v2(1.f, 1.f), Quad->Colour },
        { Quad->Position + v2(0.f, Quad->Size.y),          v2(0.f, 1.f), Quad->Colour },
    };
    
    index Indices[6] = {
        0, 1, 2, 3, 0, 2
    };
    
    render_command *Command = GetRenderCommand(6, 4);
    Copy(Command->Vertices, Vertices, sizeof(vertex) * 4);
    Copy(Command->Indices, Indices, sizeof(index) * 6);
    
    if(Quad->Shader) Command->Shader = Quad->Shader;
    else Command->Shader = &Cache.Textured;
    Command->Texture = Texture;
}

void RendererBeginFrame() {
}

u32 LastProgram = 999999;
void RendererEndFrame() {
    TracyGpuZone("SubmitCommands");
    
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, Cache.Indices.Used, Cache.Indices.Data);
    glBufferSubData(GL_ARRAY_BUFFER, 0, Cache.Vertices.Used, Cache.Vertices.Data);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    s32 RenderCommandCount = Cache.RenderCommands.Used / sizeof(render_command);
    for(s32 i = 0; i < RenderCommandCount; i++) {
        render_command *Command = (render_command *)Cache.RenderCommands.Data + i;
        
        if(LastProgram != Command->Shader->ID) {
            glUseProgram(Command->Shader->ID);
            LastProgram = Command->Shader->ID;
        }
        
        ShaderSetMatrix(Command->Shader, "OrthoMatrix", CreateOrthoMatrix(0.f, 1366.f, 768.f, 0.f));
        
        if(Command->Shader->MaterialFunction) {
            Command->Shader->MaterialFunction(Command->Shader);
        }
        
        if(Command->Texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(Command->Texture->Target, Command->Texture->ID);
            u32 Location = glGetUniformLocation(Command->Shader->ID, "Texture0");
            glUniform1i(Location, 0);
        }
        
        glDrawElementsBaseVertex(GL_TRIANGLES, Command->IndexCount, GL_UNSIGNED_INT, (index *)(Command->FirstIndex * sizeof(index)), Command->FirstVertex);
    }
    
    Cache.Indices.Used = 0;
    Cache.Vertices.Used = 0;
    Cache.RenderCommands.Used = 0;
}


void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    s8 *String = Format("%s\n", message);
    PlatformAppendFile("OpenGL.log", String, StringLength(String));
#ifdef WINDOWS_BUILD
#ifdef DEBUG_BUILD
    OutputDebugString(String);
#endif
#endif
}
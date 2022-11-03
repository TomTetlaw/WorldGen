#ifndef _RENDERER_H
#define _RENDERER_H

struct renderer_quad {
    vec2 Position;
    vec2 Size;
    colour Colour;
    s32 Shader;
};

struct renderer_line {
    vec2 A;
    vec2 B;
    f32 Thickness;
    colour Colour;
};

void InitRenderer();
void RendererBeginFrame();
void RendererEndFrame();

void PushLine(renderer_line *Line);
void PushQuad(renderer_quad *Quad);
void PushQuads(renderer_quad *Quads, s32 QuadCount);

struct texture;
void PushTexture(renderer_quad *Quad, texture *Texture);

#endif //_RENDERER_H

#ifndef _ENTITY_H
#define _ENTITY_H

struct entity {
    vec2 Position;
    vec2 Size;
    vec4 Colour;
    vec2i TilePosition;
    
    texture *Texture;
    
    s32 Parity;
    bool Free;
    bool Active;
    entity *Prev;
};

void InitEntitySystem();

entity *CreateEntity();
void RemoveEntity(entity *Entity);
void RemoveAllEntities();

struct world_data;
void UpdateAllEntities(platform *Platform, world_data *World);
void RenderAllEntities(world_data *World);

#endif //_ENTITY_H

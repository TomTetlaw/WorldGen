in vec4 Position;
in vec2 TexCoord;
in vec4 Colour;

out vec4 FragColour;

uniform float TileSize;
uniform int WidthInTiles;
uniform int HeightInTiles;

uniform usamplerBuffer Texture0;

vec4 rgba(float r, float g, float b, float a) {
	return vec4(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

vec4 UnpackColour(uint ColourCode) {
	switch(ColourCode) {
		case 1u: return rgba(255.0, 255.0, 255.0, 255.0);
		case 2u: return rgba(9.0, 150.0, 181.0, 255.0);
		case 3u: return rgba(24.0, 110.0, 0, 255.0);
		case 4u: return rgba(159.0, 250.0, 2.0, 255.0);
		case 5u: return rgba(225.0, 250.0, 2.0, 255.0);
		case 6u: return rgba(250.0, 167.0, 2.0, 255.0);
		case 7u: return rgba(0.0, 0.0, 255.0, 255.0);
	}
	return vec4(1.0, (ColourCode*10.0)/255.0, 1.0 - ColourCode, 1.0);
}

void main() {
	vec2 TotalSize = vec2(WidthInTiles, HeightInTiles) * TileSize;
	vec2 LocalPos = TexCoord * TotalSize;
	int x = int(LocalPos.x / TileSize);
	int y = int(LocalPos.y / TileSize);
	
	int Index = x + y*WidthInTiles;
	uvec4 Sample = texelFetch(Texture0, Index);
	FragColour = UnpackColour(Sample.r);
}

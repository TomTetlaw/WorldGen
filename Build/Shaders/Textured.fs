in vec4 Position;
in vec2 TexCoord;
in vec4 Colour;

out vec4 FragColour;

uniform sampler2D Texture0;

void main() {
	FragColour = Colour * texture(Texture0, TexCoord);
}
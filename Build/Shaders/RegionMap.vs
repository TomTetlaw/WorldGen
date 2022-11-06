layout(location=0) in vec2 VertexPosition;
layout(location=1) in vec2 VertexTexCoord;
layout(location=2) in vec4 VertexColour;

uniform mat4 OrthoMatrix;

out vec4 Position;
out vec2 TexCoord;
out vec4 Colour;

void main() {
	Position = OrthoMatrix * vec4(VertexPosition, 0.0, 1.0);
	TexCoord = VertexTexCoord;
	Colour = VertexColour;
	
	gl_Position = Position;
}
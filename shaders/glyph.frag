in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;
uniform float opacity;

void main() {
    float a = texture(text, TexCoords).r;
    color = vec4(textColor.rgb, a * textColor.a * opacity);
}
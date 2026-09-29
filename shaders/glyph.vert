layout (location = 0) in vec2 aCorner;
out vec2 TexCoords;

uniform mat4 projection;
uniform vec4 rect;

void main()
{
    vec2 pos = rect.xy + aCorner * rect.zw;
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    TexCoords = vec2(aCorner.x, 1.0 - aCorner.y);
}
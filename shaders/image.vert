layout (location = 0) in vec2 aCorner;
out vec2 TexCoords;

uniform mat4 projection;
uniform vec4 rect;
// The part of the image to show, in 0..1 texture space: xy is its top-left, zw its size.
uniform vec4 uvRect;

void main()
{
    vec2 pos = rect.xy + aCorner * rect.zw;
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    // Corner y = 0 is the quad's top, and stb loads row 0 as the image's top.
    TexCoords = uvRect.xy + aCorner * uvRect.zw;
}

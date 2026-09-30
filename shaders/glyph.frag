in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform sampler2D clipMask;
uniform vec4 maskRect;
uniform float viewportHeight;
uniform vec4 textColor;
uniform float opacity;

float clipCoverage() {
    vec2 pix = vec2(gl_FragCoord.x, viewportHeight - gl_FragCoord.y);
    vec2 p = pix - maskRect.xy;
    if (any(lessThan(p, vec2(0.0))) || any(greaterThanEqual(p, maskRect.zw))) {
        return 0.0f;
    }
    return texture(clipMask, p / maskRect.zw).r;
}

void main() {
    float a = texture(text, TexCoords).r;
    color = vec4(textColor.rgb, a * textColor.a * opacity * clipCoverage());
}
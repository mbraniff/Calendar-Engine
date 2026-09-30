in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D coverage;
uniform sampler2D clipMask;
uniform vec4 maskRect;
uniform float viewportHeight;
uniform vec4 color;
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
    float a = texture(coverage, TexCoords).r;
    FragColor = vec4(color.rgb, a * color.a * opacity * clipCoverage());
}
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform sampler2D clipMask;
uniform vec4 maskRect;
uniform float viewportHeight;
uniform vec4 color;
uniform float opacity;
// 0 draws the image's own colors tinted by color; 1 draws its shape (alpha) in color alone.
uniform float templateMode;

float clipCoverage() {
    vec2 pix = vec2(gl_FragCoord.x, viewportHeight - gl_FragCoord.y);
    vec2 p = pix - maskRect.xy;
    if (any(lessThan(p, vec2(0.0))) || any(greaterThanEqual(p, maskRect.zw))) {
        return 0.0f;
    }
    return texture(clipMask, p / maskRect.zw).r;
}

void main() {
    vec4 c = texture(image, TexCoords);
    vec3 base = mix(c.rgb, vec3(1.0), templateMode);
    FragColor = vec4(base * color.rgb, c.a * color.a * opacity * clipCoverage());
}

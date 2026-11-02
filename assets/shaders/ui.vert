#version 330 core

layout (location = 0) in vec4 aPosSpriteIndex;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

uniform int uSpritesPerRow;
float uSpriteSize = 1.f/uSpritesPerRow;
uniform int spriteUnit;
uniform mat4 transform;

out vec2 texCoord;
out vec4 color;

void main()
{
    float spriteIndex = aPosSpriteIndex.w;
    vec2 spriteLoc = spriteIndex >= 0 ? vec2(int(spriteIndex) % uSpritesPerRow, int(spriteIndex) / uSpritesPerRow) : vec2(1, 1);
    texCoord = (vec2(spriteLoc.x, spriteLoc.y) + vec2(1, -1) * aUV/float(spriteUnit) ) * uSpriteSize;
    color = aColor;
    gl_Position = transform * vec4(aPosSpriteIndex.xyz, 1);
}
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D aTexture;
//uniform sampler2D aDepth;

float near = 0.1;
float far = 800.f;
void main()
{
 //float depth = texture(aDepth, TexCoord).r;  // Sample the depth texture
 //   float linearDepth = (2.0 * near * far) / (far + near - depth * (far - near));
 //   linearDepth = 1 - (linearDepth / far); // Normalize to 0-1 range
 //   FragColor = vec4(linearDepth *texture(aTexture, TexCoord)); // Display as grayscale
//
 //   if(depth == 1){FragColor = texture(aTexture, TexCoord);}
 //FragColor = (depth); //* texture(aTexture, TexCoord);
 //FragColor = vec4(TexCoord,1,1);

 //float gamma = 2.2f;
 FragColor = texture(aTexture, TexCoord);
 //FragColor.rgb = pow(FragColor.rgb, vec3(1.f/gamma));
 //FragColor = vec4(0, TexCoord, 1);
}

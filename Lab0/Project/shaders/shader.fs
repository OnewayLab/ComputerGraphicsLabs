#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 BoxTexCoord;
in vec2 FaceTexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, BoxTexCoord), texture(texture2, FaceTexCoord), 0.2);
}
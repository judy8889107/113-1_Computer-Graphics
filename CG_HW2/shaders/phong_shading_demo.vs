#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

// Transformation matrix.
uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;

// Data pass to fragment shader.
// --------------------------------------------------------
// TODO:Add your data for interpolation.
out vec3 iPosWorld;
out vec3 iNormalWorld;
// --------------------------------------------------------

void main()
{
    // --------------------------------------------------------
    // TODO:Add your implementation.
    gl_Position = MVP * vec4(Position, 1.0);
    
    vec4 worldPos = worldMatrix * vec4(Position, 1.0);

    iPosWorld = worldPos.xyz / worldPos.w;
    iNormalWorld = (normalMatrix * vec4(Normal, 0.0)).xyz;
    // --------------------------------------------------------
}
#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

// Transformation matrix.
uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;
// --------------------------------------------------------
// Add more uniform variables if needed.
// --------------------------------------------------------

// Data pass to fragment shader.
// --------------------------------------------------------
// Add your data for interpolation.
// --------------------------------------------------------

void main()
{
    // --------------------------------------------------------
    // Add your implementation.
    // --------------------------------------------------------
}
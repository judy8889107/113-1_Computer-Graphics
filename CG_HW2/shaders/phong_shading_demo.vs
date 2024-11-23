#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

// Transformation matrix.
uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;

// Data pass to fragment shader.
// --------------------------------------------------------
// Add your data for interpolation.
/* vertex shader回傳的世界座標和轉換到世界座標的法向量，打光於fragment shader中計算 */
out vec3 iPosWorld;
out vec3 iNormalWorld;
// --------------------------------------------------------
/* Ref:Lighting and Shading (part II) p.33 */
void main()
{
    // --------------------------------------------------------
    gl_Position = MVP * vec4(Position, 1.0);

    // --------------------------------------------------------
    vec4 worldPos = worldMatrix * vec4(Position, 1.0);

    iPosWorld = worldPos.xyz / worldPos.w; // 取得NDC座標
    iNormalWorld = (normalMatrix * vec4(Normal, 0.0)).xyz; // 將vertex法向量乘以NormalMatrix，從objSpace轉換到世界座標
    
}
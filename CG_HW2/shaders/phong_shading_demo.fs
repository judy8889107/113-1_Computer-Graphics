#version 330 core

// Data from vertex shader.
// --------------------------------------------------------
// TODO:Add your data for interpolation.
in vec3 iPosWorld;
in vec3 iNormalWorld;
// --------------------------------------------------------

// --------------------------------------------------------
// TODO:Add your uniform variables.
// Material properties.
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
// Light data.
uniform vec3 cameraPos;
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
// spot light (name in shaderprog.cpp)
uniform vec3 spotlightLightDir;
uniform vec3 spotlightLightPos;
uniform vec3 spotlightLightIntensity;
uniform float spotlightCutoffStartInDegree;
uniform float spotlightTotalWidthInDegree;

// --------------------------------------------------------
out vec4 FragColor;
/*Ref: gouraud_shading_demo.vs */

// Diffuse
vec3 Diffuse(vec3 Kd, vec3 I, vec3 N, vec3 lightDir)
{
    return Kd * I * max(0, dot(N, lightDir));
}
// Specular(Blinn-Phong)
vec3 Specular(vec3 Ks, vec3 I, float Ns, vec3 N, vec3 lightDir, vec3 vE)
{
    vec3 vH = normalize(vE + lightDir); // compute half vector -Lighting and Shading (Part I) page.11
    return Ks * I * pow(max(0, dot(N, vH)), Ns);
}
// Ambient
vec3 Ambient(vec3 Ka, vec3 I)
{
    return Ka * I;
}

// Spot Light
vec3 SpotLight(vec3 Ks, vec3 I, float Ns, vec3 N, vec3 lightDir, vec3 vE, float cutoffStartInDegree, float totalWidthInDegree)
{
    vec3 fsNormalWorld = normalize(iNormalWorld);
    vec3 fsPosition = normalize(iPosWorld);
    // 計算光源方向
    vec3 fsLightDir = normalize(spotlightLightPos - iPosWorld);

    // 計算衰減
    float distSurfaceToLight = distance(spotlightLightPos, iPosWorld);
    float attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);

    // 計算光強度
    vec3 radiance = spotlightLightIntensity * attenuation;

    // 計算聚光燈的角度
    float theta = dot(fsLightDir, normalize(-spotlightLightDir)); // 角度
    float cutoff = cos(radians(cutoffStartInDegree));             // 開始的截止角度
    float intensityFactor = smoothstep(cos(radians(totalWidthInDegree)), cos(radians(cutoffStartInDegree)), theta); // 總寬度


    // 計算漫反射和鏡面反射
    // vec3 radiance = spotlightLightIntensity * intensityFactor;
    vec3 fsViewDir = normalize(cameraPos - fsPosition);
    vec3 diffuse = Diffuse(Kd, radiance, fsNormalWorld, fsLightDir);
    vec3 specular = Specular(Ks, radiance, Ns, fsNormalWorld, fsLightDir, fsViewDir);

    return (diffuse + specular) * intensityFactor; // 返回最終的光照顏色
}

void main()
{
    // --------------------------------------------------------
    // TODO:Add your implementation.
    // Shared parameters name.
    vec3 fsNormalWorld = normalize(iNormalWorld);
    vec3 fsPosition = normalize(iPosWorld);
    vec3 diffuse, specular, ambient, fsLightDir, fsViewDir, radiance;
    // --------------------------------------------------------
    // Ambient.
    ambient = Ambient(Ka, ambientLight);
    // Directional light.
    fsLightDir = normalize(-dirLightDir); // 入射方向
    // Diffuse.
    diffuse = Diffuse(Kd, dirLightRadiance, fsNormalWorld, fsLightDir);
    // Specular.
    fsViewDir = normalize(cameraPos - fsPosition);
    specular = Specular(Ks, dirLightRadiance, Ns, fsNormalWorld, fsLightDir, fsViewDir);
    vec3 dirLight = diffuse + specular;
    // --------------------------------------------------------
    // Point light. (Point Light Postion is in world space)
    fsLightDir = normalize(pointLightPos - fsPosition);
    float distSurfaceToLight = distance(pointLightPos, fsPosition);
    float attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    radiance = pointLightIntensity * attenuation;
    // // Diffuse.
    diffuse = Diffuse(Kd, radiance, fsNormalWorld, fsLightDir);
    // // Specular.
    fsViewDir = normalize(cameraPos - fsPosition);
    specular = Specular(Ks, radiance, Ns, fsNormalWorld, fsLightDir, fsViewDir);
    vec3 pointLight = diffuse + specular;
    // -------------------------------------------------------------
    // Spot light.
    fsViewDir = normalize(cameraPos - spotlightLightPos);
    vec3 spotlight = SpotLight(Ks, spotlightLightIntensity, Ns, fsNormalWorld, spotlightLightDir, fsViewDir, spotlightCutoffStartInDegree, spotlightTotalWidthInDegree);

    // --------------------------------------------------------
    vec3 lighting = dirLight + pointLight + spotlight + ambient;
    FragColor = vec4(lighting, 1.0);
    // --------------------------------------------------------
}

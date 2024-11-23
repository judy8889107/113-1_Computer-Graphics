#version 330 core

// Data from vertex shader.
// --------------------------------------------------------
// Add your data for interpolation.
// Material properties.
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
// Light data.
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightIntensity;
uniform float spotLightCutoffStartInDegree;
uniform float spotLightTotalWidthInDegree;
uniform vec3 cameraPos;

// 從vertex shader傳過來的資料
in vec3 iPosWorld;
in vec3 iNormalWorld;

// --------------------------------------------------------

// --------------------------------------------------------
// Add your uniform variables.
// --------------------------------------------------------

// fragment shader 傳出的資料
out vec4 FragColor;

/* Ref: gouraud_shading_demo.vs & Lighting and Shading (part I) 的公式 */

/* 計算ambient light */
vec3 Ambient(vec3 Ka, vec3 Ia)
{
    return Ka * Ia;
}

/* 計算Diffuse light(N=face normal, vL=light direction) */
vec3 Diffuse(vec3 Kd, vec3 I, vec3 N, vec3 vL)
{
    return Kd * I * max(0.0, dot(N, vL));
}
// /* 計算Specular light(N=face normal, vL=light direction, vH=half vector=(vL+vE)/2) */
vec3 Specular(vec3 Ks, vec3 I, vec3 N, vec3 vE, vec3 vL, float n)
{

    vec3 vH = normalize(vL + vE);
    return Ks * I * pow(max(dot(N, vH), 0.0), n);
}

/* 計算Spot light(N=face normal, vL=light direction, vH=half vector=(vL+vE)/2) */
vec3 Spotlight(vec3 Kd, vec3 Ks, float Ns, vec3 V, vec3 N, vec3 spotPosition, vec3 spotDirection, vec3 intensity, float cutoffStartInDegree, float totalWidthInDegree)
{
    // 計算光源方向
    vec3 vsLightDir = normalize(spotPosition - iPosWorld);
    // 一樣計算距離和衰減
    float dist = distance(spotPosition, iPosWorld);
    float attenuation = 1.0f / (dist * dist);

    // 計算光線和聚光燈夾角
    float theta = dot(vsLightDir, normalize(-spotDirection));
    // 計算聚光燈有效範圍
    float epsilon = cos(radians(cutoffStartInDegree)) - cos(radians(totalWidthInDegree));
    // 計算強度因子
    float intensityFactor = smoothstep(cos(radians(totalWidthInDegree)), cos(radians(cutoffStartInDegree)), theta);

    vec3 spotlightDiffuse = Diffuse(Kd, intensity * attenuation, N, vsLightDir);
    vec3 spotlightSpecular = Specular(Ks, intensity * attenuation, N, V, vsLightDir, Ns);

    return (spotlightDiffuse + spotlightSpecular) * intensityFactor;
}

void main()
{ // 於世界座標系
    // --------------------------------------------------------
    // Add your implementation.
    // 標準化世界座標的法向量
    vec3 N = normalize(iNormalWorld);
    // view direction (vE)
    vec3 vE = normalize(cameraPos - iPosWorld);
    // light direction (vL)
    vec3 vL = normalize(-dirLightDir);

    // Ambient light
    vec3 ambient = Ambient(Ka, ambientLight);

    // Directional light.
    vec3 diffuse = Diffuse(Kd, dirLightRadiance, N, vL);
    vec3 specular = Specular(Ks, dirLightRadiance, N, vE, vL, Ns);
    vec3 dirLight = diffuse + specular;

    // Point light.(Ref: gouraud_shading_demo.vs)
    // pointLightPos in world space, 計算light direction
    vec3 vsLightDir = normalize(pointLightPos - iPosWorld);
    float distSurfaceToLight = distance(pointLightPos, iPosWorld);
    float attenuation = 1.0f / (distSurfaceToLight * distSurfaceToLight);
    vec3 radiance = pointLightIntensity * attenuation; // 光強度*距離衰減
    vec3 pointDiffuse = Diffuse(Kd, radiance, N, vsLightDir);
    vec3 pointSpecular = Specular(Ks, radiance, N, vE, vsLightDir, Ns);
    vec3 pointLight = pointDiffuse + pointSpecular;

    // // Spot light.
    vec3 spotlight = Spotlight(Kd, Ks, Ns, vE, N, spotLightPosition, spotLightDirection, spotLightIntensity, spotLightCutoffStartInDegree, spotLightTotalWidthInDegree);

    // sum light
    vec3 lighting = ambient + dirLight + pointLight + spotlight;
    FragColor = vec4(lighting, 1.0);
    // --------------------------------------------------------
}

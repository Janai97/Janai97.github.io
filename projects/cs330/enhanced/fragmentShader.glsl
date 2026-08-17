#version 330 core
out vec4 fragmentColor;

in vec3 fragmentPosition;
in vec3 fragmentVertexNormal;
in vec2 fragmentTextureCoordinate;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
}; 

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool bActive;
};

struct PointLight {
    vec3 position;
    vec3 ambient; // Keeps structure intact, but we won't stack this 6x
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    bool bActive;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
    bool bActive;
};

#define TOTAL_POINT_LIGHTS 6

uniform bool bUseTexture = false;
uniform bool bUseLighting = false;
uniform vec4 objectColor = vec4(1.0f);
uniform vec3 viewPosition;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[TOTAL_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform sampler2D objectTexture;
uniform vec2 UVscale = vec2(1.0f, 1.0f);

// Function prototypes updated to take a base ambient control flag
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    if(bUseLighting == true)
    {
        vec3 phongResult = vec3(0.0f);
        vec3 norm = normalize(fragmentVertexNormal);
        vec3 viewDir = normalize(viewPosition - fragmentPosition);
        
        // Use the texture coordinates with UV scale applied
        vec2 mappedUV = fragmentTextureCoordinate * UVscale;
        vec3 baseColor = bUseTexture ? vec3(texture(objectTexture, mappedUV)) : vec3(objectColor);

        // Phase 1: Directional Light provides the baseline global ambient + its own diffuse/specular
        if(directionalLight.bActive == true)
        {
            phongResult += CalcDirectionalLight(directionalLight, norm, viewDir);
        }
        else
        {
            // Fallback baseline ambient if directional light is missing/disabled so scene isn't pure pitch black
            phongResult += vec3(0.05f) * baseColor;
        }

        // Phase 2: Point Lights add ONLY localized diffuse and specular (preventing 6x ambient stacking)
        for(int i = 0; i < TOTAL_POINT_LIGHTS; i++)
        {
            if(pointLights[i].bActive == true)
            {
                phongResult += CalcPointLight(pointLights[i], norm, fragmentPosition, viewDir);   
            }
        } 

        // Phase 3: Spot Light
        if(spotLight.bActive == true)
        {
            phongResult += CalcSpotLight(spotLight, norm, fragmentPosition, viewDir);    
        }
    
        if(bUseTexture == true)
        {
            fragmentColor = vec4(phongResult, texture(objectTexture, mappedUV).a);
        }
        else
        {
            fragmentColor = vec4(phongResult, objectColor.a);
        }
    }
    else
    {
        if(bUseTexture == true)
        {
            fragmentColor = texture(objectTexture, fragmentTextureCoordinate * UVscale);
        }
        else
        {
            fragmentColor = objectColor;
        }
    }
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec2 mappedUV = fragmentTextureCoordinate * UVscale;
    vec3 baseColor = bUseTexture ? vec3(texture(objectTexture, mappedUV)) : vec3(objectColor);
    
    vec3 lightDirection = normalize(-light.direction);
    
    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 1.0));

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * material.diffuseColor * baseColor;
    vec3 specular = light.specular * spec * material.specularColor; // Removed texture multiply from specular highlights
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec2 mappedUV = fragmentTextureCoordinate * UVscale;
    vec3 baseColor = bUseTexture ? vec3(texture(objectTexture, mappedUV)) : vec3(objectColor);
    
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 1.0));
   
    // Attenuation calculation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Notice we skip standard ambient calculation here to prevent compounding brightness bugs
    vec3 diffuse = light.diffuse * diff * material.diffuseColor * baseColor;
    vec3 specular = light.specular * spec * material.specularColor;
    
    return (diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec2 mappedUV = fragmentTextureCoordinate * UVscale;
    vec3 baseColor = bUseTexture ? vec3(texture(objectTexture, mappedUV)) : vec3(objectColor);
    
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 1.0));
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Spotlight intensity cone boundaries
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 diffuse = light.diffuse * diff * material.diffuseColor * baseColor;
    vec3 specular = light.specular * spec * material.specularColor;
    
    return (diffuse + specular) * attenuation * intensity;
}
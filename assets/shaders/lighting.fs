#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

#define     MAX_LIGHTS              30
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse*fragColor;
    vec3 baseColor = texelColor.rgb*tint.rgb;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);
            float attenuation = 1.0;

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                vec3 toLight = lights[i].position - fragPosition;
                float dist = length(toLight);
                light = normalize(toLight);
                attenuation = 1.0/(1.0 + 0.09*dist + 0.032*dist*dist);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL*attenuation;

            float specCo = 0.0;
            if (NdotL > 0.0)
                specCo = pow(max(0.0, dot(viewD, reflect(-light, normal))), 24.0);

            specular += lights[i].color.rgb*specCo*0.22*attenuation;
        }
    }

    vec3 ambientColor = ambient.rgb*baseColor;
    vec3 diffuseColor = lightDot*baseColor;
    vec3 litColor = ambientColor + diffuseColor + specular;

    finalColor = vec4(litColor, texelColor.a*tint.a);

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}

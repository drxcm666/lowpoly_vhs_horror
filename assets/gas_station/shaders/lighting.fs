#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 albedo = texelColor.rgb;

    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 viewDir = normalize(viewPos - fragPosition);

    float distance = length(lightPos - fragPosition);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

    // Warm night lamp: soft ambient + diffuse + moderate specular.
    vec3 ambient = 0.06 * lightColor;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.25 * spec * lightColor;

    float lightIntensity = 7.5;
    vec3 lighting = ambient + ((diffuse + specular) * attenuation * lightIntensity);
    vec3 litColor = albedo * lighting;

    // Basic tonemapping + gamma correction for less harsh highlights.
    litColor = litColor / (litColor + vec3(1.0));
    litColor = pow(litColor, vec3(1.0 / 2.2));

    finalColor = vec4(litColor, texelColor.a);
}
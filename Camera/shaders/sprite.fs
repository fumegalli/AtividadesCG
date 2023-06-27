#version 450

in vec3 finalColor;
in vec3 scaledNormal;
in vec2 textureCoord;
in vec3 fragmentPosition;

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;

uniform vec3 cameraPos;
uniform sampler2D tex_buffer;

out vec4 color;

void main()
{
	vec3 ambient = ka * lightColor;
	
	vec3 N = normalize(scaledNormal);
	vec3 L = normalize(lightPosition - fragmentPosition);
	float diff = max(dot(N,L),0.0);
	vec3 diffuse = kd * diff * lightColor;

	vec3 V = normalize(cameraPos - fragmentPosition);
	vec3 R = normalize(reflect(-L,N));
	float spec = max(dot(R,V),0.0);
	spec = pow(spec, q);
	vec3 specular = ks * spec * lightColor;

	vec3 texColor = texture(tex_buffer, textureCoord).xyz;
	vec3 result = (ambient + diffuse) * texColor + specular;

	color = vec4(result, 1.0f);
}

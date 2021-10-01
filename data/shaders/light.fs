
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec3 u_light_pos;
uniform vec3 u_light_color;
uniform vec3 u_camera_pos;
uniform vec3 u_light_color2;


uniform vec3 u_specular;
uniform vec3 u_diffuse;
uniform float u_shininess;

void main()
{
	vec4 color = texture2D( u_texture, v_uv );
	vec3 Ia = u_light_color;
	vec3 Id = u_light_color;
	vec3 Is = u_light_color;

	vec3 ambient_light = color.xyz * Ia;

	vec3 N = normalize(v_normal); // also we can use normal texture in the future
	vec3 L = normalize(v_world_position - u_light_pos); 
	vec3 V = normalize(v_world_position - u_camera_pos); 
	vec3 R = reflect(-L,N);
	//vec3 H = normalize(V + L);
	
	vec3 light_intensity = vec3(0,0,0);

	vec3 diffuse_light = u_diffuse*clamp(dot(L,N), 0.0f, 1.0f)*Id; 
	vec3 specular_light = u_specular*pow(clamp(dot(R,V),0.0f, 1.0f),u_shininess)*Is;

	light_intensity = ambient_light + diffuse_light + specular_light;

	gl_FragColor = vec4( clamp(light_intensity,0.0,1.0) , 1.0f);
}


varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec3 u_camera_pos;

uniform vec3 u_light_pos;
uniform vec3 u_Ia, u_Id, u_Is; //Ambient, diffuse, specular 

uniform vec3 u_specular;
uniform vec3 u_diffuse;
uniform float u_shininess;

void main()
{
	vec4 color = texture2D( u_texture, v_uv );
	
	vec3 ambient_light = color.xyz * u_Ia;
	
	vec3 N = normalize(v_normal); // also we can use normal texture in the future
	vec3 L = normalize(u_light_pos - v_world_position  ); // HE CAMBIADO EL ORDEN DE ESTO, YA QUE SE ILUMINABA AL REVÉS
	vec3 V = normalize(u_camera_pos - v_world_position ); 
	vec3 R = reflect(-L,N);
	//vec3 H = normalize(V + L);
	
	vec3 light_intensity = vec3(0,0,0);

	vec3 diffuse_light = u_diffuse* clamp(dot(L,N), 0.0f, 1.0f) * u_Id; 
	vec3 specular_light = u_specular * pow(clamp(dot(R,V),0.0f, 1.0f),u_shininess) * u_Is;

	light_intensity = ambient_light + diffuse_light + specular_light;

	vec3 test = vec3(0.0, 0.0, 1.0);
	gl_FragColor = vec4( light_intensity, 1.0);
}

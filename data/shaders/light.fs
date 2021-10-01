
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec3 u_light_pos;
uniform vec3 u_light_color;
uniform vec3 u_camera_pos;

uniform vec3 u_specular;
uniform vec3 u_diffuse;

void main()
{
	vec4 color = texture2D( u_texture, v_uv );
	vec3 Ia = u_light_color;
	vec3 Id = u_light_color;
	vec3 Is = u_light_color;

	vec3 ambient_light = color.xyz * Ia;


	vec3 light_intensity = vec3(0,0,0);




	gl_FragColor = color;
}

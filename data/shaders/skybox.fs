
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform samplerCube u_texture_cube;
uniform vec3 u_camera_position;

void main()
{
	vec3 direction = normalize(u_camera_position - v_world_position);
	vec4 color = textureCube( u_texture_cube, direction );
	
	gl_FragColor = color;
}

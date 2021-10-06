
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform samplerCube u_texture;
uniform vec3 u_camera_position;
uniform mat4 u_model;

void main()
{
	vec3 normal_view = (u_model * vec4( v_normal.xyz, 0.0)).xyz;
	vec3 N = normalize(normal_view);
	vec3 position_view = (u_model * vec4( v_world_position.xyz, 1.0)).xyz;
	vec3 direction = normalize(position_view - u_camera_position);
	vec3 refleced_dir = reflect(direction,N);
	vec4 color = textureCube( u_texture, direction );
	// gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
	gl_FragColor = color;
}

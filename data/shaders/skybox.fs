
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
<<<<<<< Updated upstream
	vec3 position_view = (u_model * vec4( v_world_position.xyz, 1.0)).xyz;
	vec3 direction = normalize(position_view - u_camera_position);
	vec4 color = textureCube( u_texture, direction );
	// gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
=======

	//vec3 direction = normalize(u_camera_position - v_world_position);
	vec3 direction = normalize( v_world_position - u_camera_position );

	vec4 color = textureCube( u_texture, direction );

>>>>>>> Stashed changes
	gl_FragColor = color;
}

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
	//In this case, we compute the vector R which corresponds to the reflection of vector V (vector toward the eye)
	//with respect to the normal, where this vector V is computed from the corresponding point in the scene to the cameraPos, 
	//since we want to reflect our environment, not the scene objects.
	vec3 N = normalize(v_normal);
	vec3 V = normalize(u_camera_position - v_world_position );
	vec3 refleced_dir = reflect(-V, N);
	vec4 color = textureCube( u_texture, refleced_dir );
	gl_FragColor = color;
}

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

//Utils uniforms parameters
//uniform samplerCube u_texture;
uniform samplerCube u_panorama_tex;
uniform samplerCube u_snow_tex;

uniform vec3 u_camera_position;
uniform float u_output;

void main()
{
	//We want to find the direction of vector from cameraPosition the corresponding point in the scene
	//And using the function textureCube, we get the right side of the cube
	vec3 direction = normalize( v_world_position - u_camera_position );
	vec4 color = vec4(0.0);
	if(u_output == 0.0)
 		color = textureCube( u_panorama_tex, direction );
	else if(u_output == 1.0)
		color = textureCube(u_snow_tex, direction);
	
	gl_FragColor = color;
}


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

// vec3 toneMapUncharted2Impl(vec3 color)
// {
//     const float A = 0.15;
//     const float B = 0.50;
//     const float C = 0.10;
//     const float D = 0.20;
//     const float E = 0.02;
//     const float F = 0.30;
//     return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
// }

// vec3 toneMapUncharted(vec3 color)
// {
//     const float W = 11.2;
//     color = toneMapUncharted2Impl(color * 2.0);
//     vec3 whiteScale = 1.0 / toneMapUncharted2Impl(vec3(W));
//     return color * whiteScale;
// }


void main()
{
	//We want to find the direction of vector from cameraPosition the corresponding point in the scene
	//And using the function textureCube, we get the right side of the cube
	vec3 direction = normalize( v_world_position - u_camera_position );
	vec4 color = vec4(0.0);
	if(u_output == 0.0){
 		color = textureCube( u_panorama_tex, direction );
		//color = vec4(toneMapUncharted(color.xyz), 1.0);
	}
	else if(u_output == 1.0)
		color = textureCube(u_snow_tex, direction);
	
	gl_FragColor = color;
}

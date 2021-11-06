#define MAX_ITERATIONS 10

varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
// varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

uniform vec3 u_camera_position;
uniform mat4 u_model;
uniform mat4 u_inverse_model;
uniform sampler3D u_texture;
uniform vec4 u_color;
uniform float u_length_step;

void main(){
    // 1. Ray setup

    vec3 camera_pos_local = (u_inverse_model * vec4(u_camera_position, 1.0f)).xyz; // En teoria si es un punto ponemos 1 y si es vector 0, pero estaria bien preguntarlo
	vec3 ray_dir = normalize(v_position - camera_pos_local);
	vec3 first_sample = v_position;
	vec3 curr_sample_point = first_sample;
	vec3 step_vector = (first_sample + ray_dir) * u_length_step;

	vec4 final_color = vec4(0.0f);
	vec3 curr2tex_coord = vec3(0.0f);

    // Ray loop
    for(int=0; i<MAX_ITERATIONS; i++){
        // 2. Volume sampling
		// Convert to texture coord
		curr2tex_coord = (curr_sample_point + 1.0f)/2; // si no funciona poner vec3
        float d = texture3D(u_texture, curr2tex_coord).x;

        // 3. Classification
        vec4 sample_color = vec4(d,d,d,d);

        // 4. Composition
		sample_color.rgb *= sample_color.a;
        final_color += u_length_step * (1.0 - final_color.a) * sample_color;

        // 5. Next sample
        // sample_vector = (sample_vector + ray_dir) * u_length_step;

        curr_sample_point = curr_sample_point + step_vector;

        // 6. Early termination
        if (final_color.a >= 1.0f) {
			final_color.a = 1.0f;
			break;
		}
    }

    //7. Final color
    //...
    
    gl_FragColor = final_color;
}

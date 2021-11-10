#define MAX_ITERATIONS 100

varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
// varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

uniform vec3 u_camera_position;
uniform mat4 u_model;
uniform mat4 u_inverse_model;
uniform sampler3D u_texture;
// uniform vec4 u_color;
uniform float u_length_step; //ray step
uniform float u_brightness;
void main(){
    // 1. Ray setup

    vec3 camera_l_pos = (u_inverse_model * vec4(u_camera_position, 1.0)).xyz; 
    vec3 ray_dir = normalize( v_position - camera_l_pos);
	vec3 sample_pos = v_position; //initialiced as entry point to the volume
	vec4 final_color = vec4(0.0f);

    vec3 step_vector = u_length_step*ray_dir;
	// vec3 curr_sample_point = first_sample;
	// vec3 step_vector = (first_sample + ray_dir) * u_length_step;
	// vec3 curr2tex_coord = vec3(0.0f);
    // vec4 sample_color = vec4(0.0f);
    float d = 0.0f;
    vec3 uv_3D = vec3(0.0f);
    vec4 sample_color = vec4(0.0f);
    vec3 curr_sample_pos = vec3(0.0f);
    // float limit_l_min = -1.0f;
    // float limit_l_max = 1.0f;

    // Ray loop
    for(int i=0; i<MAX_ITERATIONS; i++){

        // 2. Volume sampling

        uv_3D = (sample_pos + 1.0f)/2.0f;
        d = texture3D(u_texture, uv_3D).x;

        // 3. Classification
        sample_color = vec4(d,d,d,d);

        // 4. Composition
		sample_color.rgb *= sample_color.a; //CREO Q PONIENDO ESTO SE VE PEOR
        final_color += u_length_step * (1.0 - final_color.a) * sample_color;

        // 5. Next sample
        // sample_vector = (sample_vector + ray_dir) * u_length_step;
        
        curr_sample_pos = sample_pos;
        sample_pos = curr_sample_pos + step_vector;

        // curr_sample_point = curr_sample_point + step_vector;

        // 6. Early termination
        if( sample_pos.x > (1.0f) && sample_pos.y > (1.0f) && sample_pos.z > (1.0f) ) {
            break;
        }
        if( sample_pos.x < -1.0f && sample_pos.y < -1.0f && sample_pos.z < -1.0f ) {
            break;
        }
        if(  final_color.a >= 1.0f ){
            final_color.a = 1.0f;
            break;
        }
       
    }
    float threshold = 0.00f;
    if (final_color.x <= threshold ||final_color.y <= threshold||final_color.z <= threshold){
        // final_color = vec4(1,0,0,0);
        discard;
    }

    //7. Final color
    //...   
    // gl_FragColor = vec4(d,d,d, 1.0f);
    gl_FragColor = final_color * u_brightness;
}

#define MAX_ITERATIONS 1000

varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
// varying vec2 v_uv; // texture coordinates
varying vec4 v_color;


uniform mat4 u_model;
uniform mat4 u_inverse_model;
uniform vec3 u_camera_position;

//Textures uniforms
uniform sampler3D u_texture;
uniform sampler2D u_noise_texture;
uniform sampler2D u_tf_mapping_texture;
uniform vec4 u_color;

uniform float u_length_step; //ray step
uniform float u_brightness;
uniform vec4 u_plane_abcd;
uniform float u_iso_threshold;
// uniform float u_plane_a;
// uniform float u_plane_b;
// uniform float u_plane_c;
// uniform float u_plane_d;

void main(){
    
    float texture_width = 128.0f;
    vec2 uv_screen =  gl_FragCoord.xy / texture_width; 
    float offset = texture2D(u_noise_texture, uv_screen).x ;

    // 1. Ray setup
    vec4 camera_l_pos_temp = (u_inverse_model * vec4(u_camera_position, 1.0));
    vec3 camera_l_pos = camera_l_pos_temp.xyz/camera_l_pos_temp.w;
    vec3 ray_dir = normalize(v_position - camera_l_pos);
    vec3 step_vector = u_length_step*ray_dir;
    
    // vec3 step_offset = offset*step_vector;   
    vec3 step_offset = vec3(0.0f);
	
    vec3 sample_pos = v_position+step_offset; //initialiced as entry point to the volume
	vec4 final_color = vec4(0.0f);


    // Initialize values that are computed in the loop
    float d = 0.0f;
    vec3 uv_3D = vec3(0.0f);
    vec4 sample_color = vec4(0.0f);
    float plane = 0.0f;

    // Ray loop
    for(int i=0; i<MAX_ITERATIONS; i++){
       

        // 2. Volume sampling
        uv_3D = (sample_pos + 1.0f)*0.5f;
        d = texture3D(u_texture, uv_3D).x;

        // 3. Classification
        //vec3 tf_color = texture2D(u_tf_mapping_texture, vec2(d,1)).xyz;
        // Con tf
        //sample_color = vec4(tf_color.r, tf_color.g, tf_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric4

        // Classification basica
        // sample_color = vec4(u_color.r*tf_color.r,u_color.g*tf_color.g,u_color.b*tf_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric
        
        sample_color = vec4(u_color.r,u_color.r,u_color.r,d);//important that the d, 4ºcomponent. Para que funcione la volumetric
        
        // 4. Composition
		sample_color.rgb *= sample_color.a; 
        // final_color += u_length_step * (1.0 - final_color.a) * sample_color;
        // Visualizing isosurfaces
        // ME HE QUEDADO AQUI, SALE COSAS RARAS HAY Q MIRAR...
        if ( d > u_iso_threshold)
            final_color = (1.0 - final_color.a) * sample_color;

        // 5. Next sample

        sample_pos += step_vector;
        // plane = u_plane_abcd.x*sample_pos.x + u_plane_abcd.y*sample_pos.y + u_plane_abcd.z*sample_pos.z + u_plane_abcd.w;
        
        //volume clipping
        // if (plane > 0.0f) {
        //     final_color = vec4(0.0f);    
        //     //final_color += u_length_step * (1.0 - final_color.a) * sample_color;// not sure if is correct
        //     //final_color += sample_color.a > u_iso_threshold;
        // }

        // 6. Early termination
        //If is outside the auxiliar mesh
        if( sample_pos.x > 1.0f || sample_pos.y > 1.0f || sample_pos.z > 1.0f ) {
            break;
        }
        if( sample_pos.x < -1.0f || sample_pos.y < -1.0f || sample_pos.z < -1.0f ) {
            break;
        }
        //If the alpha component is larger than 1.0f
        if(  final_color.a >= 1.0f ){
            final_color.a = 1.0f;
            break;
        }
       
    }

    float threshold = 0.01f;
    if (final_color.w <= threshold){ // solo con w
        discard;
    }

    //7. Final color
    gl_FragColor = final_color * u_brightness;
    //gl_FragColor = vec4(offset);
}

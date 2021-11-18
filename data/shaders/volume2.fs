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
uniform float u_threshold_plane;
uniform float u_threshold_d_min;
uniform float u_threshold_d_max;

// TF generator variables
uniform vec4 u_density_limits;
uniform vec4 u_tf_fst_color;
uniform vec4 u_tf_snd_color;
uniform vec4 u_tf_trd_color;
uniform vec4 u_tf_frth_color;

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

    vec3 step_offset = 0.0f;
    #ifdef USE_JITTERING
        step_offset = offset*step_vector;
    #endif
    
	vec3 sample_pos = v_position+step_offset; //initialized as entry point to the volume
	vec4 final_color = vec4(0.0f);


    // Initialize values that are computed in the loop
    float d = 0.0f;
    vec3 uv_3D = vec3(0.0f);
    vec4 sample_color = vec4(0.0f);
    float plane_value = 0.0f;
    vec4 plane_abcd = (0.0f, 1.0f, 0.0f, 1.0f);

    // Ray loop
    for(int i=0; i<MAX_ITERATIONS; i++){
        plane_value = plane_abcd.x*sample_pos.x + plane_abcd.y*sample_pos.y + plane_abcd.z*sample_pos.z + plane_abcd.w;
        // plane_value += plane_abcd.y*sample_pos.y;

        // if (plane_value < u_threshold_plane)
        //     discard;

        // 2. Volume sampling
        uv_3D = (sample_pos + 1.0f)*0.5f;
        d = texture3D(u_texture, uv_3D).x;

        // 3. Classification
        // Classification basica
        // SEGURO QUE ESTO SE PUEDE HACER MÁS EFICIENTE PARA QUE NO SE HAGA SI EL TF ESTÁ ACTIVADO
        sample_color = vec4(d,d,d,d);//important that the d, 4ºcomponent. Para que funcione la volumetric

        // Con tf
        #ifdef USE_TF
            vec3 tf_color = texture2D(u_tf_mapping_texture, vec2(d,1)).xyz;
            sample_color = vec4(tf_color.r, tf_color.g, tf_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric4
            // sample_color = vec4(u_color.r*tf_color.r,u_color.g*tf_color.g,u_color.b*tf_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric
        #endif

        #ifdef USE_TF_DEBUG
            //Si estamos en el modo debug de la TF, queremos visualizar solo aquellos puntos que tengan una densidad menor a la marcada en el imgui
            if(d>u_threshold_d_max)
                discard;

            if(d<u_density_limits.x)
                sample_color = vec4(u_tf_fst_color.r,u_tf_fst_color.g,u_tf_fst_color.b,d);
            if(d>u_density_limits.x && d<u_density_limits.y)
                sample_color = vec4(u_tf_snd_color.r,u_tf_snd_color.g,u_tf_snd_color.b,d);
            if(d>u_density_limits.y && d<u_density_limits.z)
                sample_color = vec4(u_tf_trd_color.r,u_tf_trd_color.g,u_tf_trd_color.b,d);
            if(d>u_density_limits.z && d<u_density_limits.w)
                sample_color = vec4(u_tf_frth_color.r,u_tf_frth_color.g,u_tf_frth_color.b,d);

        #endif



        // 4. Composition
		sample_color.rgb *= sample_color.a; //CREO Q PONIENDO ESTO SE VE PEOR
        final_color += u_length_step * (1.0 - final_color.a) * sample_color;

        // 5. Next sample
        // if (i==1)
        //     sample_pos += offset;
        sample_pos += step_vector;

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
    // gl_FragColor = u_tf_fst_color;
}

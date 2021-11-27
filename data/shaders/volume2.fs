#define MAX_ITERATIONS 1000000000 //We define a large number of Iterations, since we have early termination

//---------------------UNIFORMS------------------------

varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
// varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

// Basics uniforms
uniform mat4 u_model;
uniform mat4 u_inverse_model;
uniform vec3 u_camera_position;
uniform vec4 u_color;

// Textures uniforms
uniform sampler3D u_texture;
uniform sampler2D u_noise_texture;
uniform sampler2D u_tf_mapping_texture;

// Utils uniforms
uniform float u_length_step; //ray step
uniform float u_brightness;
uniform vec4 u_plane_abcd;

// TF generator variables
uniform float u_max_density;
uniform vec4 u_density_limits;
uniform vec4 u_tf_fst_color;
uniform vec4 u_tf_snd_color;
uniform vec4 u_tf_trd_color;
uniform vec4 u_tf_frth_color;
uniform vec4 u_highlight;

// Thresholds
uniform float u_iso_threshold;
uniform float u_h_threshold;
uniform float u_threshold_plane;
uniform float u_discard_threshold;

// Boolean flags
uniform bool u_shade_flag;
uniform bool u_gradient_flag;
uniform bool u_phong_flag; 

// Light parameters
uniform vec3 u_light_pos;
uniform vec3 u_Ia, u_Id, u_Is; //Ambient, diffuse, specular
uniform float u_light_intensity;

// Material uniforms parameters
uniform vec3 u_diffuse;
uniform vec3 u_specular;
uniform float u_shininess;

//---------------------FUNCTIONS------------------------

vec3 compute_lightPhong(vec3 gradient, vec4 final_color){
    
    // Compute the light equation vectors
    vec3 L = normalize(u_light_pos - v_world_position);// local or world?
    vec3 N = (u_model * vec4( gradient , 0.0) ).xyz;  
    vec3 V = normalize(u_camera_position - v_world_position); 
    vec3 R = reflect(-L,N);

    //We initialize the variables to store differents parts of lights
    vec3 ambient_light = final_color.xyz * u_Ia;
    vec3 diffuse_light = u_diffuse * max(dot(L,N), 0.0f) * u_Id;
    vec3 specular_light = u_specular * pow( max(dot(R,V), 0.0f) , u_shininess) * u_Is;

    //Final phong equation
    vec3 light_intensity = ambient_light + diffuse_light + specular_light;
    return light_intensity * u_light_intensity;
}

float sampling_volume(in float x, in float y, in float z ){
    vec3 sample_pos = vec3(x,y,z);
    vec3 uv_3D = (sample_pos + 1.0f)*0.5f;
    float d = texture3D(u_texture, uv_3D).x;
    return d;
}

vec3 compute_gradient(const in vec3 sample_pos, const in float h){
    
    float f1 = sampling_volume(sample_pos.x + h,  sample_pos.y,  sample_pos.z );
    float f2 = sampling_volume(sample_pos.x,  sample_pos.y + h,  sample_pos.z );
    float f3 = sampling_volume(sample_pos.x,  sample_pos.y ,  sample_pos.z + h );
    float f4 = sampling_volume(sample_pos.x - h,  sample_pos.y ,  sample_pos.z );
    float f5 = sampling_volume(sample_pos.x,  sample_pos.y - h,  sample_pos.z );
    float f6 = sampling_volume(sample_pos.x,  sample_pos.y,  sample_pos.z - h);

    f1 = f1-f4;
    f2 = f2-f5;
    f3 = f3-f6;
    
    vec3 gradient = normalize(- 0.5*1/h*vec3(f1,f2,f3));
	return gradient;
}

void main(){
   
    float texture_width = 128.0f;
    vec2 uv_screen =  gl_FragCoord.xy / texture_width; 

    // 1. Ray setup
    vec4 camera_l_pos_temp = (u_inverse_model * vec4(u_camera_position, 1.0));
    vec3 camera_l_pos = camera_l_pos_temp.xyz/camera_l_pos_temp.w;
    vec3 ray_dir = normalize(v_position - camera_l_pos);
    vec3 step_vector = u_length_step*ray_dir;

    //Jiterring
    float offset = 0.0f;
    vec3 step_offset = vec3(0.0f);
    #ifdef USE_JITTERING
        offset = texture2D(u_noise_texture, uv_screen).x ;
        step_offset = offset*step_vector;
    #endif
    
    vec3 sample_pos = v_position + step_offset; //initialized as entry point to the volume

    // Initialize values that are computed in the loop
	vec4 final_color = vec4(0.0f);
    vec4 sample_color = vec4(0.0f);
    vec3 uv_3D = vec3(0.0f);
    float d = 0.0f;
    float plane = 0.0f;
    bool clipping_or_plane = false;
    // Ray loop
    for(int i=0; i<MAX_ITERATIONS; i++){
       
        // 2. Volume sampling
        //Get the value of the volume in the sample point
        uv_3D = (sample_pos + 1.0f)*0.5f;
        d = texture3D(u_texture, uv_3D).x;

        // 3. Classification:
        // Basic classification 
        // We store in the last component the factor density
        sample_color = vec4(u_color.r,u_color.g,u_color.b,d);


        // Transfer Function
        #ifdef USE_TF
            //Map the volume value into a color using LUT textures
            vec3 tf_color = texture2D(u_tf_mapping_texture, vec2(d,1)).xyz;
            sample_color = vec4(tf_color.r, tf_color.g, tf_color.b,d);
        #endif

        #ifdef USE_TF_DEBUG
            // If we are in TF debug mode, we want to visualize only those points 
            // that have a density lower than that marked in the ImGui
            if (d>u_max_density){
                discard;
            }
            if(d<u_density_limits.x)
                sample_color = vec4(u_tf_fst_color.r,u_tf_fst_color.g,u_tf_fst_color.b,d);
            else if(d>u_density_limits.x && d<u_density_limits.y)
                sample_color = vec4(u_tf_snd_color.r,u_tf_snd_color.g,u_tf_snd_color.b,d);
            else if(d>u_density_limits.y && d<u_density_limits.z)
                sample_color = vec4(u_tf_trd_color.r,u_tf_trd_color.g,u_tf_trd_color.b,d);
            else if(d>u_density_limits.z && d<u_density_limits.w)
                sample_color = vec4(u_tf_frth_color.r,u_tf_frth_color.g,u_tf_frth_color.b,d);
        #endif

        // Highlight some parts according to the imgui
        if(d<u_density_limits.x)
            sample_color = sample_color*u_highlight.x;
        else if (d>u_density_limits.x && d<u_density_limits.y)
            sample_color = sample_color*u_highlight.y;
        else if (d>u_density_limits.y && d<u_density_limits.z)
            sample_color = sample_color*u_highlight.z;
        else if (d>u_density_limits.z && d<u_density_limits.w)
            sample_color = sample_color*u_highlight.w;

        
        // 4. Composition

        //Clipping and Isosurface with phong
        //We compute the plane before the nextsample in case the first sample is also in the side we want to hide
		plane = u_plane_abcd.x*sample_pos.x + u_plane_abcd.y*sample_pos.y + u_plane_abcd.z*sample_pos.z + u_plane_abcd.w;

        //We check the side that we do not want to hide and treat it condition like a boolean
        //Since we use the macro like the flag to activate the function
        //we will use this to update the flag. 
        //The idea is to add the contribution of the results in case there is NO clipping and NOT in the side that we want to hide
        //If occur the opposite we will skip these iterations
        clipping_or_plane = plane<=0;
        #ifndef USE_CLIPPING
            clipping_or_plane = true;
        #endif
        
        if (clipping_or_plane) 
            //We check if we have to change the composition schemes
            //Since for phong we use First, and otherwise accumulation
            if(!u_phong_flag){
                sample_color.rgb *= sample_color.a; //attenuates the color according to the density
                final_color += u_length_step * (1.0 - final_color.a) * sample_color;     
            }
            //We find first value of density higher than a certain threshold (isosurface)
            //Once we find it, we break the loop
            //And then, we compute the gradient and the light
            //We also add two flags to show the isosurface and gradient
            else if( sample_color.a > u_iso_threshold ) { 
                if(u_shade_flag){
                    final_color = sample_color;
                    break;
                }
                vec3 gradient = compute_gradient(sample_pos, u_h_threshold);
                if(u_gradient_flag){
                    final_color = vec4(gradient,1.0)/u_brightness; 
                    break;
                }
                vec3 light_intensity = compute_lightPhong(gradient, sample_color); 
                final_color = sample_color * vec4(light_intensity, 1.0) ; //add the ilumination
                break;
            }        

        // 5. Next sample
        sample_pos += step_vector;
        
        // 6. Early termination
        //If is outside of the auxiliar mesh
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

    //We discard those pixels (noise or surface of the cube) that we are not interested
    // according with the density component
    if (final_color.w <= u_discard_threshold){ 
        discard;
    }

    // 7. Final color
    gl_FragColor = final_color * u_brightness;
}

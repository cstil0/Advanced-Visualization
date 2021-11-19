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
uniform float u_h_threshold;

//boolean flags
uniform bool u_jittering_flag;
uniform bool u_clipping_flag;
uniform bool u_TF_flag;
uniform bool u_shade_flag;

//light parameters:
uniform vec3 u_light_pos;
uniform vec3 u_Ia, u_Id, u_Is; //Ambient, diffuse, specular
uniform float u_light_intensity;

//Material uniforms parameters
uniform vec3 u_diffuse;
uniform vec3 u_specular;
uniform float u_shininess;

uniform vec3 u_test;
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
    // vec3 light_intensity = vec3(0.0);
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
    // f1 = max(f1-f4, 0.0f);
    // f2 = max(f2-f5, 0.0f);
    // f3 = max(f3-f6, 0.0f);
    vec3 gradient = normalize(- 0.5*1/h*vec3(f1,f2,f3));
    // gradient = max(gradient,v);
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
    
    vec3 step_offset = vec3(0.0f);
    if (u_jittering_flag) {
        float offset = texture2D(u_noise_texture, uv_screen).x ;
        step_offset = offset * step_vector;   
    }

    vec3 sample_pos = v_position + step_offset; //initialiced as entry point to the volume

    // Initialize values that are computed in the loop
	vec4 final_color = vec4(0.0f);
    vec4 sample_color = vec4(0.0f);
    vec3 uv_3D = vec3(0.0f);
    float d = 0.0f;
    
    float plane = 0.0f;
    float h = u_h_threshold;
    vec3 gradient = vec3(0.0f);
    vec3 light_intensity = vec3(0.0f);
    
    // Ray loop
    for(int i=0; i<MAX_ITERATIONS; i++){
       
        // 2. Volume sampling
        uv_3D = (sample_pos + 1.0f)*0.5f;
        d = texture3D(u_texture, uv_3D).x;

        // 3. Classification
        if(u_TF_flag){
        //vec3 tf_color = texture2D(u_tf_mapping_texture, vec2(d,1)).xyz;
        // Con tf
        //sample_color = vec4(tf_color.r, tf_color.g, tf_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric4

        // Classification basica
        // sample_color = vec4(u_color.r*tf_color.r,u_color.g*tf_color.g,u_color.b*tf_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric
        }

        sample_color = vec4(u_color.r,u_color.g,u_color.b,d);//important that the d, 4ºcomponent. Para que funcione la volumetric
      
        // 4. Composition
        // aqui esta bn, pq el primer sample, tendra el plane a 0.0
        // si el flag de clipping esta activado q salte esta linea, si no hay clipping entre al if
        // usamos or por q cuando no hay clipiing tamb pueda hacer esta contribucion de final color!!
        if (!u_clipping_flag || plane <= 0.0f){ // que haga la suma total de las contribuciones, si no esto, pues no hay sumas...
            sample_color.rgb *= sample_color.a; //atenuendo el color dependiendo de alpha
            final_color += u_length_step * (1.0 - final_color.a) * sample_color; 
            
        }
        if(u_shade_flag && (sample_color.a > u_iso_threshold ) && (!u_clipping_flag || plane <= 0.0f )){
            gradient = compute_gradient(sample_pos, h);
            light_intensity = compute_lightPhong(gradient, final_color);
            final_color = sample_color * vec4(light_intensity, 1.0) ; //le damos la iluminacion
            // final_color = vec4(gradient,1.0);
            break;
        }
        // // Visualizing isosurfaces
        // if ( u_shade_flag && (sample_color.a > u_iso_threshold )){
        //     gradient = compute_gradient(sample_pos, h);
        //     light_intensity = compute_lightPhong(gradient, final_color);
        //     final_color = sample_color * vec4(light_intensity, 1.0) ; //le damos la iluminacion
        //     break;
        // }
        // else{
        //     sample_color.rgb *= sample_color.a; //atenuendo el color dependiendo de alpha
        //     final_color += u_length_step * (1.0 - final_color.a) * sample_color; 
        // }
            

        // 5. Next sample
        sample_pos += step_vector;
        
        //  (computing the next step always). entonces hay que ponerlo abajo ?¿??????
		plane = u_plane_abcd.x*sample_pos.x + u_plane_abcd.y*sample_pos.y + u_plane_abcd.z*sample_pos.z + u_plane_abcd.w;
        
        
        // else {
            
        //     if ( u_shade_flag && (sample_color.a > u_iso_threshold )){
        //         gradient = compute_gradient(sample_pos, h);
        //         light_intensity = compute_lightPhong(gradient, final_color);
        //         final_color = sample_color * vec4(light_intensity, 1.0) ; //le damos la iluminacion
        //         break;
        //     }
        //     sample_color.rgb *= sample_color.a; //atenuendo el color dependiendo de alpha
        //     final_color += u_length_step * (1.0 - final_color.a) * sample_color; 
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
    gl_FragColor = final_color * u_brightness ;
    // gl_FragColor = vec4(u_test,1.0);
    // gl_FragColor = final_color;
    // gl_FragColor = vec4(gradient,1.0);
}

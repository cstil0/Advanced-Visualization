
varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

//Utils uniforms parameters
uniform sampler2D u_texture;
uniform vec3 u_camera_pos;
uniform vec4 u_color;

//Lights uniforms parameters
#define MAX_LIGHTS 10 
uniform vec3 u_light_pos[MAX_LIGHTS];
uniform vec3 u_Ia;
uniform vec3 u_Id[MAX_LIGHTS], u_Is[MAX_LIGHTS]; //Ambient, diffuse, specular

//Material uniforms parameters
uniform vec3 u_specular;
uniform vec3 u_diffuse;
uniform float u_shininess;

void main()
{
    vec2 uv = v_uv;
    vec4 color = u_color;
    color *= texture2D( u_texture, uv );
    
    //We calculate the light equation vectors
    vec3 L = vec3(0.0f);
    vec3 N = normalize(v_normal); 
    vec3 V = normalize(u_camera_pos - v_world_position);
   
    //vec3 H = normalize(V + L);

    //We initialize the variables to store differents parts of lights
    vec3 ambient_light = color.xyz * u_Ia;
    vec3 diffuse_light = vec3(0.0f);
    vec3 specular_light = vec3(0.0f);

    //Total light factor
    vec3 light_intensity = vec3(0.0f);
     
    //Using singlepass architecture to accumulate total light factor.
    for (int i = 0; i < MAX_LIGHTS; i++){
        L = normalize(u_light_pos[i] - v_world_position);
        vec3 R = reflect(L,N);
        diffuse_light += u_diffuse * clamp(dot(L,N), 0.0f, 1.0f) * u_Id[i];
        specular_light += u_specular * pow( clamp(dot(R,V), 0.0, 1.0) , u_shininess) * u_Is[i];
    }
    //Final phong equation
    light_intensity += ambient_light + diffuse_light + specular_light;

    //gl_FragColor = vec4( 1.0, 0, 0, 1.0);

    gl_FragColor = vec4( light_intensity, 1.0);
}
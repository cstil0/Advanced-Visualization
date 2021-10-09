varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec3 u_camera_pos;
uniform mat4 u_model;

#define MAX_LIGHTS 10
uniform vec3 u_light_pos[MAX_LIGHTS];
uniform vec3 u_Ia[MAX_LIGHTS], u_Id[MAX_LIGHTS], u_Is[MAX_LIGHTS]; //Ambient, diffuse, specular

uniform vec3 u_specular;
uniform vec3 u_diffuse;
uniform float u_shininess;

void main()
{
    vec4 color = v_color;
    color += texture2D( u_texture, v_uv );

    vec3 L = vec3(0.0f);
    vec3 N = normalize(v_normal); 
    vec3 V = normalize(u_camera_pos- v_world_position);

    vec3 R = reflect(-L,N);
    //vec3 H = normalize(V + L);
    vec3 ambient_light = vec3(0.0f);
    vec3 diffuse_light = vec3(0.0f);
    vec3 specular_light = vec3(0.0f);
    vec3 light_intensity = vec3(0.0f);

    for (int i = 0; i < MAX_LIGHTS; i++){
        L = normalize(u_light_pos[i] - v_world_position);
        ambient_light += color.xyz * u_Ia[i];
        diffuse_light += u_diffuse * clamp(dot(L,N), 0.0f, 1.0f) * u_Id[i];
        specular_light += u_specular * pow( clamp(dot(R,V), 0.0, 1.0) , u_shininess) * u_Is[i];
     
    }
    light_intensity += ambient_light + diffuse_light + specular_light;

    gl_FragColor = vec4( light_intensity, 1.0);
}
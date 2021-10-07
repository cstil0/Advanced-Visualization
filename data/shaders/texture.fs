
varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

uniform sampler2D u_texture;

void main()
{
	vec2 uv = v_uv;
	vec4 color = v_color;
	color += texture2D( u_texture, v_uv );
	gl_FragColor = color;
}

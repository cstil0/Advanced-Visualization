
varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec4 u_color;

uniform sampler2D u_roughness_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_normalmap_texture;


struct PBRMat
{
	//properties
	float roughness;
	float metalness;
	// not sure aqui
	float F;
	float G;
	float D;

	//vectors
	vec3 N;
	vec3 L;
	vec3 H;
	vec3 R;
	vec3 V;
	
};


//roughness = texture2D(u_roughness, uv);


void main()
{

	vec2 uv = v_uv;
	vec4 color = u_color;
	//We assign the color of the pixel according to the texture coordinates
	color *= texture2D( u_texture, v_uv );
	gl_FragColor = color;

	// 1. Create Material
	// ...
	
	// 2. Fill Material
	// ...

	// 3. Shade (Direct + Indirect)
	// ...

	// 4. Apply Tonemapping
	// ...

	// 5. Any extra texture to apply after tonemapping
	// ...

	// Last step: to gamma space
	// ...

}

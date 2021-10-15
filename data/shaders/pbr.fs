//utilis definitions
#define PI 3.14159265359

//
varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
varying vec2 v_uv; // texture coordinates
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_normalmap_texture;

uniform vec3 u_camera_pos;

//Lights uniforms parameters
uniform vec3 u_light_pos;

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

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv){
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel ){
	#ifdef USE_POINTS
	return N;
	#endif

	// assume N, the interpolated vertex normal and
	// V, the view vector (vertex to eye)
	//vec3 normal_pixel = texture2D(normalmap, texcoord ).xyz;
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlick(float LdotN, vec3 F0)
{
    return F0 + ( 1.0 - F0) * pow(clamp(1.0 - LdotN, 0.0, 1.0), 5.0); // otra vez clmap??
}


float GeometryDistributionFunction(vec3 N, vec3 L, vec3 H, vec3 V){
	float NdotH = clamp(dot(N,H), 0.0f, 1.0f);
	float NdotV = clamp(dot(N,V), 0.0f, 1.0f);
	float VdotH = clamp(dot(V,H), 0.0f, 1.0f);
	float NdotL = clamp(dot(N,L), 0.0f, 1.0f);

	return min(1, min( (2*NdotH*NdotV)/VdotH , (2*NdotH*NdotL)/VdotH ) );
}

//void computeVectors(out L, ) PARA CALCULAR LOS VECTORES, PERO HAY QUE MIRAR SI ES IN O OUT


void main()
{
	vec2 uv = v_uv;

	// Compute the light equation vectors
	vec3 L = normalize(u_light_pos - v_world_position);
    vec3 normal = normalize(v_normal); 
	vec3 normal_pixel = texture2D(u_normalmap_texture, uv).xyz;
	vec3 V = normalize(u_camera_pos - v_world_position);

	vec3 N = perturbNormal(normal, V, uv, normal_pixel );
	vec3 H = normalize(V + L);
	vec3 R = reflect(L, N);

	vec4 albedo = u_color;

	// textures:
	
	albedo *= texture2D( u_texture, uv ); //base color
	float roughness = texture2D(u_roughness_texture, uv).x;
	float metalness = texture2D(u_metalness_texture, uv).x;

	vec3 f_diffuse = ((1.0 - metalness) *albedo.xyz) / PI; //since we are doing the linear interpolation with dialectric and conductor material
 	vec3 F0 = mix(0.04, albedo.xyz, metalness);

	float LdotN = clamp(dot(L,N), 0.0f, 1.0f); //? OR MAYBE nDOTl
	// vec3 F = FresnelSchlickRoughness( LdotN, F0, roughness);
	vec3 F = FresnelSchlick( LdotN, F0 );

	float G = GeometryDistributionFunction(N, H, L, V);
	




	// vec3 f_specular = 0.0;





	gl_FragColor = vec4(F, 1.0);

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

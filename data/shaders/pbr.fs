
//utilis definitions
#define PI 3.14159265359

varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
varying vec2 v_uv; // texture coordinates
//varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_normalmap_texture;
uniform sampler2D u_mr_texture;

uniform vec3 u_camera_pos;
uniform vec3 u_ambient_light;
uniform float u_output;
uniform bool u_met_rou;

//factors
uniform float u_roughness;
uniform float u_metalness;

//Lights uniforms parameters
uniform vec3 u_light_pos;

struct PBRMat
{
	//properties
	float roughness;
	float metalness;

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

//------------functions for Specular BRDP----------------------------------------------------

float DistributionGGX(const in float roughness, const in float NdotH)
{
	float alpha2 = roughness*roughness;
	float denom = (NdotH*NdotH) * (alpha2 - 1.0) + 1.0;
	denom = PI * denom * denom;
    return alpha2 / denom ;
}

vec3 FresnelSchlick(const in float LdotN, const in vec3 F0)
{
    return F0 + ( 1.0 - F0) * pow( clamp(1.0 - LdotN, 0.0, 1.0) , 5.0); 
}

vec3 FresnelSchlickRoughness(const in float cosTheta, const in vec3 F0, const in float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow( clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float GeometryGGX(const in float dotVector, const in float k )
{
	float nom = dotVector;
    float denom = dotVector * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(const in float NdotV, const in float NdotL, const in float roughness)
{
	float k = ( (roughness+1)*(roughness+1) ) / 8.0;
	float G1_v = GeometryGGX(NdotV, k);
	float G1_n = GeometryGGX(NdotL, k);
	return G1_v * G1_n;
}

vec3 BRDFSpecular(float roughness, float metalness, float NdotH, float LdotN, float NdotV, float NdotL, vec3 baseColor)
{
	float D = DistributionGGX(roughness, NdotH);

	vec3 F0 = mix( vec3(0.04), baseColor, metalness);
	vec3 F = FresnelSchlick( LdotN, F0 );

	float G = GeometrySmith(NdotV, NdotL, roughness);
	
	return (F*G*D) / (4.0 * NdotL * NdotV + 1e-7 ); // in case of zero div
}

// void computeVectors(vec3 u_light_pos, vec3 v_world_position, vec3 u_camera_pos, vec3 v_normal, )
// {
// 	vec3 L = normalize(u_light_pos - v_world_position);
// 	vec3 V = normalize(u_camera_pos - v_world_position);
// 	vec3 normal = normalize(v_normal); 
// 	vec3 normal_pixel = texture2D(u_normalmap_texture, uv).xyz;
// 	vec3 N = perturbNormal(normal, V, uv, normal_pixel );
// 	vec3 H = normalize(V + L);
// 	vec3 R = reflect(L, N);
// } 

void main()
{
	vec2 uv = v_uv;

	// Compute the light equation vectors
	vec3 L = normalize(u_light_pos - v_world_position);
	vec3 V = normalize(u_camera_pos - v_world_position);
	vec3 normal = normalize(v_normal); 
	vec3 normal_pixel = texture2D(u_normalmap_texture, uv).xyz;
	vec3 N = perturbNormal(normal, V, uv, normal_pixel );
	vec3 H = normalize( V + L);
	vec3 R = reflect(-L, N);


	float NdotH = max(dot(N,H), 0.0f);
	float NdotV = max(dot(N,V), 0.0f);
	float VdotH = max(dot(V,H), 0.0f);
	float NdotL = max(dot(N,L), 0.0f);
	float LdotN = max(dot(L,N), 0.0f); 
	vec4 color = u_color;


	// textures:
	color *= texture2D( u_texture, uv ); //base color
	float roughness_tex = 0.0;
	float metalness_tex = 0.0;
	if (u_met_rou){
		roughness_tex = texture2D(u_mr_texture, uv).y;
		metalness_tex = texture2D(u_mr_texture, uv).z;
	}
	else{
		roughness_tex = texture2D(u_roughness_texture, uv).x;
		metalness_tex = texture2D(u_metalness_texture, uv).x;
	}
	
	float roughness = roughness_tex * u_roughness; //total roughnesss
	float metalness = metalness_tex * u_metalness; //total metalness

	// BSDF: bidirectional scattering distribution function
	vec3 f_diffuse = ((1.0 - metalness) * color.xyz) / PI; //since we are doing the linear interpolation with dialectric and conductor material
	//vec3 f_diffuse = mix( vec3(0.0), color.xyz, metalness) / PI; 

	vec3 f_specular = BRDFSpecular( roughness, metalness, NdotH, LdotN, NdotV, NdotL, color.xyz );
	
	// float D = DistributionGGX(roughness, NdotH);

	// vec3 F0 = mix( vec3(0.04), color.xyz, metalness);
	// vec3 F = FresnelSchlick( LdotN, F0 );

	// float G = GeometrySmith(NdotV, NdotL, roughness);
	

	vec3 direct = f_diffuse + f_specular;
	//vec3 direct = f_diffuse ;
	//u_light_intensity = NdotL;
	//compute how much light received the pixel
	//vec3 lightParams = u_light_color * u_light_intensity ;

	vec3 light = direct * NdotL;

	
	if ( u_output == 1.0 )
		color = texture2D( u_texture, uv );
	else if(u_output == 2.0)
		color = texture2D(u_roughness_texture, uv);
	else if(u_output == 3.0)	
		color = texture2D(u_metalness_texture, uv);
	else
		color = vec4(N, 1.0);

	//gl_FragColor = color;
	
	//gl_FragColor = vec4(G);
	//gl_FragColor = vec4(G*NdotL);
	gl_FragColor = vec4(light, 1.0);



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

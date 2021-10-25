//utilis definitions
#define PI 3.14159265359

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;
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
uniform sampler2D u_met_rou; // ESTA TEXTURA Y LA DE ABAJO SON LA MISMA NO??
uniform sampler2D u_mr_texture;

uniform samplerCube u_texture_prem;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

uniform float u_roughness; // ??
uniform float u_metalness; //??

uniform sampler2D u_BRDFLut;

uniform vec3 u_camera_pos;

//Lights uniforms parameters
uniform vec3 u_light_pos;

struct PBRStruct
{
	vec4 albedo;
	
	//properties
	float roughness;
	float metalness;
	float roughness_tex;
	float metalness_tex;

	//vectors
	vec3 N;
	vec3 V;
	vec3 L;
	vec3 H;
	vec3 R;

	float NdotH;
	float NdotV;
	float VdotH;
	float NdotL;
	float LdotN;

	vec3 f_diffuse;
	vec3 f_specular;

}PBRMat;

// degamma
vec3 gamma_to_linear(vec3 color)
{
	return pow(color, vec3(GAMMA));
}

// gamma
vec3 linear_to_gamma(vec3 color)
{
	return pow(color, vec3(INV_GAMMA));
}

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

vec3 getReflectionColor(vec3 r, float roughness)
{
	float lod = roughness * 5.0;

	vec4 color;

	if(lod < 1.0) color = mix( textureCube(u_texture_prem, r), textureCube(u_texture_prem_0, r), lod );
	else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, r), textureCube(u_texture_prem_1, r), lod - 1.0 );
	else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, r), textureCube(u_texture_prem_2, r), lod - 2.0 );
	else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, r), textureCube(u_texture_prem_3, r), lod - 3.0 );
	else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, r), textureCube(u_texture_prem_4, r), lod - 4.0 );
	else color = textureCube(u_texture_prem_4, r);

	//color.rgb = linear_to_gamma(color.rgb);

	return color.rgb;
}

void computeVectors(inout PBRStruct mat)
{
	mat.L = normalize(u_light_pos - v_world_position);
	mat.V = normalize(u_camera_pos - v_world_position);
	vec3 normal = normalize(v_normal); 
	vec3 normal_pixel = texture2D(u_normalmap_texture, v_uv).xyz;
	mat.N = perturbNormal(normal, mat.V, v_uv, normal_pixel );
	mat.H = normalize(mat.V + mat.L);
	mat.R = reflect(-mat.L, mat.N);
} 

void computeDotProducts(inout PBRStruct mat)
{
	mat.NdotH = max(dot(mat.N,mat.H), 0.0f);
	mat.NdotV = max(dot(mat.N,mat.V), 0.0f);
	mat.VdotH = max(dot(mat.V,mat.H), 0.0f);
	mat.NdotL = max(dot(mat.N,mat.L), 0.0f);
	mat.LdotN = max(dot(mat.L,mat.N), 0.0f);
}

void fillPBRProperties(inout PBRStruct mat)
{
	mat.albedo = u_color;
	mat.albedo *= texture2D( u_texture, v_uv ); //base color
	//roughness_tex = 0.0;
	//float metalness_tex = 0.0;
	if (u_met_rou){
		mat.roughness_tex = texture2D(u_mr_texture, v_uv).y;
		mat.metalness_tex = texture2D(u_mr_texture, v_uv).z;
	}
	else{
		mat.roughness_tex = texture2D(u_roughness_texture, v_uv).x;
		mat.metalness_tex = texture2D(u_metalness_texture, v_uv).x;
	}
	
	mat.roughness = mat.roughness_tex * u_roughness; //total roughnesss
	mat.metalness = mat.metalness_tex * u_metalness; //total metalness
}

//void computeVectors(out L, ) PARA CALCULAR LOS VECTORES, PERO HAY QUE MIRAR SI ES IN O OUT


void main()
{
	// vec2 uv = v_uv;
	// vec4 color = u_color;

	// 1. Create Material
	PBRMat = PBRStruct(

						vec4(0.0),

						0.0f, //roughness
						0.0f, //metalness
						0.0f,
						0.0f,

						vec3(0.0f), //vectors
						vec3(0.0f),
						vec3(0.0f),
						vec3(0.0f),
						vec3(0.0f),
						
						0.0f, //dot products
						0.0f,
						0.0f,
						0.0f, 
						0.0f,
						
						vec3(0.0f),
						vec3(0.0f)
						);

	// 2. Fill Material
	computeVectors(PBRMat);
	computeDotProducts(PBRMat);
	fillPBRProperties(PBRMat);
	vec2 uv = v_uv;

	// Compute the light equation vectors
	// vec3 L = normalize(u_light_pos - v_world_position);
    // vec3 normal = normalize(v_normal); 
	// vec3 normal_pixel = texture2D(u_normalmap_texture, uv).xyz;
	// vec3 V = normalize(u_camera_pos - v_world_position);

	// vec3 N = perturbNormal(normal, V, uv, normal_pixel );
	// vec3 H = normalize(V + L);
	// vec3 R = reflect(L, N);

	// vec4 albedo = u_color;

	// Gamma space
	vec3 albedo_gamma = gamma_to_linear(PBRMat.albedo.xyz); // GUARDAR DENTRO DE MAT??

	// textures:
	// albedo_gamma *= texture2D( u_texture, uv ).xyz; //base color
	// float roughness = texture2D(u_roughness_texture, uv).x;
	// float metalness = texture2D(u_metalness_texture, uv).x;

	// roughness *= u_roughness;
	// metalness *= u_metalness;

	// vec3 f_diffuse = ((1.0 - metalness) * albedo.xyz) / PI; //since we are doing the linear interpolation with dialectric and conductor material
 	// vec3 F0 = mix( vec3(0.04), albedo.xyz, metalness);

	// float LdotN = clamp(dot(L,N), 0.0f, 1.0f); //? OR MAYBE nDOTl
	// // vec3 F = FresnelSchlickRoughness( LdotN, F0, roughness);
	// vec3 F = FresnelSchlick( LdotN, F0 );

	// float G = GeometryDistributionFunction(N, H, L, V);

	// -- INDIRECT--
	// -- IBL - specular term --

	//Incomming light term
	vec3 specularSample = getReflectionColor(PBRMat.R, PBRMat.roughness);
	//Material Response term
	// float NdotV = clamp(dot(N,V), 0.0f, 1.0f);
	// float HdotV = clamp(dot(H,V), 0.0f, 1.0f);
	// float LdotN = clamp(dot(L,N), 0.0f,1.0f);
	vec2 LUT_coord = vec2(PBRMat.NdotV, PBRMat.roughness);
	vec4 brdf2D = texture2D( u_BRDFLut, LUT_coord );

	vec3 F0 = mix(vec3(0.04), albedo_gamma.xyz, PBRMat.metalness);
	// float LdotN = clamp(dot(L,N), 0.0f, 1.0f);
	vec3 F_rg = FresnelSchlickRoughness(PBRMat.VdotH, F0, PBRMat.roughness); // POR ALGUN MOTIVO ESTÁ COMO TENIENDO EN CUENTA LA LUZ Y TIENE MUY MALA PUNTA

	vec3 specularBRDF = F_rg * brdf2D.x + brdf2D.y;
	vec3 specularIBL = specularSample * specularBRDF;

	// -- IBL - diffuse term --
	// Incomming light term
	float most_roughness = 1.0f; 
	vec3 diffuseSample = getReflectionColor(PBRMat.N, most_roughness);

	// Material response term
	vec3 diffuseColor = mix(albedo_gamma.xyz, vec3(0.0f),PBRMat.metalness) / PI; //since we are doing the linear interpolation with dialectric and conductor material
	vec3 diffuseIBL = diffuseSample * diffuseColor;
	diffuseIBL *= (1-F_rg);

	vec3 finalIBL = specularIBL +  diffuseIBL;

	// Gamma space
	finalIBL = linear_to_gamma(vec3(PBRMat.roughness));
	gl_FragColor = vec4(finalIBL, 0.0f);

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

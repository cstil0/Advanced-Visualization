
//utilis definitions
#define PI 3.14159265359


varying vec3 v_position; //position in local coords
varying vec3 v_world_position; //position in world coord
varying vec3 v_normal; //normal in the pixel
varying vec2 v_uv; // texture coordinates
//varying vec4 v_color;

//Utils uniforms
uniform vec4 u_color;//color of the material, that is the base color
uniform vec3 u_camera_pos;

//Lights uniforms parameters
uniform vec3 u_light_pos;
uniform vec3 u_ambient_light;
uniform vec3 u_light_color;
uniform float u_light_intensity;

//Textures
uniform sampler2D u_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_normalmap_texture;
uniform sampler2D u_mr_texture; 

//boolean
uniform float u_output;
uniform bool u_met_rou;

//IMGUI factors
uniform float u_roughness;
uniform float u_metalness;

struct matStruct {
    vec4 ambientColor ;
    vec4 diffuseColor ;
} newMaterial;

struct PBRStruct
{
	vec4 color;
	
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

	float NdotH ;
	float NdotV ;
	float NdotL ;

	vec3 f_diffuse;
	vec3 f_specular;

}PBRMat;

//---------------Get Normalmap functions-----------------------------
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

//---------------functions for Specular BRDF-----------------------------
// BSDF: bidirectional scattering distribution function
float DistributionGGX(const in float roughness, const in float NdotH)
{
	float alpha = roughness*roughness;
	float alpha2 = alpha*alpha;
	float denom = (NdotH*NdotH) * (alpha2 - 1.0) + 1.0;
	denom = PI * denom * denom;
    return alpha2 / denom ;
}

vec3 FresnelSchlick(const in float cosTheta, const in vec3 F0)
{
    return F0 + ( 1.0 - F0) * pow( clamp(1.0 - cosTheta, 0.0, 1.0) , 5.0);  /////quizas cambiarlo con max!
}

vec3 FresnelSchlickRoughness(const in float cosTheta, const in vec3 F0, const in float roughness) // este cosTheta es loh or hov
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

vec3 BRDFSpecular(float roughness, float metalness, float NdotH, float NdotV, float NdotL, vec3 baseColor)
{
	float D = DistributionGGX(roughness, NdotH);

	vec3 F0 = mix( vec3(0.04), baseColor, metalness);
	vec3 F = FresnelSchlick( NdotL, F0 ); //same NdotL right????????

	float G = GeometrySmith(NdotV, NdotL, roughness);
	
	return (F*G*D) / (4.0 * NdotL * NdotV + 1e-7 ); // in case of zero div
}


//---------------compute Vectors-----------------------------

void computeVectors(inout PBRStruct mat)
{
	mat.L = normalize(u_light_pos - v_world_position);
	mat.V = normalize(u_camera_pos - v_world_position);
	vec3 normal = normalize(v_normal); 
	vec3 normal_pixel = texture2D(u_normalmap_texture, v_uv).xyz;
	mat.N = perturbNormal(normal, mat.V, v_uv, normal_pixel );
	mat.H = normalize(mat.V + mat.L);
	mat.R = reflect(mat.L, mat.N);
} 

void computeDotProducts(inout PBRStruct mat)
{
	mat.NdotH = max(dot(mat.N,mat.H), 0.0f);
	mat.NdotV = max(dot(mat.N,mat.V), 0.0f);
	//float VdotH = max(dot(V,H), 0.0f);
	mat.NdotL = max(dot(mat.N,mat.L), 0.0f);
	//float LdotN = max(dot(L,N), 0.0f);
}

void fillPBRProperties(inout PBRStruct mat)
{
	mat.color = u_color;
	mat.color *= texture2D( u_texture, v_uv ); //base color
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

void computeBRDF(inout PBRStruct mat)
{
	// vec4 colorMat = mat.color;
	mat.f_diffuse = ((1.0 - mat.metalness) * mat.color.xyz) / PI; //since we are doing the linear interpolation with dialectric and conductor material
	//vec3 f_diffuse = mix( vec3(0.0), color.xyz, metalness) / PI; 
	// mat.f_diffuse = colorMat.xyz
	mat.f_specular = BRDFSpecular( mat.roughness, mat.metalness, mat.NdotH, mat.NdotV, mat.NdotL, mat.color.xyz );
}

//-----
void test(inout matStruct mat)
{
	mat.ambientColor = vec4(u_light_color, 1.0);
	mat.diffuseColor = u_color;
}


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
						
						vec3(0.0f),
						vec3(0.0f)
						);

	// 2. Fill Material
	computeVectors(PBRMat);
	computeDotProducts(PBRMat);
	fillPBRProperties(PBRMat);

	// vec3 L = normalize(u_light_pos - v_world_position);
	// vec3 V = normalize(u_camera_pos - v_world_position);
	// vec3 normal = normalize(v_normal); 
	// vec3 normal_pixel = texture2D(u_normalmap_texture, uv).xyz;
	// vec3 N = perturbNormal(normal, V, uv, normal_pixel );
	// vec3 H = normalize( V + L);
	// vec3 R = reflect(-L, N);

	// float NdotH = max(dot(N,H), 0.0f);
	// float NdotV = max(dot(N,V), 0.0f);
	// //float VdotH = max(dot(V,H), 0.0f);
	// float NdotL = max(dot(N,L), 0.0f);
	// //float LdotN = max(dot(L,N), 0.0f); 
	
	// // textures:
	// color *= texture2D( u_texture, uv ); //base color
	// float roughness_tex = 0.0;
	// float metalness_tex = 0.0;
	// if (u_met_rou){
	// 	roughness_tex = texture2D(u_mr_texture, uv).y;
	// 	metalness_tex = texture2D(u_mr_texture, uv).z;
	// }
	// else{
	// 	roughness_tex = texture2D(u_roughness_texture, uv).x;
	// 	metalness_tex = texture2D(u_metalness_texture, uv).x;
	// }
	
	// float roughness = roughness_tex * u_roughness; //total roughnesss
	// float metalness = metalness_tex * u_metalness; //total metalness


	// 3. Shade (Direct + Indirect)

	computeBRDF(PBRMat);
	

	// vec3 f_diffuse = ((1.0 - metalness) * color.xyz) / PI; //since we are doing the linear interpolation with dialectric and conductor material
	// //vec3 f_diffuse = mix( vec3(0.0), color.xyz, metalness) / PI; 

	// vec3 f_specular = BRDFSpecular( roughness, metalness, NdotH, LdotN, NdotV, NdotL, color.xyz );
	
	// //-----------dubug
	// // float D = DistributionGGX(roughness, NdotH);
	// // vec3 F0 = mix( vec3(0.04), color.xyz, metalness);
	// // vec3 F = FresnelSchlick( LdotN, F0 );
	// // float G = GeometrySmith(NdotV, NdotL, roughness);
	
	vec3 direct = PBRMat.f_diffuse + PBRMat.f_specular;

	// //compute how much light received the pixel
	vec3 light = vec3(0.0);
	// //light += u_ambient_light; -> preguntar si fuese NECESARIO!

	light += direct * PBRMat.NdotL * u_light_color * u_light_intensity;

	//---to debug the textures
	vec4 finalColor = vec4(0.0);

	if ( u_output == 0.0 ) //complete
		finalColor = vec4(light, 1.0);
	else if ( u_output == 1.0 ) //albedo
		finalColor = texture2D( u_texture, v_uv );
	else if(u_output == 2.0)//roughness
		finalColor = vec4(PBRMat.roughness_tex);
	else if(u_output == 3.0)//metalness
		finalColor = vec4(PBRMat.metalness_tex);
	else
		finalColor = vec4(PBRMat.N, 1.0);

	//gl_FragColor = color;
	//gl_FragColor = vec4(G);
	//gl_FragColor = vec4(G*NdotL);
	// gl_FragColor = vec4(light, 1.0);


	newMaterial = matStruct(vec4(0.0),vec4(0.0));

	test(newMaterial);					
							
	gl_FragColor = finalColor;
	// gl_FragColor = vec4(PBRMat.f_diffuse, 1.0);



	// 4. Apply Tonemapping
	// ...

	// 5. Any extra texture to apply after tonemapping
	// ...

	// Last step: to gamma space
	// ...

}

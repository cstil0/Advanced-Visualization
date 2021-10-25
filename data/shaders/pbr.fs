
//utilis definitions
#define PI 3.14159265359
#define RECIPROCAL_PI 0.3183098861837697

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

//varying variables
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
uniform vec3 u_light_color;
uniform vec3 u_ambient_light;
uniform float u_light_intensity;

//Textures
uniform sampler2D u_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_normalmap_texture;
uniform sampler2D u_mr_texture; 
uniform sampler2D u_emissive_texture;
uniform sampler2D u_BRDFLut;

//HDRE 
uniform samplerCube u_texture_prem;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

//boolean 
uniform float u_output;
uniform bool u_met_rou;

//IMGUI factors
uniform float u_roughness;
uniform float u_metalness;


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

//---------------Gamma correction-----------------------------

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

//---------------Tonemap correction-----------------------------

vec3 toneMap(vec3 color)
{
    return color / (color + vec3(1.0));
}

vec3 toneMapUncharted2Impl(vec3 color)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 toneMapUncharted(vec3 color)
{
    const float W = 11.2;
    color = toneMapUncharted2Impl(color * 2.0);
    vec3 whiteScale = 1.0 / toneMapUncharted2Impl(vec3(W));
    return color * whiteScale;
}

//---------------PBRStruct struct-----------------------------

struct PBRStruct
{
	vec3 color;
	
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
	float LdotH ; 
	//float LdotV ;

	vec3 F0;
	vec3 F_RG;
	vec3 f_diffuse;
	vec3 f_specular;


}PBRMat;


//---------------Fill PBR Materials Functions-----------------------------

void computeVectors(inout PBRStruct mat)
{
	mat.L = normalize(u_light_pos - v_world_position);
	mat.V = normalize(u_camera_pos - v_world_position);
	vec3 normal = normalize(v_normal); 
	vec3 normal_pixel = texture2D(u_normalmap_texture, v_uv).xyz;
	mat.N = perturbNormal(normal, mat.V, v_uv, normal_pixel );
	mat.H = normalize(mat.V + mat.L);
	mat.R = reflect(-mat.V, mat.N);
} 

void computeDotProducts(inout PBRStruct mat)
{
	mat.NdotH = max(dot(mat.N,mat.H), 0.0f);
	mat.NdotV = clamp(dot(mat.N,mat.V), 0.1f, 0.9f);
	mat.NdotL = max(dot(mat.N,mat.L), 0.0f);
	mat.LdotH = max(dot(mat.L,mat.H), 0.0f);
	//mat.LdotV = max(dot(mat.L,mat.V), 0.0f);

}

void fillPBRProperties(inout PBRStruct mat)
{
	mat.color = u_color.xyz;//base color
	mat.color *= texture2D( u_texture, v_uv ).xyz; 
	mat.color = gamma_to_linear(mat.color);
	if (u_met_rou){
		mat.roughness_tex = texture2D(u_mr_texture, v_uv).y;
		mat.metalness_tex = texture2D(u_mr_texture, v_uv).z;
	}
	else{
		mat.roughness_tex = texture2D(u_roughness_texture, v_uv).x;
		mat.metalness_tex = texture2D(u_metalness_texture, v_uv).x;
	}
	
	mat.roughness = clamp(mat.roughness_tex * u_roughness, 0.1f, 0.9f); //total roughness and clamp it 
	mat.metalness = mat.metalness_tex * u_metalness; //total metalness
	mat.F0 = mix( vec3(0.04), mat.color, mat.metalness);

}

//---------------functions for BRDF-----------------------------

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

vec3 BRDFSpecular(float roughness, float metalness, float NdotH, float NdotV, float NdotL, vec3 baseColor, vec3 F0)
{
	float D = DistributionGGX(roughness, NdotH);

	vec3 F = FresnelSchlick( NdotL, F0 ); 

	float G = GeometrySmith(NdotV, NdotL, roughness);
	
	return (F*G*D) / (4.0 * NdotL * NdotV + 1e-7 ); // in case of zero div
}


void computeBRDF(inout PBRStruct mat)
{
	//mat.f_diffuse = ((1.0 - mat.metalness) * mat.color) * RECIPROCAL_PI; //since we are doing the linear interpolation with dialectric and conductor material
	mat.f_diffuse = mix( mat.color, vec3(0.0), mat.metalness) * RECIPROCAL_PI; 
	mat.f_specular = BRDFSpecular( mat.roughness, mat.metalness, mat.NdotH, mat.NdotV, mat.NdotL, mat.color, mat.F0 );
}

//---------------compute IBL-----------------------------

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

vec3 computeSpecularIBL(inout PBRStruct mat){
	mat.F_RG = FresnelSchlickRoughness( mat.LdotH, mat.F0 , mat.roughness ); 
	vec2 brdf_coord = vec2( mat.NdotV, mat.roughness);
	vec4 brdf2D = texture2D(u_BRDFLut, brdf_coord);// not sure el orden

	vec3 specularSample = getReflectionColor(mat.R, mat.roughness);
	vec3 specularBRDF = mat.F_RG * brdf2D.x + brdf2D.y;
	vec3 specularIBL = specularSample * specularBRDF;
	return specularIBL;
}

vec3 computeDiffuseIBL(inout PBRStruct mat){
	vec3 diffuseSample = getReflectionColor(mat.N, 1.0f); //less roughness = 1
	vec3 diffuseColor = mat.f_diffuse;
	vec3 diffuseIBL = diffuseSample * diffuseColor;
	diffuseIBL *= (1 - mat.F_RG);
	return diffuseIBL;
}

vec3 computeIBL (inout PBRStruct mat){
	vec3 specularIBL = computeSpecularIBL(mat);
	vec3 diffuseIBL = computeDiffuseIBL(mat);
	return specularIBL + diffuseIBL;
	 
}


void main()
{
	// 1. Create Material
	PBRMat = PBRStruct(

						vec3(0.0), //color

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
						
						
						vec3(0.0f), //F0
						vec3(0.0f), 
						vec3(0.0f),
						vec3(0.0f)
						);

	// 2. Fill Material
	computeVectors(PBRMat);
	computeDotProducts(PBRMat);
	fillPBRProperties(PBRMat);

	// 3. Shade (Direct + Indirect)
	computeBRDF(PBRMat);
	vec3 direct = PBRMat.f_diffuse + PBRMat.f_specular;
	vec3 indirect = computeIBL(PBRMat);

	// Compute how much light received the pixel
	vec3 light = vec3(0.0);
	light += direct * PBRMat.NdotL * u_light_color * u_light_intensity;
	light += indirect;

	// 4. Apply Tonemapping
	light = toneMapUncharted(light);

	// 5. Any extra texture to apply after tonemapping
	//apply emmisive tex

	light += gamma_to_linear( texture2D(u_emissive_texture, v_uv).xyz);

	//---to debug the textures with diff cases
	vec4 finalColor = vec4(0.0);
	if ( u_output == 0.0 ) //complete
		finalColor = vec4(linear_to_gamma(light), 0.0f);
		//finalColor = vec4(D);
	else if ( u_output == 1.0 ) //albedo
		finalColor = texture2D( u_texture, v_uv );
	else if(u_output == 2.0)//roughness
		finalColor = vec4(PBRMat.roughness_tex);
	else if(u_output == 3.0)//metalness
		finalColor = vec4(PBRMat.metalness_tex);
	else if(u_output == 4.0)//normal
		finalColor = vec4(PBRMat.N, 1.0);
	else{ //LUT
		vec2 brdf_coord = vec2( PBRMat.NdotV, PBRMat.roughness);
		vec4 brdf2D = texture2D(u_BRDFLut, brdf_coord);
		finalColor = brdf2D;
	}

	gl_FragColor = finalColor;

}

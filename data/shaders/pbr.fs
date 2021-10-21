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

uniform samplerCube u_texture_prem;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

uniform float u_roughness;
uniform float u_metalness;

uniform sampler2D u_BRDFLut;

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

	roughness *= u_roughness;
	metalness *= u_metalness;

	// vec3 f_diffuse = ((1.0 - metalness) * albedo.xyz) / PI; //since we are doing the linear interpolation with dialectric and conductor material
 	// vec3 F0 = mix( vec3(0.04), albedo.xyz, metalness);

	// float LdotN = clamp(dot(L,N), 0.0f, 1.0f); //? OR MAYBE nDOTl
	// // vec3 F = FresnelSchlickRoughness( LdotN, F0, roughness);
	// vec3 F = FresnelSchlick( LdotN, F0 );

	// float G = GeometryDistributionFunction(N, H, L, V);

	// -- INDIRECT--
	// -- IBL - specular term --

	//Incomming light term
	vec3 specularSample = getReflectionColor(N, roughness); // HE PUESTO N POR QUE ES EL QUE MEJOR SE VE, PERO NO LO ACABO DE ENTENDER YA QUE LA PARTE ESPECULAR SI QUE DEPENDE DE V...NO?

	//Material Response term
	float NdotV = clamp(dot(N,V), 0.0f, 1.0f);
	vec2 LUT_coord = vec2(NdotV, roughness);
	vec4 brdf2D = texture2D( u_BRDFLut, LUT_coord );
	// CUANDO UNAMOS LAS DOS PARTES ESTO ESTARÁ REPETIDO
	vec3 F0 = mix(vec3(0.04), albedo.xyz, metalness);
	// float LdotN = clamp(dot(L,N), 0.0f, 1.0f);
	vec3 F_rg = FresnelSchlickRoughness(NdotV, F0, roughness);

	vec3 specularBRDF = F_rg * brdf2D.x + brdf2D.y;
	vec3 specularIBL = specularSample * specularBRDF;

	// -- IBL - diffuse term --
	// Incomming light term
	float most_roughness = 0.9f; // He puesto 0.9 por que por algun motivo la textura u_texture_prem_4 es completamente negra...
	vec3 diffuseSample = getReflectionColor(N, most_roughness);

	// Material response term
	vec3 diffuseColor = mix(albedo.xyz, vec3(0.0f),metalness) / PI; //since we are doing the linear interpolation with dialectric and conductor material
	vec3 diffuseIBL = diffuseSample * diffuseColor;
	diffuseIBL *= (1-F_rg); // NO ESTOY MUY SEGURA DE QUE ESTE SEA EL TÉRMINO, YA QUE VEO QUE NO TIENE EL MISMO VALOR EN TODAS LAS COMPONENTES DEPENDIENDO DEL ROUGHNESS

	vec3 finalIBL = specularIBL +  diffuseIBL;
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

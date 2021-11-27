#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"

//Forward declaration
class Light;

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	StandardMaterial(Shader* sh, Texture* tex);
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

class PhongMaterial : public StandardMaterial {
public:

	vec3 specular;
	vec3 diffuse;
	float shininess;
	
	std::vector <Light*> light_list;

	PhongMaterial();
	PhongMaterial(Shader* sh, Texture* tex);
	~PhongMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void renderInMenu();
};

class SkyboxMaterial : public StandardMaterial {
public:
	SkyboxMaterial();
	SkyboxMaterial(Shader* sh);
	~SkyboxMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class PBRMaterial : public StandardMaterial {
public:

	//Textures
	Texture* normal_texture = NULL;
	Texture* roughness_texture = NULL;
	Texture* mr_texture = NULL;
	Texture* metalness_texture = NULL;
	Texture* emissive_texture = NULL;
	Texture* ao_texture = NULL;
	Texture* opacity_texture= NULL;

	Texture* BRDFLut = NULL;

	// factors
	float roughness;
	float metalness;
	
	//flags to know if the textures are active
	bool bool_met_rou;

	Light* light = NULL;

	PBRMaterial();
	PBRMaterial(Shader* sh, Texture* tex, Texture* normal, Texture* rough, Texture* metal, bool bool_mr, Texture* mr = NULL);
	~PBRMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void setTextures();
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class VolumeMaterial : public StandardMaterial {
public:
	enum TYPEOFMATERIAL {
		BASIC,
		PHONG
	};

	float length_step;
	float brightness;
	int typeOfMaterial;
	
	// Flags for different visualization
	//flags
	bool jittering_flag, clipping_flag, TF_flag, TF_debug_flag;
	bool jittering_flag_imgui, clipping_flag_imgui, TF_flag_imgui, TF_debug_flag_imgui;

	// Threshold
	vec4 plane_abcd;

	// Textures
	Texture* noise_texture;
	Texture* tf_mapping_texture;
	float discard_threshold;
	// Variables for TF generator
	float density_threshold_max;
	Vector4 density_limits;
	vec4 TF_first_color;
	vec4 TF_second_color;
	vec4 TF_third_color;
	vec4 TF_forth_color;

	// Used to know which of the four parts need to be highlighted
	vec4 highlight;
	//bool save_texture;

	VolumeMaterial();
	VolumeMaterial(Shader* sh, Texture* tex);
	~VolumeMaterial();

	void resetMaterialColor(int typeOfVolume);
	void setUniforms(Camera* camera, Matrix44 model, Matrix44 inverse_model);
	void render(Mesh* mesh, Matrix44 model, Matrix44 inverse_model, Camera* camera);
	void saveTexture();
	void renderInMenu();
	void renderInMenu_TF();
	void renderInMenu_highlight();
};

class VolumetricPhong :public VolumeMaterial {
public:

	vec3 specular;
	vec3 diffuse;
	float shininess;

	float iso_threshold;
	float h_threshold;

	Light* light = NULL;

	//flags
	bool shade_flag, gradient_flag;

	VolumetricPhong();
	VolumetricPhong(Shader* sh, Texture* tex);
	~VolumetricPhong();

	void setUniforms(Camera* camera, Matrix44 model, Matrix44 inverse_model);
	void render(Mesh* mesh, Matrix44 model, Matrix44 inverse_model, Camera* camera);
	void renderInMenu();
};

#endif

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
	//void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class SkyboxMaterial : public StandardMaterial {
public:

	Texture* panorama_tex = NULL;
	Texture* pisa_tex = NULL;

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

	//Texture* hdre_level0 = NULL;
	//Texture* hdre_level1 = NULL;
	//Texture* hdre_level2 = NULL;
	//Texture* hdre_level3 = NULL;
	//Texture* hdre_level4 = NULL;
	//Texture* hdre_level5 = NULL;

	Texture* BRDFLut = NULL;


	// factors
	float roughness;
	float metalness;
	//float spec_scale;
	//float reflactanc
	
	//flags
	bool bool_met_rou;
	bool bool_em;

	Light* light = NULL;

	PBRMaterial();
	PBRMaterial(Shader* sh, Texture* tex, Texture* normal, Texture* rough, Texture* metal, bool bool_mr, Texture* mr = NULL);
	~PBRMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};


#endif
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

//subclass-1----------------------------------------------

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	virtual void setUniforms(Camera* camera, Matrix44 model);
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera);
	virtual void renderInMenu();
};

//subclass-2----------------------------------------------

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

//subclass-3----------------------------------------------

class LightMaterial : public Material {
public:

	Light* light = NULL ; 

	vec3 ambient_intensity;
	vec3 diffuse_intensity;
	vec3 specular_intensity;

	vec3 specular;
	vec3 diffuse;
	float shininess;

	LightMaterial();
	~LightMaterial();

	void setUniforms(Camera* camera, Matrix44 model) ;
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class SkyboxMaterial : public Material {
public:

	SkyboxMaterial();
	~SkyboxMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
};

#endif
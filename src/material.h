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

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

//subclass-2----------------------------------------------

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

//subclass-3----------------------------------------------

class LightMaterial : public StandardMaterial {
public:

	vec3 specular;
	vec3 diffuse;
	float shininess;

	Light* light = NULL; 

	LightMaterial();
	~LightMaterial();

	//void setUniforms(Camera* camera, Matrix44 model, Light* light) ;
	//void render(Mesh* mesh, Matrix44 model, Camera* camera, Light* light);

	void setUniforms(Camera* camera, Matrix44 model) ;
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
	void renderInMenu();
};

class SkyboxMaterial : public StandardMaterial {
public:

	SkyboxMaterial();
	~SkyboxMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera* camera);
};

#endif
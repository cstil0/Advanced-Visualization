#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"

#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"

class Light;

class SceneNode {
public:
	// Used to identify which type of node it is
	enum TYPEOFNODE {
		SKYBOX,
		NODE,
		LIGHT
	};

	static unsigned int lastNameId;
	int typeOfNode;

	SceneNode();
	SceneNode(const char* name, Material* material, Mesh* mesh);
	
	~SceneNode();

	Material * material = NULL;
	std::string name;

	Mesh* mesh = NULL;
	Matrix44 model;

	bool visible_flag;

	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};

class Light : public SceneNode {
public:

	static unsigned int lastNameId;

	
	vec3 diffuse_intensity;
	vec3 specular_intensity;

	Light();
	Light(const char* name);
	~Light();

	void render(Camera* camera);
	void renderInMenu();
};

class Skybox : public SceneNode {
public:

	Skybox();
	Skybox(const char* name);
	~Skybox();

	void render(Camera* camera);
	void renderInMenu();
};

#endif
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

	static unsigned int lastNameId;
	int typeOfNode;

	SceneNode();
	SceneNode(const char* name);
	~SceneNode();

	Material * material = NULL;
	std::string name;

	Mesh* mesh = NULL;
	Matrix44 model;

	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};

class Light : public SceneNode {
public:

	static unsigned int lastNameId;

	vec3 ambient_intensity;
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
	//void updatePosition(Camera* camera);
};

#endif
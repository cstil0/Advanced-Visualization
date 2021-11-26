#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"

#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"

class Light;
class Volume;

class SceneNode {
public:

	// Used to identify which type of node it is
	enum TYPEOFNODE {
		SKYBOX,
		LIGHT,
		NODE,
		VOLUME
	};
	
	enum APPMODE {
		PHONG,
		PBR,
		VOLUME_MODE
	};

	// Used to identify which kind of mesh it have
	enum TYPEOFMODEL {
		BASIC,
		HELMET,
		LANTERN
	};

	static unsigned int lastNameId;
	int typeOfNode;
	int typeOfModel;

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
	
	float light_intensity;

	Light();
	Light(const char* name);
	~Light();

	void render(Camera* camera);
	void renderInMenu();
};

class Skybox : public SceneNode {
public:

	// Used to identify the type of environment
	enum TYPEOFSKYBOX {
		PANORAMA,
		PISA,
		BRIDGE
	};

	int typeOfSkybox;

	// HDRE
	Texture* hdre_level0;
	Texture* hdre_level1;
	Texture* hdre_level2;
	Texture* hdre_level3;
	Texture* hdre_level4;
	Texture* hdre_level5;

	Skybox();
	Skybox(const char* name);
	~Skybox();

	void render(Camera* camera);
	void renderInMenu();
};

class VolumeNode : public SceneNode {
public:
	Matrix44 inverse_model;
	//VolumeMaterial* volume_material; // Para acceder al inverse model desde setUniform -- quizá se puede buscar otra forma
	Volume* volume; // Util si queremos escalar en ImGUI

	VolumeNode();
	VolumeNode(const char* name);
	~VolumeNode();

	void render(Camera* camera);
	void renderInMenu();
};

#endif
#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"


unsigned int SceneNode::lastNameId = 0;
unsigned int Light::lastNameId = 0;
//unsigned int Skybox::lastNameId = 0;

unsigned int mesh_selected = 0;

SceneNode::SceneNode()
{
	this->typeOfNode = int(TYPEOFNODE::NODE);
	this->name = std::string("Node " + std::to_string(lastNameId++));
	this->visible_flag = TRUE;
}


SceneNode::SceneNode(const char * name,Material* material, Mesh* mesh)
{
	this->typeOfNode = int(TYPEOFNODE::NODE);
	this->name = name;
	this->material = material;
	this->mesh = mesh;
	this->visible_flag = TRUE;

}

SceneNode::~SceneNode()
{

}

void SceneNode::render(Camera* camera)
{
	if (material && visible_flag)
		material->render(mesh, model, camera);
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	ImGui::Checkbox("Visible", &visible_flag);
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
		
		ImGui::TreePop();
	}

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		material->renderInMenu();
		ImGui::TreePop();
	}

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0HELMET\0");

		ImGui::TreePop();
	}

	
}

Light::Light()
{
	
	this->visible_flag = TRUE;
	this->name = std::string("Light " + std::to_string(lastNameId++));
	this->typeOfNode = (int)TYPEOFNODE::LIGHT;
	this->ambient_intensity.set(0.7, 0.7, 0.7);
	this->diffuse_intensity.set(0.9, 0.1, 0.9);
	this->specular_intensity.set(0.0, 0.9, 0.0);

}


Light::Light(const char* name)
{
	
	this->name = name;
	this->visible_flag = TRUE;
	this->typeOfNode = (int)TYPEOFNODE::LIGHT;
	this->ambient_intensity.set(0.7, 0.7, 0.7);
	this->diffuse_intensity.set(0.9, 0.1, 0.9);
	this->specular_intensity.set(0.0, 0.9, 0.0);
}

Light::~Light()
{
}

void Light::render(Camera* camera)
{
	if (material&& visible_flag)
		material->render(mesh, model, camera);
}

void Light::renderInMenu()
{
	SceneNode::renderInMenu();

	#ifndef SKIP_IMGUI 
		if (ImGui::TreeNode("Light intensities"))
		{
			ImGui::ColorEdit3("ambient_intensity", (float*)&this->ambient_intensity);
			ImGui::ColorEdit3("diffuse_intensity", (float*)&this->diffuse_intensity);
			ImGui::ColorEdit3("specular_intensity", (float*)&this->specular_intensity);
			ImGui::TreePop();
		}

	#endif
}


Skybox::Skybox()
{
	this->visible_flag = TRUE;
	this->typeOfNode = (int)TYPEOFNODE::SKYBOX;
	this->name = this->name = std::string("Skybox " + std::to_string(lastNameId++));
}

Skybox::Skybox(const char* name)
{
	this->name = name;
	this->visible_flag = TRUE;
	this->typeOfNode = (int)TYPEOFNODE::SKYBOX;
}

Skybox::~Skybox()
{
}

void Skybox::render(Camera* camera)
{
	if (material && this->visible_flag)
		material->render(mesh, model, camera);
}



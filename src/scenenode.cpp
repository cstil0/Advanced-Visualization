#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"


unsigned int SceneNode::lastNameId = 0;
unsigned int Light::lastNameId = 0;

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
	if (! (this->typeOfNode == TYPEOFNODE::LIGHT)) {
		ImGui::Checkbox("Visible", &visible_flag);
	}
	
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

	
	
	//Textures
	if (!(this->typeOfNode == TYPEOFNODE::LIGHT)&& ImGui::TreeNode("Textures"))
	{
		int& output = Application::instance->output;
		ImGui::Combo("Textures", &output, "COMPLETE\0\ALBEDO\0\ROUGHNESS\0\METALNESS\0\NORMAL\0\EMMISIVE\0A_OCC\0LUT");

		ImGui::TreePop();
	}

	//Geometry
	if (!(this->typeOfNode == TYPEOFNODE::LIGHT) &&  mesh && ImGui::TreeNode("Geometry"))
	{
		ImGui::Combo("Mesh", &Application::instance->typeOfModel_ImGUI, "BALL\0HELMET\0LANTERN\0");
		ImGui::TreePop();
	}
}

Light::Light()
{
	this->visible_flag = TRUE;
	this->name = std::string("Light " + std::to_string(lastNameId++));
	this->typeOfNode = (int)TYPEOFNODE::LIGHT;
	this->diffuse_intensity.set(1.0, 1.0, 1.0);
	this->specular_intensity.set(1.0f, 1.0f, 1.0f);
	this->light_intensity = 0.5f;
}


Light::Light(const char* name)
{	
	this->name = name;
	this->visible_flag = TRUE;
	this->typeOfNode = (int)TYPEOFNODE::LIGHT;
	this->diffuse_intensity.set(1.0, 1.0, 1.0);
	this->specular_intensity.set(1.0f, 1.0f, 1.0f);
	this->light_intensity = 0.5f;
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

	int numLight = 0;
	
	if (ImGui::TreeNode("Light intensity") ) 
	{
		ImGui::SliderFloat("Intensity", &this->light_intensity, 0.0f, 10.0f);
		//If is not the mode of PBR, we can anable the following
		//Application::instance-> es un puntero
		if (!( Application::instance->app_mode == Application::APPMODE::PBR)) {

			ImGui::ColorEdit3("Specular_intensity", (float*)&this->specular_intensity);
			ImGui::ColorEdit3("Diffuse_intensity", (float*)&this->diffuse_intensity);
		}
		ImGui::TreePop();
	}
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

void Skybox::renderInMenu()
{
	ImGui::Checkbox("Visible", &visible_flag);
}

VolumeNode::VolumeNode()
{
}

VolumeNode::VolumeNode(const char* name)
{
	this->name = name;
	this->typeOfNode = TYPEOFNODE::VOLUME;
	Matrix44 inv_m_aux = model;
	inv_m_aux.inverse();
	this->inverse_model = inv_m_aux;
}

VolumeNode::~VolumeNode()
{
}

void VolumeNode::render(Camera* camera)
{
	if (material && visible_flag) {
		// Downcast --> con este dynamic cast podemos recuperar las variables propias del VolumeNode si fue creado asi aunque le hayamos hecho un downcast
		int typeOfMaterial = Application::instance->typeOfMaterial_ImGUI;
		// HAY QUE CREAR ENUM (PHONG Y BASIC)
		if (typeOfMaterial == 0) { //Basic Material
			VolumeMaterial* volume_mat = dynamic_cast<VolumeMaterial*>(material);
			volume_mat->render(mesh, model, inverse_model, camera);
			
		}
		else { //Phong
			VolumetricPhong* volume_mat = dynamic_cast<VolumetricPhong*>(material);
			volume_mat->render(mesh, model, inverse_model, camera);
			
		}
	}
}


// ESTAMOS REPITIENDO
void VolumeNode::renderInMenu()
{
	ImGui::Checkbox("Visible", &visible_flag);


	//Model edit
	if (ImGui::TreeNode("Model"))
	{
		// 	COMPROBAR QUE ESTO HACE ALGO
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);

		ImGui::TreePop();
	}

	// Material
	if (ImGui::TreeNode("Material")) {
		material->renderInMenu();
		ImGui::TreePop();
	}

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		ImGui::Combo("Volume", &Application::instance->typeOfVolume_ImGUI, "FOOT\0TEA\0ABDOMEN\0BONSAI\0ORANGE\0");
		ImGui::TreePop();
	}

	// TF generator
	// Downcast
	VolumeMaterial* volume_mat = dynamic_cast<VolumeMaterial*>(this->material);
	if (volume_mat->typeOfMaterial == VolumeMaterial::TYPEOFMATERIAL::BASIC && mesh && ImGui::TreeNode("TF Generator"))
	{
		volume_mat->renderInMenu_TF();
	}

	// Highlight
	if (volume_mat->typeOfMaterial == VolumeMaterial::TYPEOFMATERIAL::BASIC  && mesh && ImGui::TreeNode("Highlight"))
	{
		volume_mat->renderInMenu_highlight();
	}
}

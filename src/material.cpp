#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

StandardMaterial::StandardMaterial()
{
	this->color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::StandardMaterial(Shader* sh, Texture* tex)
{
	this->color = vec4(1.f, 1.f, 1.f, 1.f);
	this->shader = sh;
	this->texture = tex;
}


StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);

	shader->setUniform("u_color", this->color);

	if (texture)
		shader->setUniform("u_texture", texture, EOutput::ALBEDO);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	
}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

PhongMaterial::PhongMaterial()
{
	this->color.set(0.7f, 0.7f, 0.2f, 0.f); //ambient material color
	this->specular.set(1.0f, 1.0f, 0.0f);
	this->diffuse.set(0.0f, 0.0f, 1.0f);
	this->shininess = 20;
}

PhongMaterial::PhongMaterial(Shader* sh, Texture* texture)
{
	this->color.set(0.7f, 0.7f, 0.7f, 0.f); //ambient material color
	this->specular.set(1.0f, 1.0f, 0.0f);
	this->diffuse.set(0.0f, 0.0f, 1.0f);
	this->shininess = 20;
	
	this->shader = sh;
	this->texture = texture;
}

PhongMaterial::~PhongMaterial()
{
}

void PhongMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_color", this->color);

	// Create some variables to store lights information and then fill using a for loop
	std::vector<vec3> light_position;
	std::vector<vec3> light_Id;
	std::vector<vec3> light_Is;

	for (int i = 0; i < this->light_list.size(); i++)
	{
		Light* light = light_list[i];
		light_position.push_back(light->model.getTranslation());
		light_Id.push_back(light->diffuse_intensity);
		light_Is.push_back(light->specular_intensity);

	}

	shader->setUniform("u_light_pos", light_position);
	shader->setUniform("u_Ia", Application::instance->ambient_light); //just one time
	shader->setUniform("u_Id", light_Id);
	shader->setUniform("u_Is", light_Is);

	shader->setUniform("u_specular", specular);
	shader->setUniform("u_diffuse", diffuse);
	shader->setUniform("u_shininess", shininess);

	if (texture)
		shader->setUniform("u_texture", texture, EOutput::ALBEDO);
}




void PhongMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color A. Material", (float*)&this->color); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular", (float*)&this->specular);
	ImGui::ColorEdit3("Diffuse", (float*)&this->diffuse);
	ImGui::SliderFloat("Shininess", (float*)&this->shininess, 1, 50);

}

SkyboxMaterial::SkyboxMaterial()
{
}



SkyboxMaterial::SkyboxMaterial( Shader* sh )
{
	this->shader = sh;
}

SkyboxMaterial::~SkyboxMaterial()
{
}

void SkyboxMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_color", color);

	Skybox* skybox = Application::instance->skybox_node;
	if (skybox->hdre_level0)
		shader->setTexture("u_environment_tex", skybox->hdre_level0, 0);
}

void SkyboxMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);
		
		// We disable the depth test because we want render skybox as the background of the scene
		// And to avoid objects that are behind the cube if we take into account the Zbuffer
		glDisable(GL_DEPTH_TEST);
		
		//do the draw call
		mesh->render(GL_TRIANGLES);

		glEnable(GL_DEPTH_TEST);

		//disable shader
		shader->disable();
	}
}

void SkyboxMaterial::renderInMenu()
{

}

PBRMaterial::PBRMaterial()
{
}

PBRMaterial::PBRMaterial(Shader* sh, Texture* tex, Texture* normal, Texture* rough, Texture* metal, bool bool_mr, Texture* mr_texture) {
	this->shader = sh;
	this->texture = tex;
	this->normal_texture = normal;
	this->roughness_texture = rough;
	this->metalness_texture = metal;
	this->mr_texture = mr_texture;
	
	this->roughness = 0.1;
	this->metalness = 0.4;

	this->bool_met_rou = bool_mr;
}

PBRMaterial::~PBRMaterial()
{
}

void PBRMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_color", color);

	shader->setUniform("u_light_pos", light->model.getTranslation());
	shader->setUniform("u_ambient_light", Application::instance->ambient_light);
	shader->setUniform("u_light_color", light->material->color.xyz );
	shader->setUniform("u_light_intensity", light->light_intensity);

	shader->setUniform("u_output", Application::instance->output);
	shader->setUniform("u_roughness", this->roughness);
	shader->setUniform("u_metalness", this->metalness);
	shader->setUniform("u_met_rou", this->bool_met_rou);

	setTextures();
}

void PBRMaterial::setTextures()
{
	// Generate black and white textures in case a texture is not active
	Texture* black_tex = Texture::getBlackTexture();
	Texture* white_tex = Texture::getWhiteTexture();

	if (texture)
		shader->setTexture("u_texture", texture, EOutput::ALBEDO);
	if (this->normal_texture)
		shader->setTexture("u_normalmap_texture", this->normal_texture, EOutput::NORMALS);
	if (this->roughness_texture)
		shader->setTexture("u_roughness_texture", this->roughness_texture, EOutput::ROUGHNESS);
	if (this->metalness_texture)
		shader->setTexture("u_metalness_texture", this->metalness_texture, EOutput::METALNESS);
	if (this->bool_met_rou && this->mr_texture)
		shader->setTexture("u_mr_texture", this->mr_texture, EOutput::METALNESS_ROUGHNESS);
	
	// If emissive texture is not active we use the black one because it will not add any extra light
	if (this->emissive_texture)
		shader->setTexture("u_emissive_texture", this->emissive_texture, EOutput::EMISSIVE);
	else
		shader->setTexture("u_emissive_texture", black_tex, EOutput::EMISSIVE);
	// If opacity texture is not active we use the white one because it will make the material completely opaque
	if (this->opacity_texture)
		shader->setTexture("u_opacity_texture", this->opacity_texture, EOutput::OPACITY);
	else
		shader->setTexture("u_opacity_texture", white_tex, EOutput::OPACITY);
	// If ambient occlusion texture is not active we use the white one because it will not create any extra shadow
	if (this->ao_texture)
		shader->setTexture("u_ao_texture", this->ao_texture, EOutput::AMBIENT_OCCLUSION);
	else
		shader->setTexture("u_ao_texture", white_tex, EOutput::AMBIENT_OCCLUSION);

	// Get skybox node of the environement and load its HDRE textures
	Skybox* skybox = Application::instance->skybox_node;
	if (skybox->hdre_level0)
		shader->setTexture("u_texture_prem", skybox->hdre_level0, EOutput::LEVEL0);
	if (skybox->hdre_level1)
		shader->setTexture("u_texture_prem_0", skybox->hdre_level1, EOutput::LEVEL1);
	if (skybox->hdre_level2)
		shader->setTexture("u_texture_prem_1", skybox->hdre_level2, EOutput::LEVEL2);
	if (skybox->hdre_level3)
		shader->setTexture("u_texture_prem_2", skybox->hdre_level3, EOutput::LEVEL3);
	if (skybox->hdre_level4)
		shader->setTexture("u_texture_prem_3", skybox->hdre_level4, EOutput::LEVEL4);
	if (skybox->hdre_level5)
		shader->setTexture("u_texture_prem_4", skybox->hdre_level5, EOutput::LEVEL5);
	if (this->BRDFLut)
		shader->setTexture("u_BRDFLut", this->BRDFLut, EOutput::BRDFLut);
}

void PBRMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		// Flags used to apply oppacity map
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE); // active cull face to render only one time every pixel
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		//disable shader
		shader->disable();
	}
}

void PBRMaterial::renderInMenu() 
{
	ImGui::ColorEdit3("Base Color", color.v); // Edit 3 floats representing a color
	ImGui::SliderFloat("Roughness",&this->roughness, 0.0f, 1.0f);
	ImGui::SliderFloat("Metalness", &this->metalness, 0.0f, 1.0f);
}
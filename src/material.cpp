#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 0.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::StandardMaterial(Shader* sh, Texture* tex)
{
	color = vec4(0.4f, 0.5f, 0.5f, 1.f);
	
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
	//shader->setUniform("u_time", Application::instance->time);
	//shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_color", this->color);
	//shader->setUniform("u_exposure", Application::instance->scene_exposure);

	if (texture)
		shader->setUniform("u_texture", texture, EOutput::ALBEDO);
	//if (normal)
	//	shader->setUniform("u_normal", normal);
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
	this->specular.set(0.5f, 0.0f, 0.0f);
	this->diffuse.set(0.0f, 0.0f, 0.5f);
	this->shininess = 20;
}

PhongMaterial::PhongMaterial(Shader* sh, Texture* texture)
{
	this->color.set(0.7f, 0.7f, 0.2f, 0.f); //ambient material color
	this->specular.set(0.5f, 0.0f, 0.0f);
	this->diffuse.set(0.0f, 0.0f, 0.5f);
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

	//We create variables to store lights information and then fill them with a bucle for
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

	//In case of one light
	/*shader->setUniform("u_Ia", this->light->ambient_intensity);
	shader->setUniform("u_Id", this->light->diffuse_intensity);
	shader->setUniform("u_Is", this->light->specular_intensity);
	shader->setUniform("u_light_pos", this->light->model.getTranslation());
	*/

	shader->setUniform("u_light_pos", light_position);
	shader->setUniform("u_Ia", Application::instance->ambient_light); //just one time
	shader->setUniform("u_Id", light_Id);
	shader->setUniform("u_Is", light_Is);

	shader->setUniform("u_specular", specular);
	shader->setUniform("u_diffuse", diffuse);
	shader->setUniform("u_shininess", shininess);

	if (texture)
		shader->setUniform("u_texture", texture, EOutput::ALBEDO);
	//if (normal)
	//	shader->setUniform("u_normal", normal);
}


void PhongMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
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

void PhongMaterial::renderInMenu()
{

	ImGui::ColorEdit3("Color A. Material", (float*)&this->color); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Specular", (float*)&this->specular);
	ImGui::ColorEdit3("Diffuse", (float*)&this->diffuse);
	ImGui::SliderFloat("Shininess", (float*)&this->shininess, 0.1, 50);

}

SkyboxMaterial::SkyboxMaterial()
{
}

SkyboxMaterial::SkyboxMaterial( Shader* sh, Texture* tex )
{
	this->shader = sh;
	this->texture = tex;
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
	
	if (texture)
		shader->setUniform("u_texture", texture, EOutput::ALBEDO);
}

void SkyboxMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);
		
		//We disable the depth test because we want render skybox as the background of the scene
		// And to avoid objects that is hebind the cube if we have into account the Zbuffer
		
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


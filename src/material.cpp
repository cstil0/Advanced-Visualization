#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

enum class TextureType {
	Material,
	CUBEMAP
};


//subclass-1----------------------------------------------

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
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
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_output", Application::instance->output);

	shader->setUniform("u_color", color);
	shader->setUniform("u_exposure", Application::instance->scene_exposure);

	if (texture)
		shader->setTexture("u_texture", texture, ALBEDO);
	
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

//subclass-2----------------------------------------------

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

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //?s
	}
}

//subclass-3----------------------------------------------

PhongMaterial::PhongMaterial() { //calculate with phong ecuation
	
	this->color = vec4(1.f, 1.f, 1.f, 1.f); //color of the light
	this->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/light.fs");
	
	this->diffuse.set(1.0f, 1.0f, 1.0f);
	this->specular.set(1.0f, 1.0f, 1.0f);
	this->shininess = 0.5;
	this->ambient_intensity.set(1.0, 1.0, 1.0);
	this->diffuse_intensity.set(1.0, 1.0, 1.0);
	this->specular_intensity.set(1.0, 1.0, 1.0);
}

PhongMaterial::~PhongMaterial()
{
}


void PhongMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms

	shader->setUniform("u_viewprojection", camera->viewprojection_matrix); 
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	//shader->setUniform("u_time", Application::instance->time);
	//shader->setUniform("u_output", Application::instance->output);

	//shader->setUniform("u_color", color);
	//shader->setUniform("u_exposure", Application::instance->scene_exposure);

	shader->setUniform("u_diffuse", this->diffuse);
	shader->setUniform("u_specular", this->specular);
	shader->setUniform("u_shininess", this->shininess);

	
	shader->setUniform("u_light_pos", light->model.getTranslation());
	shader->setUniform("u_Ia", this->ambient_intensity);
	shader->setUniform("u_Id", this->diffuse_intensity);
	shader->setUniform("u_Is", this->specular_intensity);

	if (texture)
		shader->setTexture("u_texture", texture, ALBEDO);


}


void PhongMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (!mesh && !shader && !light)
		return;

	//enable shader
	shader->enable();

	
	//upload uniforms
	setUniforms(camera, model);


	//do the draw call
	//glFrontFace(GL_CCW);
	mesh->render(GL_TRIANGLES);
	
	
	//disable shader
	shader->disable();
	
}

void PhongMaterial::renderInMenu() 
{
	ImGui::ColorEdit3("Material Color", (float*)&this->color);
	ImGui::ColorEdit3("Specular", (float*)&this->specular);
	ImGui::ColorEdit3("Diffuse", (float*)&this->diffuse);
	ImGui::SliderFloat("Shininess", (float*)&this->shininess, 0, 50);

}

SkyboxMaterial::SkyboxMaterial()
{
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");

}

SkyboxMaterial::~SkyboxMaterial()
{
}

void SkyboxMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	//shader->setUniform("u_model", model);
	// in case to update every time that the camera moves...
	model.translate(camera->eye.x, camera->eye.y, camera->eye.z);
	shader->setUniform("u_model", model);
	//model.scale(1, 1, 1);
	if (texture)
		shader->setTexture("u_texture_cube", texture, int(TextureType::CUBEMAP)); // poner el slot de textura!!!
	

}

void SkyboxMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (!mesh && !shader)
		return;

	//enable shader
	shader->enable();

	//upload uniforms
	setUniforms(camera, model);

	
	glDisable(GL_DEPTH_TEST);
	//do the draw call
	//glFrontFace(GL_CCW);
	//glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);
	mesh->render(GL_TRIANGLES);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//disable shader
	shader->disable();

}

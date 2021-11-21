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

VolumeMaterial::VolumeMaterial()
{
}

VolumeMaterial::VolumeMaterial(Shader* sh, Texture* tex, vec4 d_lim, vec4 TF_fst_col, vec4 TF_snd_col, vec4 TF_trd_col, vec4 TF_frth_col)
{
	this->shader = sh;
	this->texture = tex;
	this->length_step = 0.06f;// cambiando a un valor mas pequeño
	this->density_threshold_min = 0.0f;
	this->density_threshold_max = 1.0f;
	this->brightness = 5.0f;
	
	jittering_flag = false;
	TF_flag = false;
	TF_debug_flag = false;
	illumination_flag = false;
	clipping_flag = false;

	jittering_flag_imgui = false;
	TF_flag_imgui = false;
	TF_debug_flag_imgui = false;
	illumination_flag_imgui = false;
	clipping_flag_imgui = false;

	this->noise_texture = Texture::Get("data/blueNoise.png");
	this->threshold_plane = 0.0f;

	density_limits = d_lim;
	TF_first_color = TF_fst_col;
	TF_second_color = TF_snd_col;
	TF_third_color = TF_trd_col;
	TF_forth_color = TF_frth_col;

	save_texture = false;

	//this->plane_a = 0.0f;
	//this->plane_b = 0.0f;
	//this->plane_c = 0.0f;
	//this->plane_d = 0.0f;
}

VolumeMaterial::~VolumeMaterial()
{
}

void VolumeMaterial::setUniforms(Camera* camera, Matrix44 model, Matrix44 inverse_model)
{
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_inverse_model", inverse_model);
	shader->setUniform("u_color", color);

	shader->setUniform("u_length_step", length_step);
	shader->setUniform("u_threshold_d_min", density_threshold_min);
	shader->setUniform("u_threshold_d_max", density_threshold_max);
	shader->setUniform("u_brightness", this->brightness);
	shader->setUniform("u_threshold_plane", this->threshold_plane);

	shader->setUniform("u_density_limits", this->density_limits);
	shader->setUniform("u_tf_fst_color", this->TF_first_color);
	shader->setUniform("u_tf_snd_color", this->TF_second_color);
	shader->setUniform("u_tf_trd_color", this->TF_third_color);
	shader->setUniform("u_tf_frth_color", this->TF_forth_color);

	//shader->setUniform("u_plane_a", this->plane_a);
	//shader->setUniform("u_plane_b", this->plane_b);
	//shader->setUniform("u_plane_c", this->plane_c);
	//shader->setUniform("u_plane_d", this->plane_d);

	if (texture)
		shader->setTexture("u_texture", texture, 0); // poner offset para numerar las texturas
	if(noise_texture)
		shader->setTexture("u_noise_texture", noise_texture, 1);
	if (tf_mapping_texture)
		shader->setTexture("u_tf_mapping_texture", tf_mapping_texture, 2);
}

void VolumeMaterial::render(Mesh* mesh, Matrix44 model, Matrix44 inverse_model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model, inverse_model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void VolumeMaterial::saveTexture()
{
	int TF_width = 20;
	int TF_height = 1;

	Image* TF_texture = new Image(TF_width, TF_height);
	// POR ALGUN MOTIVO NO ME FUNCIONA LA DIVISIÓN
	float step = 0.05;
	float position = 0.0f;
	// Iterate through the image to set the pixels
	//for (int i = 0; i < TF_width; i++) {
	//	// Check between which limits the pixel is
	//	position = (i + 1) * step;
	//	Color c = Color(255, 0, 0, 100);
	//	TF_texture->setPixel(i, 0, c);
	//}
	for (int i = 0; i < TF_width; i++) {
		// Check between which limits the pixel is
		position = (i+1) * step;
		if (position <= density_limits.x) {
			Color c = Color(TF_first_color.x*255, TF_first_color.y*255, TF_first_color.z*255, TF_first_color.w);
			TF_texture->setPixel(i, 0, c);
		}
		else if (position <= density_limits.y) {
			TF_texture->setPixel(i, 0, Color(TF_second_color.x * 255, TF_second_color.y * 255, TF_second_color.z * 255, TF_second_color.w));
		}
		else if (position <= density_limits.z) {
			TF_texture->setPixel(i, 0, Color(TF_third_color.x * 255, TF_third_color.y * 255, TF_third_color.z * 255, TF_third_color.w));
		}
		else if (position <= density_limits.w) {
			TF_texture->setPixel(i, 0, Color(TF_forth_color.x * 255, TF_forth_color.y * 255, TF_forth_color.z * 255, TF_forth_color.w));
		}
		//TF_texture->setPixel(i,0, Color(255.0f,255.0f,255.0f,1.0f));
	}

	TF_texture->saveTGA("data/TF_texture.tga", true);
	// Load the texture again to show the updated one
	this->tf_mapping_texture = new Texture(TF_texture);
	//this->tf_mapping_texture
	save_texture = false;
}

void VolumeMaterial::renderInMenu()
{
	ImGui::SliderFloat("Length Step", &this->length_step, 0.001, 1);
	ImGui::SliderFloat("Clipping Plane", &threshold_plane, -1.0f, 0.0f);
	ImGui::SliderFloat("Brightness", &this->brightness, 1.0f, 50.0f);
	ImGui::ColorEdit3("Color", color.v); 
	ImGui::Checkbox("Jittering", &jittering_flag_imgui);
	ImGui::Checkbox("Transfer function", &TF_flag_imgui);
	ImGui::Checkbox("Clipping", &clipping_flag_imgui);
	ImGui::Checkbox("Illumination", &illumination_flag_imgui);
}

void VolumeMaterial::renderInMenu_TF(){
	ImGui::Checkbox("Debug TF", &TF_debug_flag_imgui);
	ImGui::SliderFloat("MAX Density threshold", &this->density_threshold_max, 0.0, 1);
	ImGui::SliderFloat4("Density limits", &density_limits.x, 0.0f, 1.0f);
	ImGui::ColorEdit3("First Color", TF_first_color.v); 
	ImGui::ColorEdit3("Second Color", TF_second_color.v);
	ImGui::ColorEdit3("Third Color", TF_third_color.v);
	ImGui::ColorEdit3("Forth Color", TF_forth_color.v);
	ImGui::Checkbox("Save TF texture", &save_texture);
}

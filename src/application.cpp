#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "extra/imgui/imgui.h"
#include "extra/imgui/imgui_impl_sdl.h"
#include "extra/imgui/imgui_impl_opengl3.h"

#include <cmath>

bool render_wireframe = false;
Camera* Application::camera = nullptr;
Application* Application::instance = NULL;

Shader* sh = NULL;
unsigned int LEVEL = 0;

Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	scene_exposure = 1;
	output = 0.0;

	//define the color of the ambient as a global variable since it is a property of the scene
	ambient_light.set(0.1, 0.2, 0.3);

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(5.f, 5.f, 5.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	{
		// Load the different environments that we can choose in the imGUI
		loadSkybox_Pisa();
		loadSkybox_Panorama();
		loadSkybox_Bridge();

		//create a light
		int numb_lights = 1;
		Light* light = new Light();
		for (int i = 0; i < numb_lights; i++) {
			light->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");

			StandardMaterial* l_mat = new StandardMaterial();
			light->material = l_mat;
			light->model.translate(-0.4, 1.9, 2.0);
			light->model.scale(0.1, 0.1, 0.1);
			node_list.push_back(light);
		}

		Texture* BRDFLut = Texture::Get("data/brdfLUT.png");

		// Load the different models that we can choose in the imGUI
		loadBall(light, BRDFLut);
		loadHelmet(light, BRDFLut);
		loadLantern(light, BRDFLut);

		// Save a pointer that will update according to the option selected in the imGUI
		SceneNode* current_node = optional_node_list[0];
		// Update the selected option of the imGUI
		typeOfModel_ImGUI = optional_node_list[0]->typeOfModel;
		// Add the current node to the rendering list of nodes
		node_list.push_back(current_node);


		// Functions used in the phong lab
		//renderPhongEquation();

		//renderReflection();
		
	}
	
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

void Application::loadBall(Light* light, Texture* BRDFLut){
	// Load textures
	Texture* ball_color_texture = Texture::Get("data/models/ball/albedo.png");
	Texture* ball_normalmap_texture = Texture::Get("data/models/ball/normal.png");
	Texture* ball_roughness_texture = Texture::Get("data/models/ball/roughness.png");
	Texture* ball_metalness_texture = Texture::Get("data/models/ball/metalness.png");

	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");

	// Create material
	PBRMaterial* ball_material = new PBRMaterial(sh, ball_color_texture, ball_normalmap_texture, ball_roughness_texture, ball_metalness_texture, false, NULL);
	ball_material->BRDFLut = BRDFLut;
	SceneNode* ball_node = new SceneNode("BallPBR", ball_material, Mesh::Get("data/models/ball/sphere.obj.mbin"));
	// Add the direct light to the material
	ball_material->light = light;
	ball_node->typeOfModel = SceneNode::TYPEOFMODEL::BASIC;
	ball_node->material = ball_material;
	// Add the node to the possible nodes that can be selected in the imGUI
	optional_node_list.push_back(ball_node);
}

void Application::loadHelmet(Light* light, Texture* BRDFLut) {
	// Load textures
	Texture* helmet_color_texture = Texture::Get("data/models/helmet/albedo.png");
	Texture* helmet_normalmap_texture = Texture::Get("data/models/helmet/normal.png");
	Texture* helmet_mr_texture = Texture::Get("data/models/helmet/roughness.png");
	Texture* helmet_e_texture = Texture::Get("data/models/helmet/emissive.png");

	PBRMaterial* helmet_material = new PBRMaterial(sh, helmet_color_texture, helmet_normalmap_texture, NULL, NULL, true, helmet_mr_texture);

	helmet_material->BRDFLut = BRDFLut;
	helmet_material->bool_ao = TRUE; // Activate ao texture
	helmet_material->ao_texture = Texture::Get("data/models/helmet/ao.png");
	helmet_material->bool_em = TRUE;// Activate emissive texture
	helmet_material->emissive_texture = Texture::Get("data/models/helmet/emissive.png");

	SceneNode* helmet_node = new SceneNode("HelmetPBR2", helmet_material, Mesh::Get("data/models/helmet/helmet.obj.mbin"));
	helmet_node->model.setScale(2.0f, 2.0f, 2.0f);
	helmet_material->light = light;
	helmet_node->material = helmet_material;
	helmet_node->material = helmet_material;
	helmet_node->typeOfModel = SceneNode::TYPEOFMODEL::HELMET;
	optional_node_list.push_back(helmet_node);
}

void Application::loadLantern(Light* light, Texture* BRDFLut) {
	Texture* lantern_color_texture = Texture::Get("data/models/lantern/albedo.png");
	Texture* lantern_normalmap_texture = Texture::Get("data/models/lantern/normal.png");
	Texture* lantern_roughness_texture = Texture::Get("data/models/lantern/roughness.png");
	Texture* lantern_metalness_texture = Texture::Get("data/models/lantern/metalness.png");

	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");

	PBRMaterial* lantern_material = new PBRMaterial(sh, lantern_color_texture, lantern_normalmap_texture, lantern_roughness_texture, lantern_metalness_texture, false);
	lantern_material->BRDFLut = BRDFLut;
	lantern_material->bool_ao = TRUE; // Activate ao texture
	lantern_material->ao_texture = Texture::Get("data/models/lantern/ao.png");
	lantern_material->bool_opacity = TRUE;// Activate opacity texture
	lantern_material->opacity_texture = Texture::Get("data/models/lantern/opacity.png");

	SceneNode* lantern_node = new SceneNode("LanternPBR3", lantern_material, Mesh::Get("data/models/lantern/lantern.obj.mbin"));
	lantern_node->model.setScale(0.05, 0.05, 0.05);
	lantern_material->light = light;
	lantern_node->material = lantern_material;
	//pbr_node->material = pbr_material;
	lantern_node->typeOfModel = SceneNode::TYPEOFMODEL::LANTERN;
	optional_node_list.push_back(lantern_node);
}

void Application::loadSkybox_Pisa() {
	Skybox* skybox_pisa = new Skybox("Skybox");
	skybox_pisa->mesh = Mesh::getCube();
	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
	SkyboxMaterial* sky_mat_pisa = new SkyboxMaterial(sh);

	HDRE* hdre_pisa = new HDRE();
	if (hdre_pisa->load("data/environments/pisa.hdre"))
		hdre_pisa = HDRE::Get("data/environments/pisa.hdre");

	skybox_pisa->typeOfNode = SceneNode::TYPEOFNODE::SKYBOX;

	skybox_pisa->hdre_level0 = new Texture();
	skybox_pisa->hdre_level1 = new Texture();
	skybox_pisa->hdre_level2 = new Texture();
	skybox_pisa->hdre_level3 = new Texture();
	skybox_pisa->hdre_level4 = new Texture();
	skybox_pisa->hdre_level5 = new Texture();

	skybox_pisa->hdre_level0->cubemapFromHDRE(hdre_pisa, 0);
	skybox_pisa->hdre_level1->cubemapFromHDRE(hdre_pisa, 1);
	skybox_pisa->hdre_level2->cubemapFromHDRE(hdre_pisa, 2);
	skybox_pisa->hdre_level3->cubemapFromHDRE(hdre_pisa, 3);
	skybox_pisa->hdre_level4->cubemapFromHDRE(hdre_pisa, 4);
	skybox_pisa->hdre_level5->cubemapFromHDRE(hdre_pisa, 5);

	// Set the principal texture to the skybox
	sky_mat_pisa->texture = skybox_pisa->hdre_level0;

	skybox_pisa->material = sky_mat_pisa;
	optional_skybox_list.push_back(skybox_pisa);
	skybox_pisa->typeOfSkybox = Skybox::TYPEOFSKYBOX::PISA;
}

void Application::loadSkybox_Panorama()
{
	//---SkyboxNode---
	//We first create a skybox node and we need to render it first, since it needs a specific material
	//We create a SkyboxMaterial to save all its corresponding information (shader, cubemap),  
	//then we pass this material to the skybox node, and finally push it in the list of nodes.

	Skybox* skybox_panorama = new Skybox("Skybox");
	skybox_panorama->mesh = Mesh::getCube();
	SkyboxMaterial* sky_mat_panorama = new SkyboxMaterial(sh);

	HDRE* hdre_panorama = new HDRE();
	if(hdre_panorama->load("data/environments/panorama.hdre"))
		hdre_panorama = HDRE::Get("data/environments/panorama.hdre");

	skybox_panorama->typeOfNode = SceneNode::TYPEOFNODE::SKYBOX;

	skybox_panorama->hdre_level0 = new Texture();
	skybox_panorama->hdre_level1 = new Texture();
	skybox_panorama->hdre_level2 = new Texture();
	skybox_panorama->hdre_level3 = new Texture();
	skybox_panorama->hdre_level4 = new Texture();
	skybox_panorama->hdre_level5 = new Texture();

	skybox_panorama->hdre_level0->cubemapFromHDRE(hdre_panorama, 0);
	skybox_panorama->hdre_level1->cubemapFromHDRE(hdre_panorama, 1);
	skybox_panorama->hdre_level2->cubemapFromHDRE(hdre_panorama, 2);
	skybox_panorama->hdre_level3->cubemapFromHDRE(hdre_panorama, 3);
	skybox_panorama->hdre_level4->cubemapFromHDRE(hdre_panorama, 4);
	skybox_panorama->hdre_level5->cubemapFromHDRE(hdre_panorama, 5);

	sky_mat_panorama->texture = skybox_panorama->hdre_level0;

	skybox_panorama->material = sky_mat_panorama;
	optional_skybox_list.push_back(skybox_panorama);
	skybox_panorama->typeOfSkybox = Skybox::TYPEOFSKYBOX::PANORAMA;
}

void Application::loadSkybox_Bridge() {
	Skybox* skybox_bridge = new Skybox("Skybox");
	skybox_bridge->mesh = Mesh::getCube();
	SkyboxMaterial* sky_mat_bridge = new SkyboxMaterial(sh);

	HDRE* hdre_bridge = new HDRE();
	if (hdre_bridge->load("data/environments/san_giuseppe_bridge.hdre"))
		hdre_bridge = HDRE::Get("data/environments/san_giuseppe_bridge.hdre");

	skybox_bridge->typeOfNode = SceneNode::TYPEOFNODE::SKYBOX;

	skybox_bridge->hdre_level0 = new Texture();
	skybox_bridge->hdre_level1 = new Texture();
	skybox_bridge->hdre_level2 = new Texture();
	skybox_bridge->hdre_level3 = new Texture();
	skybox_bridge->hdre_level4 = new Texture();
	skybox_bridge->hdre_level5 = new Texture();

	skybox_bridge->hdre_level0->cubemapFromHDRE(hdre_bridge, 0);
	skybox_bridge->hdre_level1->cubemapFromHDRE(hdre_bridge, 1);
	skybox_bridge->hdre_level2->cubemapFromHDRE(hdre_bridge, 2);
	skybox_bridge->hdre_level3->cubemapFromHDRE(hdre_bridge, 3);
	skybox_bridge->hdre_level4->cubemapFromHDRE(hdre_bridge, 4);
	skybox_bridge->hdre_level5->cubemapFromHDRE(hdre_bridge, 5);

	sky_mat_bridge->texture = skybox_bridge->hdre_level0;

	skybox_bridge->material = sky_mat_bridge;
	optional_skybox_list.push_back(skybox_bridge);
	skybox_bridge->typeOfSkybox = Skybox::TYPEOFSKYBOX::BRIDGE;

	skybox_node = skybox_bridge;
	typeOfSkybox_ImGUI = Skybox::TYPEOFSKYBOX::BRIDGE;
	node_list.push_back(skybox_node);
}

void Application::renderPhongEquation()
{
	//---PhongImplementation---
		// Following the same flow as before, we create a ball node, and its attribute material,
		// we create the PhongMaterial with its corresponding parameters.
		// Also we want to represent the light node in the scene, so we create it and pass the information  
		// to its StanddrdMaterial, since in this case, it just represent the node light that affect our ball
		// And we can have the attribute light_node_list in the PhongMaterial to have acess of light's information
		// Finally, when every information is passed, we push both nodes in the list.
	int numb_lights = 2;
	for (int i = 0; i < numb_lights; i++) {
		Light* light = new Light();
		light->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");

		StandardMaterial* l_mat = new StandardMaterial();
		light->material = l_mat;

		light->model.translate(-2 + 4 * i, 2 * i, 2 * i);
		light->model.scale(0.1, 0.1, 0.1);
		node_list.push_back(light);
	}

	//Create a ball rendered with Phong approximation
	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/light.fs");
	Texture* node_texture = Texture::Get("data/models/ball/albedo.png");

	PhongMaterial* pm = new PhongMaterial(sh, node_texture);

	//Since in this case we want have multiple lights we use a singlePass.
	//We pass all the lights inf. to the shader, and compute the total light intensity there.
	//Therefore, we need to check the list of nodes, downcast if is a light node, and save it
	//into the attribute light_list of the node we just created.
	for (int i = 0; i < node_list.size(); i++)
	{
		if (node_list[i]->typeOfNode == SceneNode::TYPEOFNODE::LIGHT) {
			Light* current_light = (Light*)node_list[i];
			pm->light_list.push_back(current_light);
		}
	}

	SceneNode* node = new SceneNode("NodeWithLight", pm, Mesh::Get("data/meshes/sphere.obj.mbin"));
	node_list.push_back(node);
}

void Application::renderReflection()
{

	//---Reflection---
	// We create a reflected node and it's corresponfing material, since in this case
	// we don't need modify the functions (render and setUniforms), we can reuse StandardMaterial 
	// to organize this architecture. Then, we push it to the node_list.	
	Texture* reflecting_texture = new Texture();
	reflecting_texture->cubemapFromImages("data/environments/snow");
	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/reflection.fs");
	StandardMaterial* reflecting_mat = new StandardMaterial(sh, reflecting_texture);

	SceneNode* reflecting_node = new SceneNode("NodeWithReflection", reflecting_mat, Mesh::Get("data/meshes/sphere.obj.mbin"));

	reflecting_node->material = reflecting_mat;
	reflecting_node->model.scale(2, 2, 2);
	reflecting_node->model.setTranslation(-3, 0, 3);

	node_list.push_back(reflecting_node);

	//---Texture node without ilumination---
	Texture* texture = Texture::Get("data/models/ball/brick_disp.png");
	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	StandardMaterial* material = new StandardMaterial(sh, texture);

	SceneNode* texture_node = new SceneNode("NodeWithoutLight", material, Mesh::Get("data/meshes/sphere.obj.mbin"));

	texture_node->model.scale(2, 2, 2);
	texture_node->model.setTranslation(3, 0, -3);
	node_list.push_back(texture_node);

}


//what to do when the image has to be draw
void Application::render(void)
{
	//set the clear color (the background color)
	glClearColor(.1,.1,.1, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);


	for (size_t i = 0; i < node_list.size(); i++) {
		node_list[i]->render(camera);

		if(render_wireframe)
			node_list[i]->renderWireframe(camera);
	}

	//Draw the floor grid
	if(render_debug)
		drawGrid();
}

void Application::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * 10; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 1;
	
	//example
	float angle = seconds_elapsed * 10.f * DEG2RAD;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT && !ImGui::IsAnyWindowHovered() 
		&& !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())) //is left button pressed?
	{
		camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
	}

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move fast er with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f,  1.0f, 0.0f) * speed);

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	// Update the model according to the imGUI
	// Node_list[2] corresponds to the principal node IGUAL SE PODRÍA PONER MEJOR
	if (typeOfModel_ImGUI != node_list[SceneNode::TYPEOFNODE::NODE]->typeOfModel) {
		//SceneNode* principal_node = node_list[2];
		// Look for the new node to be rendered
		for (int i = 0; i < optional_node_list.size(); i++) {
			if (optional_node_list[i]->typeOfModel == typeOfModel_ImGUI)
				node_list[SceneNode::TYPEOFNODE::NODE] = optional_node_list[i];
		}
	}

	// Same with skybox
	if (typeOfSkybox_ImGUI != skybox_node->typeOfSkybox) {
		// Look for the new skybox to be rendered
		for (int i = 0; i < optional_skybox_list.size(); i++) {
			if (optional_skybox_list[i]->typeOfSkybox == typeOfSkybox_ImGUI) {
				// Update the new skybox
				if (typeOfSkybox_ImGUI == 2) {
					int s = 0;
				}
				node_list[SceneNode::TYPEOFNODE::SKYBOX] = (SceneNode*)optional_skybox_list[i];
				skybox_node = optional_skybox_list[i];
			}
		}
	}

	// Update skybox center position according to the camera position
	if(node_list[SceneNode::TYPEOFNODE::SKYBOX]->typeOfNode == (int)SceneNode::TYPEOFNODE::SKYBOX )
		node_list[SceneNode::TYPEOFNODE::SKYBOX]->model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
}

//Keyboard event handler (sync input)
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: render_debug = !render_debug; break;
		case SDLK_F2: render_wireframe = !render_wireframe; break;
		case SDLK_F5: Shader::ReloadAll(); break; 
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event)
{
}

void Application::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Application::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
		case SDL_MOUSEWHEEL:
		{
			if (event.x > 0) io.MouseWheelH += 1;
			if (event.x < 0) io.MouseWheelH -= 1;
			if (event.y > 0) io.MouseWheel += 1;
			if (event.y < 0) io.MouseWheel -= 1;
		}
	}

	if(!ImGui::IsAnyWindowHovered() && event.y)
		camera->changeDistance(event.y * 0.5);
}

void Application::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

void Application::onFileChanged(const char* filename)
{
	Shader::ReloadAll();
}

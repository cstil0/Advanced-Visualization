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
	output = 0;

	//define the color of the ambient as a global variable since it is a property of the scene
	ambient_light.set(0.7, 0.7, 0.7);

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(5.f, 5.f, 5.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	{
		
		//renderSkybox();

		//renderPhongEquation();

		//renderReflection();
		

		Texture* color_texture = Texture::Get("data/models/ball/albedo.png");
		Texture* normalmap_texture = Texture::Get("data/models/ball/normal.png");
		Texture* roughness_texture = Texture::Get("data/models/ball/roughness.png");
		Texture* metalness_texture = Texture::Get("data/models/ball/metalness.png");

		sh = Shader::Get("data/shaders/basic.vs", "data/shaders/pbr.fs");
		PBRMaterial* pbr_material = new PBRMaterial(sh, color_texture, normalmap_texture, roughness_texture, metalness_texture);
		
		SceneNode* pbr_node = new SceneNode("BallPBR", pbr_material, Mesh::Get("data/meshes/sphere.obj.mbin"));
		node_list.push_back(pbr_node);



	}
	
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

void Application::renderSkybox()
{
	//---SkyboxNode---
	//We first create a skybox node and we need to render it first, since it needs a specific material
	//We create a SkyboxMaterial to save all its corresponding information (shader, cubemap),  
	//then we pass this material to the skybox node, and finally push it in the list of nodes.
	Skybox* skybox_node = new Skybox("Skybox");
	sh = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
	Texture* cubemap_texture = new Texture();
	cubemap_texture->cubemapFromImages("data/environments/snow");
	skybox_node->mesh = Mesh::getCube();

	SkyboxMaterial* sky_mat = new SkyboxMaterial(sh, cubemap_texture);
	skybox_node->material = sky_mat;
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
	//if(render_debug)
		//drawGrid();
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

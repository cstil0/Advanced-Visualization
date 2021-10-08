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

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(5.f, 5.f, 5.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	{
		//---SkyboxNode---
		//We first create a skybox node and we need to render it first, since it need a specific material
		//We create a SkyboxMaterial to save all the corresponding information (shader, cubemap) that it need,  
		//then we pass this material to to skybox node, and finally push it in the list of nodes.
		Skybox* skybox_node = new Skybox();
		sh = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
		Texture* cubemap_texture = new Texture();
		cubemap_texture->cubemapFromImages("data/environments/snow");
		skybox_node->mesh = Mesh::getCube();
		
		SkyboxMaterial* sky_mat = new SkyboxMaterial(sh, cubemap_texture);
		skybox_node->material = sky_mat;
		node_list.push_back(skybox_node);

		//---PhongEcuation---
		// Following the same flow like before, we create a ball node, and its attribute material,
		// we create the PhongMaterial with corresponding parameters, like shader, and texture.
		// Also we want to represent the node light in the scene, so we create it and pass the information  
		// to its StanddrdMaterial, since in this case, it just represent the node light that affect our ball
		// And we can have a attribute light node in the PhongMaterial to have acess of light's information 
		// (position, colors, etc.)
		//And finally, when every information is passed, we push both nodes in the list.
		int numb_lights = 2;
		for (int i = 0; i < numb_lights; i++) {
			Light* light = new Light();
			light->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");

			StandardMaterial* l_mat = new StandardMaterial();
			light->material = l_mat;
			//light->model.setTranslation((position.x+10)*i, position.y*i, position.z*i);
			light->model.translate(3*i, 2, -3*i);
			light->model.scale(0.05,0.05,0.05);
			node_list.push_back(light);
		}

		SceneNode* node = new SceneNode("Ball");
		node->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");
		vec3 position = node->model.getTranslation();
		node->model.translate(position.x, position.y, position.z);

		sh = Shader::Get("data/shaders/basic.vs", "data/shaders/light.fs");
		Texture* node_texture = Texture::Get("data/models/ball/albedo.png");

		PhongMaterial* pm = new PhongMaterial(sh, node_texture);

		for (int i = 0; i < node_list.size(); i++)
		{
			
			if (node_list[i]->typeOfNode == SceneNode::TYPEOFNODE::LIGHT) {
				Light* current_light = (Light*)node_list[i];
				pm->light_list.push_back(current_light);
			}
		}
		
		node->material = pm;
		node_list.push_back(node);

		//---Reflection---
		// We create a reflected node and it's corresponfing material, since in this case
		//---. ME HE QUEDADO AUI!!!

		SceneNode* reflecting_node = new SceneNode("Reflection");
		reflecting_node->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");
		
		Texture* reflecting_texture = new Texture();
		reflecting_texture->cubemapFromImages("data/environments/snow");
		sh = Shader::Get("data/shaders/basic.vs", "data/shaders/reflection.fs");
		StandardMaterial* reflecting_mat = new StandardMaterial(sh, reflecting_texture);
		
		reflecting_node->material = reflecting_mat;
		reflecting_node->model.setTranslation(-3, 0, 3);
		reflecting_node->model.scale(2, 2, 2);
		node_list.push_back(reflecting_node);


	}
	
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
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
	/*for (int i = 0; i < root.size(); i++) {
		root[i]->model.rotate(angle, Vector3(0,1,0));
	}*/

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

	// Update skybox position according to the camera
	
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

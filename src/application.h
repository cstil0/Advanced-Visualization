/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "includes.h"
#include "camera.h"
#include "utils.h"
#include "scenenode.h"

enum EOutput {
	COMPLETE,
	ALBEDO,
	ROUGHNESS,
	METALNESS,
	NORMALS,
	METALNESS_ROUGHNESS,
	EMISSIVE,
	OPACITY,
	AMBIENT_OCCLUSION,
	LEVEL0,
	LEVEL1,
	LEVEL2,
	LEVEL3,
	LEVEL4,
	LEVEL5,
	BRDFLut
};

enum ESkybox {
	PANORAMA,
	PISA

};
class Application
{
public:
	static Application* instance;

	enum TYPEOFMODELIMGUI {
		BASIC,
		HELMET,
		LANTERN
	};

	enum TYPEOFSKYBOXIMGUI {
		PANORAMA,
		PISA,
		BRIDGE
	};

	int typeOfModel_ImGUI;
	int typeOfSkybox_ImGUI;

	std::vector< SceneNode* > node_list;
	// List that saves the nodes that can be selected in the imGUI
	std::vector< SceneNode* > optional_node_list;
	Skybox* skybox_node = NULL;
	// List that saves the types of skybox that can be selected in the imGUI
	std::vector< Skybox* > optional_skybox_list;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;
	bool must_exit;
	bool render_debug;
	float scene_exposure;
	
	int output;
	vec3 ambient_light;
	
	//some vars
	static Camera* camera; //our GLOBAL camera
	bool mouse_locked; //tells if the mouse is locked (not seen)

	Application( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void update( double dt );

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);
	void onFileChanged(const char* filename);

	//functions to create nodes in the scene
	void loadBall(Light* light, Texture* BRDFLUT);
	void loadHelmet(Light* light, Texture* BRDFLUT);
	void loadLantern(Light* light, Texture* BRDFLUT);
	void loadSkybox_Pisa();
	void loadSkybox_Panorama();
	void loadSkybox_Bridge();
	void renderPhongEquation();
	void renderReflection();
};


#endif 
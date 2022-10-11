#include <SDL.h>
#include "SceneManager.h"
#include "EngineManager.h"

#include "Window.h"
#include "Scene0.h"
#include "Scene4.h"
#include "Scene5.h"

SceneManager::SceneManager(): 
	currentScene(nullptr), window(nullptr) {
	Debug::Info("Starting the SceneManager", __FILE__, __LINE__);
}

SceneManager::~SceneManager() {
	if (currentScene) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
	}

	if (window) {
		delete window;
		window = nullptr;
	}
	Debug::Info("Deleting the SceneManager", __FILE__, __LINE__);
}

bool SceneManager::Initialize(std::string name_, int width_, int height_) {

	window = new Window();
	if (!window->OnCreate(name_, width_, height_)) {
		Debug::FatalError("Failed to initialize Window object", __FILE__, __LINE__);
		return false;
	}

	/********************************   Default first scene   ***********************/
	BuildNewScene(SCENE_NUMBER::SCENE5);
	
	return true;
}

/// This is the whole game
void SceneManager::Run() {
	currentScene->Update(EngineManager::Instance()->GetTimer()->GetDeltaTime());
	currentScene->Render();
	HandleEvents();
	SDL_GL_SwapWindow(window->getWindow());
	SDL_Delay(EngineManager::Instance()->GetTimerSleepTime());
}

void SceneManager::HandleEvents() {
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) {
		if (sdlEvent.type == SDL_EventType::SDL_QUIT) {
			EngineManager::Instance()->SetIsRunning(false);
			return;
		}
		else if (sdlEvent.type == SDL_KEYDOWN) {
			switch (sdlEvent.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
			/*case SDL_SCANCODE_Q:
				isRunning = false;
				return;
				[[fallthrough]]; /// C17 Prevents switch/case fallthrough warnings*/
			default:
				break;
			}
		}
		if (currentScene == nullptr) {
			Debug::FatalError("Failed to initialize Scene", __FILE__, __LINE__);
			EngineManager::Instance()->SetIsRunning(false);
			return;
		}
		currentScene->HandleEvents(sdlEvent);
	}
}

void SceneManager::BuildNewScene(SCENE_NUMBER scene) {
	bool status; 

	if (currentScene != nullptr) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
		EngineManager::Instance()->GetAssetManager()->RemoveAllComponents();
	}

	switch (scene) {
	case SCENE_NUMBER::SCENE0:
		currentScene = new Scene0();
		status = currentScene->OnCreate();
		break;

	//case SCENE_NUMBER::SCENE1:
	//	currentScene = new Scene1();
	//	status = currentScene->OnCreate();
	//	break;
	//case SCENE_NUMBER::SCENE1V2:
	//	currentScene = new Scene1v2();
	//	status = currentScene->OnCreate();
	//	break;
	//case SCENE_NUMBER::SCENE2:
	//	currentScene = new Scene2();
	//	status = currentScene->OnCreate();
	//	break;
	//case SCENE_NUMBER::SCENE3:
	//	currentScene = new Scene3();
	//	status = currentScene->OnCreate();
	//	break;
	case SCENE_NUMBER::SCENE4:
		currentScene = new Scene4();
		status = currentScene->OnCreate();
		break;
	case SCENE_NUMBER::SCENE5:
		currentScene = new Scene5();
		status = currentScene->OnCreate();
		break;

	default:
		Debug::Error("Incorrect scene number assigned in the manager", __FILE__, __LINE__);
		currentScene = nullptr;
		break;
	}	
}
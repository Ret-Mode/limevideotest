#include "SDLApplication.h"
#include "SDLGamepad.h"
#include "SDLJoystick.h"

#ifdef HX_MACOS
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif


namespace lime {
	
	
	AutoGCRoot* Application::callback = 0;
	SDLApplication* SDLApplication::currentApplication = 0;
	
	const int analogAxisDeadZone = 1000;
	std::map<int, std::map<int, int> > gamepadsAxisMap;
	
	
	SDLApplication::SDLApplication () {
		
		if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) != 0) {
			
			printf ("Could not initialize SDL: %s.\n", SDL_GetError ());
			
		}
		
		SDL_LogSetPriority (SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN);
		
		currentApplication = this;
		
		framePeriod = 1000.0 / 60.0;
		
		#ifdef EMSCRIPTEN
		emscripten_cancel_main_loop ();
		emscripten_set_main_loop (UpdateFrame, 0, 0);
		emscripten_set_main_loop_timing (EM_TIMING_RAF, 1);
		#endif
		
		currentUpdate = 0;
		lastUpdate = 0;
		nextUpdate = 0;
		
		ApplicationEvent applicationEvent;
		DropEvent dropEvent;
		GamepadEvent gamepadEvent;
		JoystickEvent joystickEvent;
		KeyEvent keyEvent;
		MouseEvent mouseEvent;
		RenderEvent renderEvent;
		SensorEvent sensorEvent;
		TextEvent textEvent;
		TouchEvent touchEvent;
		WindowEvent windowEvent;
		
		SDL_EventState (SDL_DROPFILE, SDL_ENABLE);
		SDLJoystick::Init ();
		
		#ifdef HX_MACOS
		CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL (CFBundleGetMainBundle ());
		char path[PATH_MAX];
		
		if (CFURLGetFileSystemRepresentation (resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) {
			
			chdir (path);
			
		}
		
		CFRelease (resourcesURL);
		#endif
		
	}
	
	
	SDLApplication::~SDLApplication () {
		
		
		
	}
	
	
	int SDLApplication::Exec () {
		
		Init ();
		
		#ifdef EMSCRIPTEN
		
		return 0;
		
		#else
		
		while (active) {
			
			Update ();
			
		}
		
		return Quit ();
		
		#endif
		
	}
	
	
	void SDLApplication::HandleEvent (SDL_Event* event) {
		
		switch (event->type) {
			
			case SDL_USEREVENT:
				
				currentUpdate = SDL_GetTicks ();
				applicationEvent.type = UPDATE;
				applicationEvent.deltaTime = currentUpdate - lastUpdate;
				lastUpdate = currentUpdate;
				
				nextUpdate += framePeriod;
				
				while (nextUpdate <= currentUpdate) {
					
					nextUpdate += framePeriod;
					
				}
				
				ApplicationEvent::Dispatch (&applicationEvent);
				RenderEvent::Dispatch (&renderEvent);
				break;
			
			case SDL_APP_WILLENTERBACKGROUND:
				
				windowEvent.type = WINDOW_DEACTIVATE;
				WindowEvent::Dispatch (&windowEvent);
				break;
			
			case SDL_APP_WILLENTERFOREGROUND:
				
				windowEvent.type = WINDOW_ACTIVATE;
				WindowEvent::Dispatch (&windowEvent);
				break;
			
			case SDL_CONTROLLERAXISMOTION:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
				
				ProcessGamepadEvent (event);
				break;
			
			case SDL_DROPFILE:
				
				ProcessDropEvent (event);
				break;
			
			case SDL_FINGERMOTION:
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
				
				#ifndef HX_MACOS
				ProcessTouchEvent (event);
				#endif
				break;
			
			case SDL_JOYAXISMOTION:
				
				if (SDLJoystick::IsAccelerometer (event->jaxis.which)) {
					
					ProcessSensorEvent (event);
					
				} else {
					
					ProcessJoystickEvent (event);
					
				}
				
				break;
			
			case SDL_JOYBALLMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			case SDL_JOYHATMOTION:
			case SDL_JOYDEVICEADDED:
			case SDL_JOYDEVICEREMOVED:
				
				ProcessJoystickEvent (event);
				break;
			
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				
				ProcessKeyEvent (event);
				break;
			
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
				
				ProcessMouseEvent (event);
				break;
			
			case SDL_TEXTINPUT:
			case SDL_TEXTEDITING:
				
				ProcessTextEvent (event);
				break;
			
			case SDL_WINDOWEVENT:
				
				switch (event->window.event) {
					
					case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_MOVED:
					case SDL_WINDOWEVENT_RESTORED:
						
						ProcessWindowEvent (event);
						break;
					
					case SDL_WINDOWEVENT_EXPOSED: 
						
						RenderEvent::Dispatch (&renderEvent);
						break;
					
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						
						ProcessWindowEvent (event);
						RenderEvent::Dispatch (&renderEvent);
						break;
					
					case SDL_WINDOWEVENT_CLOSE:
						
						ProcessWindowEvent (event);
						break;
					
				}
				
				break;
			
			case SDL_QUIT:
				
				active = false;
				break;
			
		}
		
	}
	
	
	void SDLApplication::Init () {
		
		active = true;
		lastUpdate = SDL_GetTicks ();
		nextUpdate = lastUpdate;
		
	}
	
	
	void SDLApplication::ProcessDropEvent (SDL_Event* event) {
		
		if (DropEvent::callback) {
			
			dropEvent.type = DROP_FILE;
			dropEvent.file = event->drop.file;
			
			DropEvent::Dispatch (&dropEvent);
			SDL_free (dropEvent.file);
			
		}
		
	}
	
	
	void SDLApplication::ProcessGamepadEvent (SDL_Event* event) {
		
		if (GamepadEvent::callback) {
			
			switch (event->type) {
				
				case SDL_CONTROLLERAXISMOTION:
					
					if (gamepadsAxisMap[event->caxis.which].empty ()) {
						
						gamepadsAxisMap[event->caxis.which][event->caxis.axis] = event->caxis.value;
						
					} else if (gamepadsAxisMap[event->caxis.which][event->caxis.axis] == event->caxis.value) {
						
						break;
						
					}
					
					gamepadEvent.type = GAMEPAD_AXIS_MOVE;
					gamepadEvent.axis = event->caxis.axis;
					gamepadEvent.id = event->caxis.which;
					
					if (event->caxis.value > -analogAxisDeadZone && event->caxis.value < analogAxisDeadZone) {
						
						if (gamepadsAxisMap[event->caxis.which][event->caxis.axis] != 0) {
							
							gamepadsAxisMap[event->caxis.which][event->caxis.axis] = 0;
							gamepadEvent.axisValue = 0;
							GamepadEvent::Dispatch (&gamepadEvent);
							
						}
						
						break;
						
					}
					
					gamepadsAxisMap[event->caxis.which][event->caxis.axis] = event->caxis.value;
					gamepadEvent.axisValue = event->caxis.value / (event->caxis.value > 0 ? 32767.0 : 32768.0);
					
					GamepadEvent::Dispatch (&gamepadEvent);
					break;
				
				case SDL_CONTROLLERBUTTONDOWN:
					
					gamepadEvent.type = GAMEPAD_BUTTON_DOWN;
					gamepadEvent.button = event->cbutton.button;
					gamepadEvent.id = event->cbutton.which;
					
					GamepadEvent::Dispatch (&gamepadEvent);
					break;
				
				case SDL_CONTROLLERBUTTONUP:
					
					gamepadEvent.type = GAMEPAD_BUTTON_UP;
					gamepadEvent.button = event->cbutton.button;
					gamepadEvent.id = event->cbutton.which;
					
					GamepadEvent::Dispatch (&gamepadEvent);
					break;
				
				case SDL_CONTROLLERDEVICEADDED:
					
					if (SDLGamepad::Connect (event->cdevice.which)) {
						
						gamepadEvent.type = GAMEPAD_CONNECT;
						gamepadEvent.id = SDLGamepad::GetInstanceID (event->cdevice.which);
						
						GamepadEvent::Dispatch (&gamepadEvent);
						
					}
					
					break;
				
				case SDL_CONTROLLERDEVICEREMOVED: {
					
					gamepadEvent.type = GAMEPAD_DISCONNECT;
					gamepadEvent.id = event->cdevice.which;
					
					GamepadEvent::Dispatch (&gamepadEvent);
					SDLGamepad::Disconnect (event->cdevice.which);
					break;
					
				}
				
			}
			
		}
		
	}
	
	
	void SDLApplication::ProcessJoystickEvent (SDL_Event* event) {
		
		if (JoystickEvent::callback) {
			
			switch (event->type) {
				
				case SDL_JOYAXISMOTION:
					
					if (!SDLJoystick::IsAccelerometer (event->jaxis.which)) {
						
						joystickEvent.type = JOYSTICK_AXIS_MOVE;
						joystickEvent.index = event->jaxis.axis;
						joystickEvent.eventValue = event->jaxis.value / (event->jaxis.value > 0 ? 32767.0 : 32768.0);
						joystickEvent.id = event->jaxis.which;
						
						JoystickEvent::Dispatch (&joystickEvent);
						
					}
					break;
				
				case SDL_JOYBALLMOTION:
					
					if (!SDLJoystick::IsAccelerometer (event->jball.which)) {
						
						joystickEvent.type = JOYSTICK_TRACKBALL_MOVE;
						joystickEvent.index = event->jball.ball;
						joystickEvent.x = event->jball.xrel;
						joystickEvent.y = event->jball.yrel;
						joystickEvent.id = event->jball.which;
						
						JoystickEvent::Dispatch (&joystickEvent);
						
					}
					break;
				
				case SDL_JOYBUTTONDOWN:
					
					if (!SDLJoystick::IsAccelerometer (event->jbutton.which)) {
						
						joystickEvent.type = JOYSTICK_BUTTON_DOWN;
						joystickEvent.index = event->jbutton.button;
						joystickEvent.id = event->jbutton.which;
						
						JoystickEvent::Dispatch (&joystickEvent);
						
					}
					break;
				
				case SDL_JOYBUTTONUP:
					
					if (!SDLJoystick::IsAccelerometer (event->jbutton.which)) {
						
						joystickEvent.type = JOYSTICK_BUTTON_UP;
						joystickEvent.index = event->jbutton.button;
						joystickEvent.id = event->jbutton.which;
						
						JoystickEvent::Dispatch (&joystickEvent);
						
					}
					break;
				
				case SDL_JOYHATMOTION:
					
					if (!SDLJoystick::IsAccelerometer (event->jhat.which)) {
						
						joystickEvent.type = JOYSTICK_HAT_MOVE;
						joystickEvent.index = event->jhat.hat;
						joystickEvent.x = event->jhat.value;
						joystickEvent.id = event->jhat.which;
						
						JoystickEvent::Dispatch (&joystickEvent);
						
					}
					break;
				
				case SDL_JOYDEVICEADDED:
					
					if (SDLJoystick::Connect (event->jdevice.which)) {
						
						joystickEvent.type = JOYSTICK_CONNECT;
						joystickEvent.id = SDLJoystick::GetInstanceID (event->jdevice.which);
						
						JoystickEvent::Dispatch (&joystickEvent);
						
					}
					break;
				
				case SDL_JOYDEVICEREMOVED:
					
					if (!SDLJoystick::IsAccelerometer (event->jdevice.which)) {
						
						joystickEvent.type = JOYSTICK_DISCONNECT;
						joystickEvent.id = event->jdevice.which;
						
						JoystickEvent::Dispatch (&joystickEvent);
						SDLJoystick::Disconnect (event->jdevice.which);
						
					}
					break;
				
			}
			
		}
		
	}
	
	
	void SDLApplication::ProcessKeyEvent (SDL_Event* event) {
		
		if (KeyEvent::callback) {
			
			switch (event->type) {
				
				case SDL_KEYDOWN: keyEvent.type = KEY_DOWN; break;
				case SDL_KEYUP: keyEvent.type = KEY_UP; break;
				
			}
			
			keyEvent.keyCode = event->key.keysym.sym;
			keyEvent.modifier = event->key.keysym.mod;
			keyEvent.windowID = event->key.windowID;
			
			KeyEvent::Dispatch (&keyEvent);
			
		}
		
	}
	
	
	void SDLApplication::ProcessMouseEvent (SDL_Event* event) {
		
		if (MouseEvent::callback) {
			
			switch (event->type) {
				
				case SDL_MOUSEMOTION:
					
					mouseEvent.type = MOUSE_MOVE;
					mouseEvent.x = event->motion.x;
					mouseEvent.y = event->motion.y;
					mouseEvent.movementX = event->motion.xrel;
					mouseEvent.movementY = event->motion.yrel;
					break;
				
				case SDL_MOUSEBUTTONDOWN:
					
					SDL_CaptureMouse (SDL_TRUE);
					
					mouseEvent.type = MOUSE_DOWN;
					mouseEvent.button = event->button.button - 1;
					mouseEvent.x = event->button.x;
					mouseEvent.y = event->button.y;
					break;
				
				case SDL_MOUSEBUTTONUP:
					
					SDL_CaptureMouse (SDL_FALSE);
					
					mouseEvent.type = MOUSE_UP;
					mouseEvent.button = event->button.button - 1;
					mouseEvent.x = event->button.x;
					mouseEvent.y = event->button.y;
					break;
				
				case SDL_MOUSEWHEEL:
					
					mouseEvent.type = MOUSE_WHEEL;
					mouseEvent.x = event->wheel.x;
					mouseEvent.y = event->wheel.y;
					break;
				
			}
			
			mouseEvent.windowID = event->button.windowID;
			MouseEvent::Dispatch (&mouseEvent);
			
		}
		
	}
	
	
	void SDLApplication::ProcessSensorEvent (SDL_Event* event) {
		
		if (SensorEvent::callback) {
			
			double value = event->jaxis.value / 32767.0f;
			
			switch (event->jaxis.axis) {
				
				case 0: sensorEvent.x = value; break;
				case 1: sensorEvent.y = value; break;
				case 2: sensorEvent.z = value; break;
				default: break;
				
			}
			
			SensorEvent::Dispatch (&sensorEvent);
			
		}
		
	}
	
	
	void SDLApplication::ProcessTextEvent (SDL_Event* event) {
		
		if (TextEvent::callback) {
			
			switch (event->type) {
				
				case SDL_TEXTINPUT:
					
					textEvent.type = TEXT_INPUT;
					break;
				
				case SDL_TEXTEDITING:
					
					textEvent.type = TEXT_EDIT;
					textEvent.start = event->edit.start;
					textEvent.length = event->edit.length;
					break;
				
			}
			
			strcpy (textEvent.text, event->text.text);
			textEvent.windowID = event->text.windowID;
			TextEvent::Dispatch (&textEvent);
			
		}
		
	}
	
	
	void SDLApplication::ProcessTouchEvent (SDL_Event* event) {
		
		if (TouchEvent::callback) {
			
			switch (event->type) {
				
				case SDL_FINGERMOTION:
					
					touchEvent.type = TOUCH_MOVE;
					break;
				
				case SDL_FINGERDOWN:
					
					touchEvent.type = TOUCH_START;
					break;
				
				case SDL_FINGERUP:
					
					touchEvent.type = TOUCH_END;
					break;
				
			}
			
			touchEvent.x = event->tfinger.x;
			touchEvent.y = event->tfinger.y;
			touchEvent.id = event->tfinger.fingerId;
			touchEvent.dx = event->tfinger.dx;
			touchEvent.dy = event->tfinger.dy;
			touchEvent.pressure = event->tfinger.pressure;
			touchEvent.device = event->tfinger.touchId;
			
			TouchEvent::Dispatch (&touchEvent);
			
		}
		
	}
	
	
	void SDLApplication::ProcessWindowEvent (SDL_Event* event) {
		
		if (WindowEvent::callback) {
			
			switch (event->window.event) {
				
				case SDL_WINDOWEVENT_SHOWN: windowEvent.type = WINDOW_ACTIVATE; break;
				case SDL_WINDOWEVENT_CLOSE: windowEvent.type = WINDOW_CLOSE; break;
				case SDL_WINDOWEVENT_HIDDEN: windowEvent.type = WINDOW_DEACTIVATE; break;
				case SDL_WINDOWEVENT_ENTER: windowEvent.type = WINDOW_ENTER; break;
				case SDL_WINDOWEVENT_FOCUS_GAINED: windowEvent.type = WINDOW_FOCUS_IN; break;
				case SDL_WINDOWEVENT_FOCUS_LOST: windowEvent.type = WINDOW_FOCUS_OUT; break;
				case SDL_WINDOWEVENT_LEAVE: windowEvent.type = WINDOW_LEAVE; break;
				case SDL_WINDOWEVENT_MINIMIZED: windowEvent.type = WINDOW_MINIMIZE; break;
				
				case SDL_WINDOWEVENT_MOVED:
					
					windowEvent.type = WINDOW_MOVE;
					windowEvent.x = event->window.data1;
					windowEvent.y = event->window.data2;
					break;
					
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					
					windowEvent.type = WINDOW_RESIZE;
					windowEvent.width = event->window.data1;
					windowEvent.height = event->window.data2;
					break;
				
				case SDL_WINDOWEVENT_RESTORED: windowEvent.type = WINDOW_RESTORE; break;
				
			}
			
			windowEvent.windowID = event->window.windowID;
			WindowEvent::Dispatch (&windowEvent);
			
		}
		
	}
	
	
	int SDLApplication::Quit () {
		
		applicationEvent.type = EXIT;
		ApplicationEvent::Dispatch (&applicationEvent);
		
		SDL_Quit ();
		
		return 0;
		
	}
	
	
	void SDLApplication::RegisterWindow (SDLWindow *window) {
		
		#ifdef IPHONE
		SDL_iPhoneSetAnimationCallback (window->sdlWindow, 1, UpdateFrame, NULL);
		#endif
		
	}
	
	
	void SDLApplication::SetFrameRate (double frameRate) {
		
		if (frameRate > 0) {
			
			framePeriod = 1000.0 / frameRate;
			
		} else {
			
			framePeriod = 1000.0;
			
		}
		
	}
	
	
	static SDL_TimerID timerID = 0;
	bool timerActive = false;
	bool firstTime = true;
	
	Uint32 OnTimer (Uint32 interval, void *) {
		
		SDL_Event event;
		SDL_UserEvent userevent;
		userevent.type = SDL_USEREVENT;
		userevent.code = 0;
		userevent.data1 = NULL;
		userevent.data2 = NULL;
		event.type = SDL_USEREVENT;
		event.user = userevent;
		
		timerActive = false;
		timerID = 0;
		
		SDL_PushEvent (&event);
		
		return 0;
		
	}
	
	
	bool SDLApplication::Update () {
		
		SDL_Event event;
		event.type = -1;
		
		#if (!defined (IPHONE) && !defined (EMSCRIPTEN))
		
		if (active && (firstTime || WaitEvent (&event))) {
			
			firstTime = false;
			
			HandleEvent (&event);
			event.type = -1;
			if (!active)
				return active;
			
		#endif
			
			while (SDL_PollEvent (&event)) {
				
				HandleEvent (&event);
				event.type = -1;
				if (!active)
					return active;
				
			}
			
			currentUpdate = SDL_GetTicks ();
			
		#if defined (IPHONE)
			
			if (currentUpdate >= nextUpdate) {
				
				event.type = SDL_USEREVENT;
				HandleEvent (&event);
				event.type = -1;
				
			}
		
		#elif defined (EMSCRIPTEN)
			
			event.type = SDL_USEREVENT;
			HandleEvent (&event);
			event.type = -1;
		
		#else
			
			if (currentUpdate >= nextUpdate) {
				
				SDL_RemoveTimer (timerID);
				OnTimer (0, 0);
				
			} else if (!timerActive) {
				
				timerActive = true;
				timerID = SDL_AddTimer (nextUpdate - currentUpdate, OnTimer, 0);
				
			}
			
		}
		
		#endif
		
		return active;
		
	}
	
	
	void SDLApplication::UpdateFrame () {
		
		currentApplication->Update ();
		
	}
	
	
	void SDLApplication::UpdateFrame (void*) {
		
		UpdateFrame ();
		
	}
	
	
	int SDLApplication::WaitEvent (SDL_Event *event) {
		
		for(;;) {
			
			SDL_PumpEvents ();
			
			switch (SDL_PeepEvents (event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
				
				case -1: return 0;
				case 1: return 1;
				default: SDL_Delay (1);
				
			}
			
		}
		
	}
	
	
	Application* CreateApplication () {
		
		return new SDLApplication ();
		
	}
	
	
}


#ifdef ANDROID
int SDL_main (int argc, char *argv[]) { return 0; }
#endif

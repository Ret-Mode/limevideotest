#ifndef LIME_UI_GAMEPAD_EVENT_H
#define LIME_UI_GAMEPAD_EVENT_H


#include <hx/CFFI.h>


namespace lime {
	
	
	enum GamepadEventType {
		
		GAMEPAD_AXIS_MOVE,
		GAMEPAD_BUTTON_DOWN,
		GAMEPAD_BUTTON_UP,
		GAMEPAD_CONNECT,
		GAMEPAD_DISCONNECT
		
	};
	
	
	class GamepadEvent {
		
		public:
			
			static AutoGCRoot* callback;
			static AutoGCRoot* eventObject;
			
			GamepadEvent ();
			
			static void Dispatch (GamepadEvent* event);
			
			int axis;
			double axisValue;
			int button;
			int id;
			GamepadEventType type;
		
	};
	
	
}


#endif
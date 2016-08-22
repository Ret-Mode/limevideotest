#ifndef LIME_GRAPHICS_RENDERER_H
#define LIME_GRAPHICS_RENDERER_H


#include <graphics/ImageBuffer.h>
#include <math/Rectangle.h>
#include <ui/Window.h>
#include <hx/CFFI.h>


namespace lime {
	
	
	class Renderer {
		
		
		public:
			
			virtual void Flip () = 0;
			virtual void* GetContext () = 0;
			virtual double GetScale () = 0;
			virtual value Lock () = 0;
			virtual void MakeCurrent () = 0;
			virtual void ReadPixels (ImageBuffer *buffer, Rectangle *rect) = 0;
			virtual const char* Type () = 0;
			virtual void Unlock () = 0;
			virtual void CreateMovieTexture(unsigned int w, unsigned int h)=0;
			virtual void UpdateMovieTexture(unsigned char* y, int ystride, unsigned char *cb, int cbstride, unsigned char *cr, int crstride)=0;
			Window* currentWindow;
		
		
	};
	
	
	Renderer* CreateRenderer (Window* window);
	
	
}


#endif
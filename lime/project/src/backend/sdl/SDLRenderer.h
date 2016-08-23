#ifndef LIME_SDL_RENDERER_H
#define LIME_SDL_RENDERER_H


#include <SDL.h>
#include <graphics/Renderer.h>


#ifdef ANDROID
#include <android/log.h>
#define LOG_SOUND(args,...) __android_log_print(ANDROID_LOG_INFO, "Lime", args, ##__VA_ARGS__)
#else
#define LOG_SOUND(args,...) printf(args, ##__VA_ARGS__)
#endif

namespace lime {
	
	
	class SDLRenderer : public Renderer {
		
		public:
			
			SDLRenderer (Window* window);
			~SDLRenderer ();
			
			virtual void Flip ();
			virtual void* GetContext ();
			virtual double GetScale ();
			virtual value Lock ();
			virtual void MakeCurrent ();
			virtual void ReadPixels (ImageBuffer *buffer, Rectangle *rect);
			virtual const char* Type ();
			virtual void Unlock ();
			
			virtual void CreateMovieTexture(unsigned int w, unsigned int h);
			virtual void UpdateMovieTexture(int scale, unsigned char* y, int ystride, unsigned char *cb, int cbstride, unsigned char *cr, int crstride);
			virtual void DestroyMovieTexture();
			
			SDL_Renderer* sdlRenderer;
			SDL_Texture* sdlTexture;
			SDL_Texture* sdlMovie;
			SDL_Window* sdlWindow;
			
		private:
			
			SDL_GLContext context;
			int width;
			int height;
		
	};
	
	
}


#endif
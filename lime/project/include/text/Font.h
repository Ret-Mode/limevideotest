#ifndef LIME_TEXT_FONT_H
#define LIME_TEXT_FONT_H


#include <graphics/ImageBuffer.h>
#include <system/System.h>
#include <utils/Resource.h>
#include <hx/CFFI.h>

#ifdef HX_WINDOWS
#undef GetGlyphIndices
#endif


namespace lime {
	
	
	typedef struct {
		
		unsigned long codepoint;
		size_t size;
		int index;
		int height;
		
	} GlyphInfo;
	
	
	typedef struct {
		
		uint32_t index;
		uint32_t width;
		uint32_t height;
		uint32_t x;
		uint32_t y;
		unsigned char data;
		
	} GlyphImage;
	
	
	class Font {
		
		
		public:
			
			Font (Resource *resource, int faceIndex = 0);
			~Font ();
			
			value Decompose (int em);
			int GetAscender ();
			int GetDescender ();
			wchar_t *GetFamilyName ();
			int GetGlyphIndex (char* character);
			value GetGlyphIndices (char* characters);
			value GetGlyphMetrics (int index);
			int GetHeight ();
			int GetNumGlyphs ();
			int GetUnderlinePosition ();
			int GetUnderlineThickness ();
			int GetUnitsPerEM ();
			int RenderGlyph (int index, Bytes *bytes, int offset = 0);
			int RenderGlyphs (value indices, Bytes *bytes);
			void SetSize (size_t size);
			
			void* library;
			void* face;
			void* faceMemory;
			
		private:
			
			size_t mSize;
		
	};
	
	
}


#endif

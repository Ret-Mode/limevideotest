#ifndef VIDEOBUFFER_H
#define VIDEOBUFFER_H

#include <system/System.h>
#include <vector>

namespace lime {
	class VideoBuffer {
	public:
		
			virtual int testVideo(const char * const file, int scale, int delay)=0;/**/
			virtual void playVideo(double timestamp)=0;/**/
			virtual int processVideo(double timestamp)=0;/**/
			virtual void cleanVideo()=0;/**/
			virtual void stopVideo()=0;
			virtual void getMovieDimensions(unsigned int *w, unsigned int *h)=0;
			virtual void getYUV(int *scale, unsigned char **y, unsigned char **u, unsigned char **v, int *ystride, int *ustride, int *vstride)=0;
	};
	
	VideoBuffer* CreateVideoBuffer ();
	
}

#endif //VIDEOBUFFER_H

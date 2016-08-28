#ifndef VIDEOBUFFER_H
#define VIDEOBUFFER_H


#ifdef ANDROID
#include <android/log.h>
#endif


#ifdef ANDROID
#define LOG_VIDEO(args,...) __android_log_print(ANDROID_LOG_INFO, "Lime", args, ##__VA_ARGS__)
#else
#ifdef IPHONE
//#define LOG_VIDEO(args,...) printf(args, ##__VA_ARGS__)
#define LOG_VIDEO(args...) { }
#elif defined(TIZEN)
#include <FBase.h>
#define LOG_VIDEO(args,...) AppLog(args, ##__VA_ARGS__)
#else
#define LOG_VIDEO(args,...) printf(args, ##__VA_ARGS__)
#endif
#endif

#include <system/System.h>
#include <vector>
#include <utils/Resource.h>

namespace lime {
	class VideoBuffer {
	public:
		
			virtual int testVideo(Resource *resource, int scale, int delay)=0;/**/
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

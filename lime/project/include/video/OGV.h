#ifndef OGV_H
#define OGV_H

#include "VideoBuffer.h"

#include <system/System.h>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <theora/codec.h>
#include <theora/theora.h>
#include <theora/theoradec.h>

#define OGGBUFFSIZE 4096*4096

namespace lime {

	class OGV : public VideoBuffer
	{
		public:
			OGV();/**/
			~OGV();/**/

			int testVideo(Resource *resource, int scale, int delay);/**/
			void playVideo(double timestamp);/**/
			int processVideo(double timestamp);/**/
			void cleanVideo();/**/
			void stopVideo();
			void getMovieDimensions(unsigned int *w, unsigned int *h);
			void getYUV(int *scale, unsigned char **y, unsigned char **u, unsigned char **v, int *ystride, int *ustride, int *vstride);
		private:
			std::vector<ogg_stream_state*> streams;
			void fMemSet(void *addr, int size);/**/
			int fFillOggPage(FILE_HANDLE *fp, ogg_sync_state *state_ptr);/**/
			int getPage(FILE_HANDLE *movie, ogg_sync_state *oggstate_ptr, ogg_page *oggpage_ptr);/**/
			int getPacket(ogg_stream_state *stream_ptr, ogg_packet *oggpacket_ptr);/**/
			int checkTheoraStream(int *headers, ogg_stream_state *state);/**/
			void increasePPlevel();/**/
			void reducePPlevel();/**/
			int decodeTheora();/**/
			int prepareTheora();/**/
			int getVideoPacket();/**/
			void feedPacket();/**/
			void drawFrame();/**/
			void waitForFrame();
			void clearStream();


			int streamID;
			int buffers;
			int haveTheora;
			int videopassed;
			int eovf;
			int readedFromState;
			double msperframe;
			int pplevelcurrent;
			int pplevelmax;
			int prevtime;
			int nexttime;
			double deltatime;
			int presentVideoPacket;
			int framecontrol;
			int framedelay;
			int videoScale;
			FILE_HANDLE *videoFile;
			//FILE_HANDLE *fhandle;			
			ogg_stream_state *oggstream;
			ogg_int64_t granpos;
			ogg_packet oggpacket;
			ogg_page oggpage;
			ogg_sync_state oggstate;
			th_setup_info *decodersetup;
			th_dec_ctx *decinstance;
			th_info tinfo;
			th_comment tcomment;
			th_ycbcr_buffer ybr;



	};

}

#endif // OGV_H

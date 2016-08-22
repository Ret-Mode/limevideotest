/**
Copyright (c) <2016> <RetardMode from OpenFL.org community>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation is not required.
2. Altered source versions must not be
   misrepresented as being the original software.
3. This notice may be removed or altered from any source distribution.
*/

#include "video/OGV.h"

#include <backend/sdl/SDLRenderer.h>

namespace lime {

	void OGV::fMemSet(void *addr, int size){
		memset(addr,0,size);
	}

	OGV::OGV(void *rend)
	{	SDL_RendererInfo info;
		SDL_Renderer *r;
		SDLRenderer *renderer_ptr = (SDLRenderer*)rend;
		SDL_Texture *txt = renderer_ptr->sdlTexture;
		window = renderer_ptr->sdlWindow;
		r = renderer_ptr->sdlRenderer;
		SDL_GetRendererInfo(r,&info);
		renderer = SDL_CreateRenderer(renderer_ptr->sdlWindow,-1,info.flags);
		//SDL_SetRenderTarget(renderer, txt);
		texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,100,100);
		eovf = 0;
		
	}

	OGV::~OGV()
	{
		//dtor
	}


	int OGV::fFillOggPage(FILE *fp, ogg_sync_state *state_ptr){
		char *buffer;
		int size;
		int dbuff;
		size=0;
		dbuff=0;
		buffer=ogg_sync_buffer(state_ptr,OGGBUFFSIZE);
		do{/*while(ogg_sync_wrote(state,size)!=0 && retval);*/
			do{/*while(size<OGGBUFFSIZE && size>0 && retval);*/

				size=fread(buffer+dbuff,1,OGGBUFFSIZE-dbuff,fp);
				dbuff+=size;
				if(feof(fp)){

					size=0;
					break;
				}
				//printf("\t%d - buffsize\n",size);
			}while(dbuff<OGGBUFFSIZE);
			//written += dbuff;
			//printf("%d\n",written);
		}while(ogg_sync_wrote(state_ptr,dbuff) && size);

		//printf("::::%d - buffsize\n",size);
		return size;
	}


	int OGV::getPage(FILE *movie, ogg_sync_state *oggstate_ptr, ogg_page *oggpage_ptr){
		while(ogg_sync_pageout(oggstate_ptr,oggpage_ptr)!=1 && eovf){
			if(readedFromState){
				if(!fFillOggPage(movie,oggstate_ptr)){
					readedFromState=0;
				}
			} else {
				eovf = 0;
				return 0;
			}
		}
		return ogg_page_serialno(oggpage_ptr);
	}

	int OGV::checkTheoraStream(int *headers, ogg_stream_state *state){
		if(*headers!=0){
			*headers=th_decode_headerin(&tinfo,&tcomment,&decodersetup,&oggpacket);
		}
		if(*headers>0 ){
			haveTheora = 1;
			streamID = state->serialno;
			oggstream = state;
		}

		if(*headers==0){
			decinstance=th_decode_alloc(&tinfo,decodersetup);
			th_setup_free(decodersetup);
			th_decode_ctl(decinstance,TH_DECCTL_GET_PPLEVEL_MAX,&pplevelmax,sizeof(pplevelmax));
			pplevelcurrent = 0;
			th_decode_ctl(decinstance,TH_DECCTL_SET_PPLEVEL,&pplevelcurrent,sizeof(pplevelcurrent));
			msperframe = 1000.f / (tinfo.fps_numerator * 1.0 / tinfo.fps_denominator);
			texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,tinfo.frame_width,tinfo.frame_height);
			int w;
			int h;

			switch(videoScale){
				case(0):
					destrect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
					if(destrect){
						SDL_GetWindowSize(window,&w,&h);
						destrect->x = w/2 - tinfo.frame_width/2;
						destrect->y = h/2 - tinfo.frame_height/2;
						destrect->w = tinfo.frame_width;
						destrect->h = tinfo.frame_height;
					}
				break;

				case(1):
					destrect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
					if(destrect){
						SDL_GetWindowSize(window,&w,&h);
						float wscale = tinfo.frame_width * 1.f / w;
						float hscale = tinfo.frame_height * 1.f / h;
						if(wscale > hscale){
							destrect->x = 0;
							destrect->w = w;
							destrect->y = (h - tinfo.frame_height/wscale)/2;
							destrect->h = tinfo.frame_height/wscale;
						} else {
							destrect->y = 0;
							destrect->h = h;
							destrect->x = (w - tinfo.frame_width / hscale)/2;
							destrect->w = tinfo.frame_width / hscale;
						}
					}
				break;

				case(2): default:
					destrect = NULL;
				break;

			}
			th_decode_packetin(decinstance,&oggpacket, &granpos);
		}
		return 0;
	}

	void OGV::increasePPlevel(){
		pplevelcurrent++;
		th_decode_ctl(decinstance,TH_DECCTL_SET_PPLEVEL,&pplevelcurrent,sizeof(pplevelcurrent));
	}

	void OGV::reducePPlevel(){
		pplevelcurrent--;
		th_decode_ctl(decinstance,TH_DECCTL_SET_PPLEVEL,&pplevelcurrent,sizeof(pplevelcurrent));
	}


	int OGV::decodeTheora(){
		int decvideo=th_decode_packetin(decinstance,&oggpacket, &granpos);/*decode frame*/

		if(decvideo==0){/*do we have full frame?*/
				//SDL_Rect srcrect;
				th_decode_ycbcr_out(decinstance,ybr);
				SDL_RenderClear(renderer);
				SDL_UpdateYUVTexture(texture,NULL, ybr[0].data,ybr[0].stride,ybr[1].data, ybr[1].stride, ybr[2].data,ybr[2].stride);
				SDL_RenderCopy(renderer, texture, NULL, destrect);
				SDL_RenderPresent(renderer);
				videopassed ++;
				//printf("%d %d\n",videopassed,audiopassed);
			} else if(decvideo == TH_DUPFRAME){
				decvideo = 0;
				videopassed ++;
			}

			//printf("--v: %d a: %d q: %d\n",buffer->videopassed, buffer->audiopassed, buffer->pplevelcurrent);

		return decvideo;
	}

	void OGV::fillAudioBuffer(ALuint *buf){
		int readed = 0;
		int audiostream = 0;
		do{
			eoaf = ov_read(&vorbisfile,audiobuffer+readed,audiobuffersize - readed,0,audiobits,1,&audiostream );
			if(eoaf >= 0){
				readed += eoaf;
			}
		}while(eoaf && readed < audiobuffersize);
		if(readed > 0){
			alBufferData(*buf,audioformat,audiobuffer,readed,vorbisfile.vi->rate);
			alSourceQueueBuffers(al_source,1,buf);
		}
	}


	int OGV::prepareAudio(char *filename){
		if(ov_fopen(filename,&vorbisfile)){
			haveVorbis = 0;
			eoaf = 0;
			return 0;
		} else {
			haveVorbis = 1;
			alGenSources(1,&al_source);
			alGenBuffers(4,al_buffers);
			//buffer->vi = ov_info(&(buffer->vorbisfile),-1);
			//buffer->vc = ov_comment(&(buffer->vorbisfile),-1);
			if(audiobits ==2 ){
				if(vorbisfile.vi->channels == 2){
					audioformat = AL_FORMAT_STEREO16;
				} else {
					audioformat = AL_FORMAT_MONO16;
				}
			} else {
				if(vorbisfile.vi->channels == 2){
					audioformat = AL_FORMAT_STEREO8;
				} else {
					audioformat = AL_FORMAT_MONO8;
				}
			}
			int tmp = vorbisfile.vi->rate * 1.0 / ((tinfo.fps_numerator * 1.0) / tinfo.fps_denominator);
			audiobuffersize = tmp * audiobits * vorbisfile.vi->channels;
			audiobuffer = (char *)malloc(audiobuffersize);
			msperaudio = (1000.f * tmp) / vorbisfile.vi->rate;
			int i=0;
			for(i=0;i<4 && eoaf;i++){
				fillAudioBuffer(al_buffers + i);
				if(!eoaf){
					break;
				}
			}
		}
		return haveVorbis;
	}


	void OGV::updateAudioBuffer(){
		ALint i = 0;
		ALint state;
		ALuint buf;
		alGetSourcei(al_source,AL_BUFFERS_PROCESSED,&i);
		audiopassed +=i;
		while(i){
			alSourceUnqueueBuffers(al_source,1,&buf);
			fillAudioBuffer(&buf);
			i--;
			if(!eoaf){
				break;
			}
		}
		alGetSourcei(al_source,AL_SOURCE_STATE,&state);
		if(state == AL_STOPPED && eoaf){
			if(pplevelcurrent > 0){
				reducePPlevel();
			}
			alSourcePlay(al_source);
		}
	}

	int OGV::getVideoPacket(){
		int state = 1;
		if(eovf){
			do{
				presentVideoPacket=getPacket(oggstream,&(oggpacket));
				if(presentVideoPacket > 0){
					state = decodeTheora();
				}

			}while(state && presentVideoPacket);
		}
		return state;
	}


	void OGV::feedPacket(){
		int serialnumber;
		do{
			do{
				serialnumber=getPage(videoFile, &oggstate, &oggpage);
			} while (eovf && serialnumber != streamID);
		}while(eovf && ogg_stream_pagein(oggstream,&oggpage)<0);
	}

	void OGV::drawFrame(){
		while(eovf && getVideoPacket()){
			feedPacket();
		}

	}

	int OGV::getPacket(ogg_stream_state *stream_ptr, ogg_packet *oggpacket_ptr){
		int flag;
		do{
			flag=ogg_stream_packetout(stream_ptr,oggpacket_ptr);
		}while(flag<0);
		return flag;
	}


	void OGV::waitForFrame(){

		prevtime = SDL_GetTicks();
		deltatime = (prevtime-nexttime);
		nexttime = prevtime;
		if(deltatime > 0){

			if(deltatime < msperframe){

				if((deltatime < msperframe/2) && (pplevelcurrent < pplevelmax)){

					increasePPlevel();

				}

				SDL_Delay((int)msperframe - deltatime);

				nexttime += (int)msperframe - deltatime;

			} else {

				nexttime += deltatime - msperframe;

				if(pplevelcurrent > 0){

					reducePPlevel();

				}
			}

		} else {

			if(pplevelcurrent > 0){

				reducePPlevel();

			}

		}

	}

	void OGV::clearStream(int *streams){
		int i = 0;
		//ogg_stream_state *state;
		if(haveTheora){
			while(i < *streams){
				if(streamList[i]->serialno != streamID){
					ogg_stream_destroy(streamList[i]);
				}
				i++;
			}
		}
		else {
			while(i < *streams){
				ogg_stream_destroy(streamList[i]);
				i++;
			}
		}


		free(streamList);
		streamList = NULL;
	}

	ogg_stream_state *OGV::addStream(int *streams, int serial){
		int i = 0;
		if(*streams == 0){
			*streams = *streams +1;
			streamList = (ogg_stream_state**)malloc(sizeof(ogg_stream_state*));
			*streamList = (ogg_stream_state*)malloc(sizeof(ogg_stream_state));
			ogg_stream_init(*streamList,serial);
			return *streamList;
		} else {
			while(i < *streams){
				if(streamList[i]->serialno == serial){
					return streamList[i];
				}
				i++;
			}
			*streams = *streams + 1;
			streamList = (ogg_stream_state**)realloc(streamList,sizeof(ogg_stream_state*) *  (*streams));
			streamList[(*streams)-1] = (ogg_stream_state*)malloc(sizeof(ogg_stream_state));
			ogg_stream_init(streamList[(*streams)-1],serial);
			return streamList[(*streams)-1];
		}
	}

	int OGV::prepareTheora(){
		int headers = 3;
		int serialnumber;
		int gotTheora = 0;
		int streams = 0;
		ogg_stream_state *state = NULL;
		do{
			if(headers){
				do{
					if(haveTheora){
						do{
							serialnumber=getPage(videoFile, &oggstate, &oggpage);
						} while (serialnumber != streamID && eovf);
						state = oggstream;
					} else {
						serialnumber=getPage(videoFile, &oggstate, &oggpage);
						state = addStream(&streams, serialnumber);

					}
				}while(eovf && ogg_stream_pagein(state,&oggpage)<0);
			}

			do{
				presentVideoPacket=getPacket(state,&oggpacket);
				if(presentVideoPacket > 0){
					checkTheoraStream(&headers,state);
				}
				if(headers == 0){
					gotTheora = 1;
				}

			}while(eovf && headers && presentVideoPacket != 0 );

		}while(eovf && headers);
		clearStream(&streams);
		return gotTheora;
	}

	int OGV::test_theora(char *filename, int scale, int delay){
		return 1;
	}


	int OGV::processVideo(){
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		return 0;
	}
	void OGV::stopVideo(){
		eoaf = 0;
		eovf = 0;
	}

	void OGV::cleanVideo(){
		

		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		//SDL_RenderClear(renderer);
		//SDL_RenderPresent(renderer);

	}

	void OGV::playVideo(){
		nexttime = prevtime = SDL_GetTicks();
		deltatime = 0;
		haveVorbis = 1;
		eoaf = 1;
		alSourcePlay(al_source);
		//SDL_Delay(msperframe);
	}

}
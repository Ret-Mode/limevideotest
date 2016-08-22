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

namespace lime {

	void OGV::fMemSet(void *addr, int size){
		memset(addr,0,size);
	}

	void OGV::getYUV(unsigned char **y, unsigned char **u, unsigned char **v, int *ystride, int *ustride, int *vstride){
		*y = ybr[0].data;
		*ystride = ybr[0].stride;
		*u = ybr[1].data;
		*ustride = ybr[1].stride;
		*v = ybr[2].data;
		*vstride = ybr[1].stride;
	}
	
	void OGV::getMovieDimensions(unsigned int *w, unsigned int *h){
		if(haveTheora){
			*w = tinfo.frame_width;
			*h = tinfo.frame_height;
		}
	}
	
	OGV::OGV()
	{
		fMemSet(&granpos,sizeof(granpos));
		fMemSet(&oggpacket,sizeof(oggpacket));
		fMemSet(&oggpage,sizeof(oggpage));
		fMemSet(&oggstate,sizeof(oggstate));
		videopassed = 0;
		decodersetup = NULL;
		decinstance = NULL;
		haveTheora = 0;
		framecontrol = 0;
		eovf = 1;
		streamID = 0;
		readedFromState = 1;
		presentVideoPacket = 1;
		/**buffer->framedelay = delay;*/
	}

	OGV::~OGV()
	{
		//dtor
	}


	int OGV::fFillOggPage(FILE_HANDLE *fp, ogg_sync_state *state_ptr){
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
				if(size==0){

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


	int OGV::getPage(FILE_HANDLE *movie, ogg_sync_state *oggstate_ptr, ogg_page *oggpage_ptr){
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

		if(decvideo==0){
				th_decode_ycbcr_out(decinstance,ybr);				
				videopassed ++;
			} else if(decvideo == TH_DUPFRAME){
				decvideo = 0;
				videopassed ++;
			}

		return decvideo;
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


	void OGV::clearStream(){
		
		if(haveTheora){
			for(int i = 0; i < streams.size();i++){
				if(streams[i]->serialno != streamID){
					ogg_stream_destroy(streams[i]);
				}
			}
		} else {
			for(int i = 0; i < streams.size();i++){		
					ogg_stream_destroy(streams[i]);
			}
		}
	}

	int OGV::prepareTheora(){
		int headers = 3;
		int serialnumber;
		int gotTheora = 0;
		
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
						int found=0;
						for(int i = 0;i<streams.size();i++){
							if(streams[i]->serialno==serialnumber){
								state = streams[i];
								found = 1;
								break;
							}
						}
						if(!found) {
							state = (ogg_stream_state*)malloc(sizeof(ogg_stream_state));
							ogg_stream_init(state,serialnumber);
							streams.push_back(state);
						}
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
		clearStream();
		return gotTheora;
	}

	int OGV::test_theora(FILE_HANDLE * file, int scale, int delay){
		videoFile = file;
		videoScale = scale;
		framedelay = delay;
		if(videoFile){
			th_info_init(&tinfo);
			th_comment_init(&tcomment);
			if(prepareTheora()){
				return 1;
			} else {
				ogg_sync_clear(&oggstate);
				fclose(videoFile);
				th_info_clear(&tinfo);
				th_comment_clear(&tcomment);
				return 0;
			}
		} else {
			return 0;
		}
	}


	int OGV::processVideo(){

		drawFrame();
		return eovf;
		
	}
	
	void OGV::stopVideo(){
		eovf = 0;
	}

	void OGV::cleanVideo(){

		ogg_stream_destroy(oggstream);

/** should free these, and close file**/
//		for(int i = 0; i < streams.size();i++){
//			free(streams[i]);
//		}
//		fclose(videoFile);
		th_info_clear(&tinfo);
		th_comment_clear(&tcomment);
		th_decode_free(decinstance);

		ogg_sync_clear(&oggstate);

		//rendererstruct->DestroyMovieTexture();


	}

	void OGV::playVideo(){
		deltatime = 0;
		//SDL_Delay(msperframe);
	}

}

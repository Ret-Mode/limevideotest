package lime.video;

import lime.graphics.Renderer;
import lime._backend.native.NativeRenderer;

#if !macro
@:build(lime.system.CFFI.build())
#end

class VideoFile {
	var file:Dynamic;
	public var gotVideo:Bool;
	public var rendererBackend:Dynamic;
	static public var SCALE_NORMAL:Int = 0;
	static public var SCALE_FIT:Int = 1;
	static public var SCALE_FILL:Int = 2;
	public function new (){
		file = null;
		gotVideo = false;
	}
	
	public function initFromPath (path:String, renderer:Dynamic, scale:Int, deltaTime:Int){
		rendererBackend = renderer;
		file = lime_video_load (path,renderer,scale,deltaTime);
		gotVideo = true;
		
		/*if(file != null){
			
		}*/
	}
	
	public function nextFrame():Bool{
		if(gotVideo){
			gotVideo = lime_video_load_frame (file,rendererBackend);
		}
		return gotVideo;
		
	}
	
	#if ((cpp || neko || nodejs) && !macro)
	@:cffi private static function lime_video_load (path:String,renderer:Dynamic,scale:Int,deltaTime:Int):Dynamic;
	@:cffi private static function lime_video_load_frame (file:Dynamic,rendererBackend:Dynamic):Bool;
	#end

}
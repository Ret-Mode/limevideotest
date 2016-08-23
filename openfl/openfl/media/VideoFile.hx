package openfl.media; #if !openfl_legacy


import openfl._internal.renderer.RenderSession;
import openfl.display.DisplayObject;
import openfl.display.InteractiveObject;
import openfl.display.Stage;
import lime.video.VideoFile in LimeVideoFile;
import lime.graphics.Renderer;
import openfl.display.Sprite;
import openfl.gl.GL;

class VideoFile extends Sprite {

	static public var SCALE_NORMAL:Int = 0;
	static public var SCALE_FIT:Int = 1;
	static public var SCALE_FILL:Int = 2;

	public var vfile:LimeVideoFile;
	
	public function new (name:String, deltaFrames:Int, scale:Int):Void {
		super ();
		vfile = new LimeVideoFile();
		var a = Lib.application.renderer.getBackend();
		vfile.initFromPath(name,a,deltaFrames,scale);
		//vfile.initFromPath (path:String, renderer:Dynamic, scale:Int, deltaTime:Int)
	}
	
	public override function __renderGL (renderSession:RenderSession):Void {
		if(vfile.gotVideo){
			vfile.nextFrame();
			GL.pixelStorei(0x0CF2, 0);
		}
	}
	
}
#end
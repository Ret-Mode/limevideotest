package lime.graphics;


import lime.app.Event;
import lime.math.Rectangle;
import lime.ui.Window;
import haxe.CallStack;

class Renderer {
	
	
	public var context:RenderContext;
	public var onContextLost = new Event<Void->Void> ();
	public var onContextRestored = new Event<RenderContext->Void> ();
	public var onRender = new Event<Void->Void> ();
	public var type:RendererType;
	public var window:Window;
	
	@:noCompletion private var backend:RendererBackend;
	
	
	public function new (window:Window) {
		
		trace(CallStack.toString(CallStack.callStack()));
		
		this.window = window;
		
		backend = new RendererBackend (this);
		
		this.window.renderer = this;
		
	}
	
	
	public function create ():Void {
		trace(CallStack.toString(CallStack.callStack()));
		backend.create ();
		
	}
	
	
	public function flip ():Void {
		
		backend.flip ();
		
	}
	
	
	public function readPixels (rect:Rectangle = null):Image {
		
		return backend.readPixels (rect);
		
	}
	
	
	private function render ():Void {
		
		backend.render ();
		
	}
	
	public function getBackend ():Dynamic {
		
		return backend.handle;
		
	}
	
}


#if flash
@:noCompletion private typedef RendererBackend = lime._backend.flash.FlashRenderer;
#elseif (js && html5)
@:noCompletion private typedef RendererBackend = lime._backend.html5.HTML5Renderer;
#else
@:noCompletion private typedef RendererBackend = lime._backend.native.NativeRenderer;
#end
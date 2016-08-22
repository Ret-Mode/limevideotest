package;

//http://haxecoder.com/post.php?id=17

import flash.display.Sprite;
import flash.events.Event;
import flash.events.KeyboardEvent;
import flash.text.TextField;
import flash.text.TextFormat;
import flash.text.TextFormatAlign;
import flash.Lib;
import openfl.media.VideoFile;

enum GameState {
	Paused;
	Playing;
}

class Main extends Sprite {
	var inited:Bool;
	private var video:VideoFile;
	private var currentGameState:GameState;
	private var scorePlayer:Int;
	private var scoreAI:Int;
	private var scoreField:flash.text.TextField;
	private var messageField:flash.text.TextField;
	private var platform1:Platform;
	private var platform2:Platform;
	private var ball:Ball;
	private var arrowUp:Bool;
	private var arrowDown:Bool;
	private var platformSpeed:Int;

	function resize (e){
		if (!inited) {
			init();
		}
	}

	function init (){
		if (inited){
			return;
		}
		inited = true;
		
		video = new VideoFile("assets/tmp.ogg",0,VideoFile.SCALE_NORMAL);
		
		arrowUp = false;
		arrowDown = false;
		platformSpeed = 7;

		var scoreFormat:TextFormat = new TextFormat("assets/Arial.ttf",24,0x00bbbb,true);
		var messageFormat:TextFormat = new TextFormat("Arial",18,0x00bbbb,true);

		scoreFormat.align = TextFormatAlign.CENTER;
		messageFormat.align = TextFormatAlign.CENTER;

		scoreField = new TextField ();
		//this.addChild(scoreField);
		scoreField.width = 500;
		scoreField.y = 30;
		scoreField.defaultTextFormat = scoreFormat;
		scoreField.selectable = false;

		messageField = new TextField ();
		//this.addChild(messageField);
		messageField.width = 500;
		messageField.y = 450;
		messageField.defaultTextFormat = messageFormat;
		messageField.selectable = false;
		messageField.text = "Press space to start\nUse Arrow keys";

		scorePlayer = 0;
		scoreAI = 0;

		platform1 = new Platform();
		platform1.x = 5;
		platform1.y = 200;
		//this.addChild(platform1);

		platform2 = new Platform();
		platform2.x = 480;
		platform2.y = 200;
		//this.addChild(platform2);

		ball = new Ball();
		ball.x = 250;
		ball.y = 250;
		this.addChild(video);
		//this.addChild(ball);

		setGameState(Paused);
		stage.addEventListener(KeyboardEvent.KEY_DOWN, keyDown);
		stage.addEventListener(KeyboardEvent.KEY_UP, keyUp);

		this.addEventListener(Event.ENTER_FRAME, everyFrame);
	}

	private function everyFrame(event:Event){
		
		
			if (currentGameState == Playing){
				if (arrowUp){
					platform1.y -= platformSpeed;
					if (platform1.y < 5){
						platform1.y = 5;
					}
				}
				if (arrowDown){
					platform1.y += platformSpeed;
					if (platform1.y > 395){
						platform1.y = 395;
					}
				}
			}
		
	}

	private function keyUp(event:KeyboardEvent){
		if (event.keyCode == 65){
			arrowUp = false;
		} 
		if (event.keyCode == 90){
			arrowDown = false;
		}
	}

	private function keyDown(event:KeyboardEvent):Void {
		if (event.keyCode == 32){
			if (currentGameState == Playing) {
				setGameState(Paused);
			} else {
				setGameState(Playing);		
			}
		} 
		if (event.keyCode == 65){
			arrowUp = true;
		} 
		if (event.keyCode == 90){
			arrowDown = true;
		}
	}

	private function updateScore ():Void {
		scoreField.text = scorePlayer + ":" + scoreAI;
	}

	private function setGameState (state:GameState):Void {
		currentGameState = state;
		updateScore();
		if(state == Paused){
			messageField.alpha = 1;
		} else {
			messageField.alpha = 0;
			platform1.y = 200;
			platform2.y = 200;
		}
	}

	public function new () {
		
		super ();
		addEventListener(Event.ADDED_TO_STAGE,added);
		
	}

	function added(e){
		removeEventListener(Event.ADDED_TO_STAGE,added);
		stage.addEventListener(Event.RESIZE, resize);
		init();
	}

	public static function main (){
		//Lib.current.stage.align = flash.display.StageAlign.TOP_LEFT;
		//Lib.current.stage.scaleMode = flash.display.StageScaleMode.NO_SCALE;
		Lib.current.addChild(new Main());
	} 
	
}
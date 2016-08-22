package ;
import openfl.display.Sprite;

class Ball extends Sprite
{

	public function new() 
	{
		super();
		this.graphics.beginFill(0x00ffff);
		this.graphics.drawCircle(0, 0, 10);
		this.graphics.endFill();
	}
	
}
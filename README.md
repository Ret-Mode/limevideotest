# limevideotest
video test of lime 2.9.1

Setup:

git clone https://github.com/RetardMode/limevideotest yourfolder
cd yourfolder
haxelib dev lime lime
haxelib dev openfl openfl
lime rebuild (add-your-os-here) -debug
cd PongVideoTest
lime build neko -debug


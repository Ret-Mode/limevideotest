# limevideotest
video test of lime 2.9.1

Setup:
```
  git clone https://github.com/RetardMode/limevideotest yourfolder

  cd yourfolder

  haxelib dev lime lime

  haxelib dev openfl openfl

  lime rebuild (add-your-os-here) -debug

  cd PongVideoTest

  lime build neko -debug
```

Changed files, relative to lime 2.9.1 and openfl 3.6.1:

  .cpp:
``` 
lime/project/Build.xml
lime/project/include/graphics/Renderer.h
lime/project/include/video/*
lime/project/lib/theora/*
lime/project/src/ExternalInterface.cpp
lime/project/src/backend/sdl/SDLRenderer.*
lime/project/src/video/*

  .hx:
  
lime/lime/video/VideoFile.hx
lime/lime/graphics/Renderer.hx
openfl/openfl/media/VideoFile.hx
```

there is still music to handle

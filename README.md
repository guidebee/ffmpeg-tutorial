# ffmpeg-tutorial
ffmpeg-tutorial

# How to compile 
 Download [vcpkg](https://github.com/microsoft/vcpkg)
 
 install SDL2, FFmpeg and Opencv
 
 please use x64 and dynamic triplets:
 
 for instance on Mac
 
 
 ```
  cat x64-osx.cmake

  set(VCPKG_TARGET_ARCHITECTURE x64)
  set(VCPKG_CRT_LINKAGE dynamic)
  set(VCPKG_LIBRARY_LINKAGE dynamic)
  
  set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
```
 
 ```
  vcpkg install SDL2 FFmpeg opencv
```

 ```
  mkdir build
  cd build
  cmake ..
  ninja
 ```

# tutorials
  tutorial01 -- basic usage of ffmpeg
  tutorial02 -- use SDL2 to display video
  tutorial03 -- use opencv to process video

# Video 

[![OpenCv ](http://img.youtube.com/vi/XgI5yTtnNPQ/0.jpg)](http://www.youtube.com/watch?v=XgI5yTtnNPQ)


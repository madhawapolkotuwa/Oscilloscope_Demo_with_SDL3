## Building a Real-Time Oscilloscope with SDL and ImGui in C++

![](https://github.com/madhawapolkotuwa/Oscilloscope_Demo_with_SDL3/blob/master/SDLOscilloscope.gif)


# YouTube Video

[![Youtube Video](https://img.youtube.com/vi/829KXcbLowE/0.jpg)](https://www.youtube.com/watch?v=829KXcbLowE)

## build
```bash
git clone https://github.com/madhawapolkotuwa/Oscilloscope_Demo_with_SDL3.git
git submodule update --init --recursive

cmake -B build -S .
cd build
cmake --build . -j12
```

executable file: build/bin/debug/
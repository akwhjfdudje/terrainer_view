# Terrainer View

This repository contains a terrain viewer, that lets users view procedurally-generated terrain, using OpenGL.

## Building

This project contains submodules that will be compiled along with the viewer itself.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```


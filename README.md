# Shaderlive
Simple GLSL shader livecoding tool

Linux only due to a usage of `stat` for checking file's last modified state.

### Compilation ###
```make clean all```

### Usage ###
Once compiled you can run `main` file, open your favorite text editor and edit files named `shader.vert` and `shader.frag`. Shaders will be recompiled and updated on each file savig operation without need of `main` program being closed and run again.

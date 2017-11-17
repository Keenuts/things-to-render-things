# Rendering things

This is a repo to store all my current rendering-related experiments.
I don't know where I'll stop, just using my daily comute time to learn new things. 

So far, I have:

- raytracer with noisy soft shadows
- pathtracer, monte-carlo method, GPU & CPU
- raytracer with many-lights to add some kind of indirect lighting

## On going task

OpenCL - OpenGL interop to have a GUI with the frame being processed.
Will have to use mapped buffer technique since my device/icd
does not support direct sharing.
status: POC done

![pathtracer](https://raw.githubusercontent.com/Keenuts/things-to-render-things/master/showcase/pathtracing.png "Pathtracer output (samples=512, depth=4)")
![raytracer](https://raw.githubusercontent.com/Keenuts/things-to-render-things/master/showcase/raytracer.png "simple raytracer, hard shadows")
![raytracer indirect](https://raw.githubusercontent.com/Keenuts/things-to-render-things/master/showcase/raytracer-manylights.png "Raytracer output with somekind of indirect lighting")


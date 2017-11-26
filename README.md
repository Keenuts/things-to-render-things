# Rendering things

This is a repo to store all my current rendering-related experiments.
I don't know where I'll stop, just using my daily comute time to learn new things. 

On some old commits, you will find some GPU implem, with a basic OpenCL-OpenGL interop.
Since my hardware/driver does not support OpenCL correctly, I decided to drop GPU support.

So far, I have:

- raytracer with noisy soft shadows
- pathtracer, monte-carlo method
- raytracer with many-lights to add some kind of indirect lighting

## On going task

- Bi-directionnal pathtracing

## On my list

- Metropolis light transport

## Examples

Pathtracing

![pathtracer](https://raw.githubusercontent.com/Keenuts/things-to-render-things/master/showcase/pathtracing.png "Pathtracer output (samples=512, depth=4)")

Simple raytracing

![raytracer](https://raw.githubusercontent.com/Keenuts/things-to-render-things/master/showcase/raytracer.png "simple raytracer, hard shadows")

Raytracing simulating GI using sub-lights

![raytracer indirect](https://raw.githubusercontent.com/Keenuts/things-to-render-things/master/showcase/raytracer-manylights.png "Raytracer output with somekind of indirect lighting")

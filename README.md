
# Procedural mesh examples for Unreal 4

## Overview

I've been wanting to learn how to procedurally generate meshes in Unreal for some time now, and have noticed interest from the community as well.

There have been a few good discussions on the Unreal forums, but not a lot of samples for how to do this.

The purpose of this project is to provide examples for how to generate geometry in code, from simple cubes to more complex geometry generated from fractal algorithms.

I would love for this to become a community project, so I will accept both pull requests for new samples as well as any corrections to my code.  I'm also willing to add contributors directly if anyone is interested (see contact information below).

I wrote the code to be readable, trying my best to explain each step along the way.  There are many ways this code could be optimized to run better, but I wanted to keep things simple in the first few examples.

There is a lot of code duplication between the different examples which I felt was OK for the first few examples, but I will refactor some of the common functions into a utility class soon.

I decided to write this as a plugin, so it can be used in other projects and also provided an example project showing how this is done:
https://github.com/SiggiG/ProceduralMeshDemos/


## Explanation of a few Unreal 4 concepts.

### Vertices, triangles, UV coordinates, normals and tangents.. oh my!
Meshes are created by setting up 3d coordinates and then defining polygons between those.

When creating meshes we enter data into a few different arrays.  All of the arrays are of the same length except the Triangles one, and all of them reference the Vertices array in some way.

##### Vertices
This is a collection of all the points (vectors) in the 3d mesh.

##### Triangles
A triangular polygon is defined by 3 points in space, and this array contains references to what entries in the Vertices array define a polygon.

The order in which we add the vertices governs the way the polygon will face, and if you look at the back of a polygon it will simply be invisible (they are all one sided).

If you add vertices on a polygon in a counter-clockwise order, the polygon will face "towards you".

Example: We have 9 entries in the Vertices array and want to create a polygon from entries 4 to 6.  The Triangles array would then contain the numbers: 3, 4, 5 (zero-based arrays).

##### UV coordinates
A UV coordinate is simply the X,Y coordinate on a texture that the corresponding vertex will have.  The GPU then interpolates how to display the texture in between the vertices.

##### Normals
A normal is a vector that defines the direction a polygon is facing, or in this case the direction the polygon faces where the vertex is positioned.

Normally (pun intended) this can be calculated easily from the vectors in the polygon, but sometimes we want to manipulate this to provide smooth surfaces and other tricks.  See SimpleCylinder for an example of smoothing a curved surface.

##### Tangents
A tangent is a vector that is perpendicular (forms a 90 degree angle) with the normal and is parallell to the polygon surface.

## Geometry examples
I currently provide all the examples as actors because of a limitation with the ProceduralMeshComponent which I am using.

ProceduralMeshComponent (PMC for short) is a useful plugin that ships with the Unreal Engine, but by default the mesh data is serialized which can cause your map sizes to baloon when you have a lot of procedurally generated meshes.

To get around this limitation I forcefully mark the PMC as Transient so none of its data is ever saved.  If I were to create a component that inherits PMC, it would mean all the parameters we set on our mesh would also not be saved.

I want to suggest to Epic to either change the mesh data to have a protected access modifier (so we can set the transient flag in the constructor), or change the default to not save the mesh data.

##### Simple Cube
We start off with a very simple example of drawing a cube where you can set the Depth, Width and Height dimensions.

**TODO Picture**

##### Simple Cylinder
Shows how to draw a cylinder with configurable radius, height and how many polygons to use on the sides.

Shows how to use normals to smooth out a surface, draw polygons on both sides and how to close the ends of the cylinder.

**TODO Picture**

##### Cylinder Strip
In this example we show how you can define multiple points in space and then draw a cylinder section between each point to form a line.

Note that this example does not join the meshes where the lines meet at the corners. I'm planning on providing an example for that later on, both in the form of a quick method (rotated cross sections that skew the cylinder) and a more expensive method that looks correct (projecting a circle on a plane defined by the angle where the lines meet).

**TODO Picture**

##### Sierpinsky Line pyramid
I have always been fascinated by fractals, and they are one of the reasons I wanted to learn how to make custom geometry in Unreal.

All fractals are drawn by a few very simple rules, but can create incredibly complicated results.

One of the first examples of a fractal I learned to draw is the Sierpinski pyramid, which looks like a triangle made out of other triangles!

For this example I went one step further and created a 3d pyramid version of the Sierpinski triangle, drawing it with cylindrical lines in 3d space.

Later on I want to provide an example pyramid drawn with a polygonal surface instead of lines.

**TODO Picture**

##### Branching Lines
Another simple algorhitm that can create complex shapes seen in nature, including trees and lightning.

You start by defining two points in space and draw a line between them. Then add a point in the center of that line, and shift it out in a random direction.  Then repeat this step for the two new sections created and repeat!

**TODO Picture**

## Future work 

##### More examples!
I want to provide more examples in the future, and would love if members of the community could provide some! Of course you will get full credit for your contributions.

- [ ] Joined cylinders (fast method and properly joined).
- [ ] Proper UV maps for Sierpinski pyramid and branching lines
- [ ] Sphere
- [ ] Sierpinski pyramid without lines
- [ ] Mesh exporter function that creates a new static mesh asset in the editor.
- [ ] Proper L-systems
- [ ] Animated lightning
- [ ] Geometry shaders?

##### Usage of PrimitiveSceneProxy/RHI directly instead of PMC
Using the ProceduralMeshComponent has some limitations as mention above.  I also want to provide animated examples in the future where updates are sent to the GPU on every frame, and I think its going to be far more efficient to do that by writing my own UPrimitiveComponent, PrimitiveSceneProxy and RHI rendering implementations.


## Contact info

If you have any questions, suggestions or want to contribute to this project please contact me at one of these places:
* You can reach me via email at siggi@siggig.is
* Send essage me on the Unreal forums on username SiggiG.
* Twitter: https://twitter.com/SiggiGG

## License

By using the code here you accept the **GPL V3** license.

Copyright (c) 2016 Sigurdur Gunnarsson

Feel free to use the code in your own projects, but please submit any useful changes back to here.

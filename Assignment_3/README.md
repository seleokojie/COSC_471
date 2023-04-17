# Triangle Rasterizer with Anti-Aliasing [Eigen3, OpenCV]

COSC 471 - Assignment 3

This program generates two Primitive Triangle overlapped with each other and implements a basic supersampling anti-aliasing algorithm.

## Notable Functions

### ```insideTriangle```
This function takes a point an a triangle in vector form and returns true if the given point lies within the triangle and false otherwise

### ```rasterize_triangle```
This function takes in a triangle and rasterizes it, that is, sets the colors for each pizel in the screen space to most accurately represent the triangle model.

### ```rasterize_triangle (Stratified SuperSampling)```
This function takes in a triangle and a sample rate and rasterizes it, but each pixel is divided into a "grid of samples" which are each checked using an algorithm to produce better rasterization results on bigger screens (This is a form of anti-aliasing)


## Example

```c
const Triangle& t;
if (insideTriangle(0, 0, t.v))
    rasterize_triangle(t, 2);
```
This segment of code checks to see if the origin is inside a triangle `t`. If it is, we rasterize `t` using the anti-aliasing overload of `rasterize_triangle`.

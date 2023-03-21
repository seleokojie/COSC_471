# Triangle Rasterizer [Eigen3, OpenCV]

COSC 471 - Assignment 2

This program generates a Primitive Triangle, rotates it using Eigen3 vectors and matrices, and displays the rotation via OpenCV.


## Notable Functions

### ```get_view_matrix```
This function takes in the camera eye position and returns a 4x4 transformation matrix that maps the world coordinates to the camera coordinates.

### ```get_model_matrix```
This function takes in a rotation angle in degrees and returns a 4x4 transformation matrix that rotates the model around the z-axis.

### ```get_project_matrix```
This function takes in the camera eye field of view, the aspect ratio, and the near and far clipping planes, and returns a 4x4 projection matrix.

### ```get_rotation```
This function takes in an axis and an angle (in degrees) and returns a 4x4 transformation matrix that rotates the model around the given axis by the given angle, going through the origin.

##### Note: All runnable files must be located in the /examples folder


## Example

```c
Eigen:Vector3f axis(0, 1, 0);
Eigen::Matrix4f rotation = get_rotation(axis, 20);

rst::rasterizer r (300, 300);
r.set_model(rotation);

r.set_view(get_view_matrix(eye_pos));
r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
```
This segment of code rotates a 3D object by 20 degrees around the y-axis. 
Then, the points of the object will be transformed by the `model` matrix, then the `view` matrix, and finally the `projection` matrix before being drawn to the screen.


## Demo

[![Triangle Rasterizer](https://res.cloudinary.com/marcomontalbano/image/upload/v1679438123/video_to_markdown/images/video--75fa46df1c7fcfb57937d24b0f00dcf1-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://i.imgur.com/g7fNZE6.mp4 "Triangle Rasterizer")

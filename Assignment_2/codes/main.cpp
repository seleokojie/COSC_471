#include "Triangle.hpp"
#include "rasterizer.hpp"
#include "eigen3/Eigen/Eigen"
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f translate;
    
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle) {
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float angle = rotation_angle * MY_PI / 180.0f;
    model(0, 0) = cos(angle);
    model(0, 1) = -sin(angle);
    model(1, 0) = sin(angle);
    model(1, 1) = cos(angle);

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar){
    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();

    float tan_half_fov = tan(eye_fov / 2.0f * MY_PI / 180.0f);
    projection (0, 0) = 1.0f / (aspect_ratio * tan_half_fov);
    projection (1, 1) = 1.0f / (tan_half_fov);
    projection (2, 2) = -(zFar + zNear) / (zFar - zNear);
    projection (2, 3) = -2.0f * zFar * zNear / (zFar - zNear);
    projection (3, 2) = -1.0f;

    return projection;
}

/*
    'get_rotation' takes an axis and an angle in degrees and
     returns a 4x4 transformation matrix that rotates the model
     around the given axis by the given angle, going through the origin.
*/
Eigen::Matrix4f get_rotation(Vector3f axis, float angle){
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();

    float radian = angle * MY_PI / 180.0f;
    float t = 1 - cos(radian);

    rotation(0, 0) = t * axis[0] * axis[0] + cos(radian);
    rotation(0, 1) = t * axis[0] * axis[1] - sin(radian) * axis[2];
    rotation(0, 2) = t * axis[0] * axis[2] + sin(radian) * axis[1];
    rotation(1, 0) = t * axis[0] * axis[1] + sin(radian) * axis[2];
    rotation(1, 1) = t * axis[1] * axis[1] + cos(radian);
    rotation(1, 2) = t * axis[1] * axis[2] - sin(radian) * axis[0];
    rotation(2, 0) = t * axis[0] * axis[2] - sin(radian) * axis[1];
    rotation(2, 1) = t * axis[1] * axis[2] + sin(radian) * axis[0];
    rotation(2, 2) = t * axis[2] * axis[2] + cos(radian);

    return rotation;
}



int main(int argc, const char** argv){
    Eigen:Vector3f axis(0, 0, 1); //Rotation axis: z-axis
    float angle = 0;
    
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};
    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //This rotation matrix allows you to rotate the model around a specified axis
        Eigen::Matrix4f rotation = get_rotation(axis, angle);
        r.set_model(rotation);

        //r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //This rotation matrix allows you to rotate the model around a specified axis
        Eigen::Matrix4f rotation = get_rotation(axis, angle);
        r.set_model(rotation);

        //r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}

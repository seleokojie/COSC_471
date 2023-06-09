// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions){
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices){
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols){
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f){
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v);

static bool insideTriangle(int x, int y, const Vector3f* _v){   
    // Bounding box
    float minX = std::min(_v[0].x(), std::min(_v[1].x(), _v[2].x()));
    float maxX = std::max(_v[0].x(), std::max(_v[1].x(), _v[2].x()));
    float minY = std::min(_v[0].y(), std::min(_v[1].y(), _v[2].y()));
    float maxY = std::max(_v[0].y(), std::max(_v[1].y(), _v[2].y()));
    if (x < minX || x > maxX || y < minY || y > maxY) {
        return false;
    }

    // Barycentric coordinates
    auto [c1, c2, c3] = computeBarycentric2D(x, y, _v);
    if (c1 < 0 || c2 < 0 || c3 < 0) {
        return false;
    }

    return true;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v){
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type){
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind){
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v){
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i){
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        //rasterize_triangle(t);

        //Rasterize using the supersampling overloaded function
        rasterize_triangle(t, 4);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    int min_x = std::min(std::min(v[0].x(), v[1].x()), v[2].x());
    int max_x = std::max(std::max(v[0].x(), v[1].x()), v[2].x());
    int min_y = std::min(std::min(v[0].y(), v[1].y()), v[2].y());
    int max_y = std::max(std::max(v[0].y(), v[1].y()), v[2].y());

    for (int x = min_x; x <= max_x; x++) {
        for (int y = min_y; y <= max_y; y++) {
            if (insideTriangle(x, y, t.v)) {
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                if (z_interpolated < depth_buf[get_index(x, y)]) {
                    set_pixel(Vector3f(x, y, z_interpolated), t.getColor());
                    depth_buf[get_index(x, y)] = z_interpolated;
                }
            }
        }
    }

}

/*  The following code is for stratified supersampling.
    Note that this code is not optimized for performance.
    You can modify it to make it faster.
*/
void rst::rasterizer::rasterize_triangle(const Triangle& t, int sample_rate) {
    //Convert vertices to homogeneous coordinates
    auto v = t.toVector4();

    //Compute the bounding box of current triangle.
    int min_x = std::floor(std::min(std::min(v[0].x(), v[1].x()), v[2].x()));
    int max_x = std::ceil(std::max(std::max(v[0].x(), v[1].x()), v[2].x()));
    int min_y = std::floor(std::min(std::min(v[0].y(), v[1].y()), v[2].y()));
    int max_y = std::ceil(std::max(std::max(v[0].y(), v[1].y()), v[2].y()));

    int samples_per_pixel = sample_rate * sample_rate;

    //Loop through all pixels in the bounding box
    for (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            //Check if the current pixel is inside the triangle
            if (insideTriangle(x + 0.5, y + 0.5, t.v)) {
                float z_interpolated = 0.0f; 
                float w_reciprocal = 0.0f; 
                Vector3f color_interpolated = Vector3f::Zero();

                //Subdivide the pixel into a grid of samples
                for (int i = 0; i < sample_rate; i++) {
                    for (int j = 0; j < sample_rate; j++) {
                        //Compute the barycentric coordinate of the current sub-pixel
                        float x_sample = x + (i + 0.5) / sample_rate;
                        float y_sample = y + (j + 0.5) / sample_rate;
                        auto[alpha, beta, gamma] = computeBarycentric2D(x_sample, y_sample, t.v);
                        
                        //Interpolate the attributes of the current sub-pixel
                        float w_reciprocal_sample = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        z_interpolated += (alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w()) * w_reciprocal_sample;
                        w_reciprocal += w_reciprocal_sample;
                        color_interpolated += w_reciprocal_sample * t.getColor();
                    }
                }
                //Average the attributes over all sub-pixels
                z_interpolated /= (samples_per_pixel);
                w_reciprocal /= (samples_per_pixel);
                color_interpolated /= (samples_per_pixel);
                
                //Multiply the interpolated attributes by the reciprocal of the interpolated w to get the true interpolated attributes
                z_interpolated *= w_reciprocal;
                color_interpolated *= w_reciprocal;
                
                //Update the depth buffer and the frame buffer if the current pixel is closer to the camera than the former one
                if (z_interpolated < depth_buf[get_index(x, y)]) {
                    set_pixel(Vector3f(x, y, z_interpolated), color_interpolated);
                    depth_buf[get_index(x, y)] = z_interpolated;
                }
            }
        }
    }
}


void rst::rasterizer::set_model(const Eigen::Matrix4f& m){
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v){
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p){
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff){
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h){
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y){
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color){
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on
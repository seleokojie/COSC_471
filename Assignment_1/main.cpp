#include<cmath>
/**
 * @file main.cpp
 * @brief 2D Rotation Matrix
 * @details This program rotates a 2D point by a given angle using a rotation matrix.
 * @author [Sele Okojie]
 * @date 2023-02-26
**/

#include"eigen3/Eigen/Core"
#include"eigen3/Eigen/Dense"
#include<iostream>

int main(){
    std::cout << "Original Point: ";
    Eigen::Vector2f p(2.0f, 1.0f); // 2D point
    std::cout << "(" << p(0) << ", " << p(1) << ")" << std::endl;

    float theta = 45.0f * M_PI / 180.0f; // 45 degrees in radians
    Eigen::Matrix2f R;
    R << cos(theta), -sin(theta), sin(theta), cos(theta); // Rotation Matrix
    Eigen::Vector2f p_rotated = R * p; //The rotation matrix is multiplied by the point to rotate it.
    printf("Rotated Point: " "(%.2f, %.2f) \n", p_rotated(0), p_rotated(1));
    return 0; 
} // main
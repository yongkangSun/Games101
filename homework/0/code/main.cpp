#include<cmath>
#include<F://computer graphics//eigen//eigen3//Eigen//Core>
#include<F://computer graphics//eigen//eigen3//Eigen//Dense>
#include<iostream>
#define M_PI 3.1415926

using namespace std;
using namespace Eigen;

int main(){
    double degree;
    cin >> degree;
    double sin_deg = sin(degree * M_PI / 180);
    double cos_deg = cos(degree * M_PI / 180);
    Vector3f v1(2, 1, 0);
    Matrix3f m_rotation;
    m_rotation << cos_deg, sin_deg, 0, -1*sin_deg, cos_deg, 0, 0, 0, 0;
    Vector3f v2 = v1.transpose() * m_rotation;
    cout << v2;
    return 0;
}
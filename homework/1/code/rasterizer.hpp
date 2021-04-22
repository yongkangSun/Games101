#pragma once

#include "Triangle.hpp"
#include <algorithm>
#include <E://graphics_lib_cpp//eigen//eigen3//Eigen//Eigen>

using namespace Eigen;
using namespace std;

namespace rst {
enum class Buffers
{
    Color = 1,
    Depth = 2
};

inline Buffers operator|(Buffers a, Buffers b)
{
    return Buffers((int)a | (int)b);
}

inline Buffers operator&(Buffers a, Buffers b)
{
    return Buffers((int)a & (int)b);
}

enum class Primitive
{
    Line,
    Triangle
};

/*
 * For the curious : The draw function takes two buffer id's as its arguments.
 * These two structs make sure that if you mix up with their orders, the
 * compiler won't compile it. Aka : Type safety
 * */

struct pos_buf_id
{
    int pos_id = 0;
};

struct ind_buf_id
{
    int ind_id = 0;
};

class rasterizer
{
  public:
    //构造器
    rasterizer(int w, int h);

    pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
    ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);

    //传递三个MVP矩阵参数
    void set_model(const Eigen::Matrix4f& m);
    void set_view(const Eigen::Matrix4f& v);
    void set_projection(const Eigen::Matrix4f& p);

    //将屏幕像素点(x,y)设为(r,g,b)的颜色, 并写入缓冲区
    void set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color);
    //清理缓冲区
    void clear(Buffers buff);

    void draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, Primitive type);

    //获取frame_buffer
    std::vector<Eigen::Vector3f>& frame_buffer() { return frame_buf; }

  private:
    void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);
    void rasterize_wireframe(const Triangle& t);

  private:
    //给定一个像素在屏幕上的位置，返回他在buffer中的位置
    int get_index(int x, int y) const;
    //维护一个顺序生成序列
    int get_next_id() { return next_id++; }


  private:
    //三个变换矩阵，MVP变换
    Eigen::Matrix4f model;
    Eigen::Matrix4f view;
    Eigen::Matrix4f projection;

    //世界视图中的模型信息，包括顶点坐标pos和顶点顺序ind
    std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
    std::map<int, std::vector<Eigen::Vector3i>> ind_buf;

    //帧缓冲对象，存储屏幕上绘制的全部点的颜色信息 深度信息
    std::vector<Eigen::Vector3f> frame_buf;
    std::vector<float> depth_buf;

    int width, height;
    int next_id = 0;
};
} // namespace rst

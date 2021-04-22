#include <algorithm>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdexcept>

using namespace rst;
using namespace std;
using namespace Eigen;

rst::pos_buf_id rst::rasterizer::load_positions(const vector<Vector3f> &positions)
{
    pos_buf_id id;
    id.pos_id = get_next_id();
    pos_buf.emplace(id.pos_id, positions);
    return id;
}

rst::ind_buf_id rst::rasterizer::load_indices(const vector<Vector3i> &indices)
{
    ind_buf_id id;
    id.ind_id = get_next_id();
    ind_buf.emplace(id.ind_id, indices);
    return {id};
}

// Bresenham's line drawing algorithm
// Code taken from a stack overflow answer: https://stackoverflow.com/a/16405254
void rst::rasterizer::draw_line(Eigen::Vector3f begin, Eigen::Vector3f end)
{
    auto x1 = begin.x();
    auto y1 = begin.y();
    auto x2 = end.x();
    auto y2 = end.y();

    Eigen::Vector3f line_color = {255, 255, 255};

    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

    dx=x2-x1;
    dy=y2-y1;
    dx1=fabs(dx);
    dy1=fabs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;

    if(dy1<=dx1)
    {
        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
        }
        Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
        set_pixel(point,line_color);
        for(i=0;x<xe;i++)
        {
            x=x+1;
            if(px<0)
            {
                px=px+2*dy1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y=y+1;
                }
                else
                {
                    y=y-1;
                }
                px=px+2*(dy1-dx1);
            }
//            delay(0);
            Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
            set_pixel(point,line_color);
        }
    }
    else
    {
        if(dy>=0)
        {
            x=x1;
            y=y1;
            ye=y2;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
        }
        Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
        set_pixel(point,line_color);
        for(i=0;y<ye;i++)
        {
            y=y+1;
            if(py<=0)
            {
                py=py+2*dx1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x=x+1;
                }
                else
                {
                    x=x-1;
                }
                py=py+2*(dx1-dy1);
            }
//            delay(0);
            Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
            set_pixel(point,line_color);
        }
    }
}

//将三维向量中的点转化为四元数
Vector4f to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}



void rst::rasterizer::draw(rst::pos_buf_id pos_buffer, rst::ind_buf_id ind_buffer, rst::Primitive type)
{
    if (type != rst::Primitive::Triangle)
    {
        throw std::runtime_error("Drawing primitives other than triangle is not implemented yet!");
    }

    //从buffer中拿到positions和indices
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];

    //不太明白在做什么
    float f1 = (100 - 0.1) / 2.0;
    float f2 = (100 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;

    for (auto& i : ind)
    {
        Triangle t;

        //根据index标注的顺序对buffer中的顶点进行mvp变换
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };

        //最后一位归一化，化成四元数的形式
        for (auto & vec : v) {
            vec /= vec.w();
        }

        //不太明白，复习完成再说，大概是投影到屏幕上
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        //将做完变换后的屏幕上的信息存储到三角形类中
        for (int j = 0; j < 3; ++j)
        {
            t.setVertex(j, v[j].head<3>());
            t.setVertex(j, v[j].head<3>());
            t.setVertex(j, v[j].head<3>());
        }

        t.setColor(0, 255.0,  0.0,  0.0);
        t.setColor(1, 0.0  ,255.0,  0.0);
        t.setColor(2, 0.0  ,  0.0,255.0);

        rasterize_wireframe(t);
    }
}

//绘制三角形，通过花三条边
void rst::rasterizer::rasterize_wireframe(const Triangle& t)
{
    draw_line(t.c(), t.a());
    draw_line(t.c(), t.b());
    draw_line(t.b(), t.a());
}

//三个MVP矩阵的数据导入函数
void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}
void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}
void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

//清理两个缓存的方法
void rst::rasterizer::clear(rst::Buffers buff)
{
    //监测buff是否是全1，满足条件颜色缓存全部放0
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    //监测buff是否是全1，满足条件深度缓存全部放inf
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

//rasterizer的构造器
rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

//给定一个像素在屏幕上的位置，返回他在buffer中的位置
int rst::rasterizer::get_index(int x, int y) const
{
    return (height - y) * width + x;
}

//根据屏幕上点坐标计算出对应index，并在color_buffer中给对应点颜色赋值
void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    if (point.x() < 0 || point.x() >= (float)width ||
        point.y() < 0 || point.y() >= (float)height) return;
    //auto ind = ((float)height - point.y()) * (float)width + point.x();
    auto ind = get_index((int)point.x(), (int)point.y());
    frame_buf[ind] = color;
}


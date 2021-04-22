#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <E://graphics_lib_cpp//eigen//eigen3//Eigen//Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate <<    1, 0, 0, -eye_pos[0],
                    0, 1, 0, -eye_pos[1],
                    0, 0, 1, -eye_pos[2],
                    0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(Eigen::Vector3f axis, Eigen::Vector3f pos, float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float radian = rotation_angle / 180 * MY_PI;
    float c = cos(radian);
    float s = sin(radian);
    float a_x = axis.x();
    float a_y = axis.y();
    float a_z = axis.z();

    Eigen::Matrix4f rotate;
    rotate <<   c + (1-c) * a_x * a_x, (1-c) * a_x * a_y - s * a_z, (1-c) * a_x * a_z + s * a_y, 0,
                (1-c) * a_x * a_y + s * a_z, c + (1-c) * a_y * a_y, (1-c) * a_y * a_z - s * a_x, 0,
                (1-c) * a_x * a_y - s * a_y, (1-c) * a_y * a_z + s * a_x, c + (1-c) * a_z * a_z, 0,
                0, 0, 0, 1;

    Eigen::Matrix4f translate_to, translate_back;
    translate_to << 1, 0, 0, -1 * pos.x(),
                    0, 1, 0, -1 * pos.y(),
                    0, 0, 1, -1 * pos.z(),
                    0, 0, 0, 1;
    translate_back = translate_to.inverse();
    model = model * translate_back * rotate * translate_to;
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    //  eye_fov : 视角
    //  aspect_ratio : 长宽比
    //  zNear : 近点z
    //  zFar : 远点z

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    float height = 2 * std::tan(eye_fov / 2 / 180 * MY_PI) * abs(zNear);
    float width = height * aspect_ratio;

    float yTop = height / 2;
    float yBottom = -1 * yTop;
    float xRight = width / 2;
    float xLeft = -1 * xRight;

    Eigen::Matrix4f M_orthographic = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f M_orthographic_translate;
    Eigen::Matrix4f M_orthographic_scale;
    M_orthographic_translate << 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, -1 * (zNear + zFar) / 2,
                                0, 0, 0, 1;
    M_orthographic_scale <<     2 / (xRight - xLeft), 0, 0, 0,
                                0, 2 / (yTop - yBottom), 0, 0,
                                0, 0, 2 / (zNear - zFar), 0,
                                0, 0, 0, 1;
    M_orthographic = M_orthographic * M_orthographic_scale * M_orthographic_translate;

    Eigen::Matrix4f M_perspective;
    M_perspective <<    zNear, 0, 0, 0,
                        0, zNear, 0, 0,
                        0, 0, zNear + zFar, -1 * zNear * zFar,
                        0, 0, 1, 0;

    projection = projection * M_orthographic * M_perspective;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "F://computer graphics//Games101//homework//1//code//output.png";

    /*if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }*/

    int choice;
    std::cout << "single test rotate any angle : 0" << endl << "multiple test rotation : 1" << endl;
    std::cin >> choice;
    if(choice == 0)
    {
        command_line = true;
        std::cout << "please input an angle : ";
        std::cin >> angle;
    }
    else
        command_line = false;


    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    Eigen::Vector3f base_pos = {0, 0, 0};
    for(auto & po : pos)
    {
        base_pos.x() += po.x();
        base_pos.y() += po.y();
        base_pos.z() += po.z();
    }
    base_pos = base_pos / 3;


    //实时绘制三角形读取键盘的控制命令
    int key = 0;
    int frame_count = 0;

    //如果给定旋转度数，只绘制一次
    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        Eigen::Vector3f z;
        z << 0, 0, 1;
        r.set_model(get_model_matrix(z, base_pos, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    //通过a,d控制绘制旋转的三角形（esc退出）
    float angle_x = 0;
    float angle_y = 0;
    float angle_z = 0;
    int mode = -1;
    Eigen::Vector3f axis_x, axis_y, axis_z;
    axis_x << 1, 0, 0;
    axis_y << 0, 1, 0;
    axis_z << 0, 0, 1;
    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        if(mode == 0)
            r.set_model(get_model_matrix(axis_x, base_pos, angle_x));
        else if(mode == 1)
            r.set_model(get_model_matrix(axis_y, base_pos, angle_y));
        else
            r.set_model(get_model_matrix(axis_z, base_pos, angle_z));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(5);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle_y += 10;
            mode = 1;
        }
        else if (key == 'd') {
            angle_y -= 10;
            mode = 1;
        }
        else if (key == 'w') {
            angle_x += 10;
            mode = 0;
        }
        else if (key == 's') {
            angle_x -= 10;
            mode = 0;
        }
        else if (key == 'q') {
            angle_z += 10;
            mode = 2;
        }
        else if (key == 'e') {
            angle_z -= 10;
            mode = 2;
        }
    }

    return 0;
}

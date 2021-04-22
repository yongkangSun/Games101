#include <iostream>
#include <opencv2/core/core.hpp>//包含openCV的基本数据结构，数组操作的基本函数
#include <opencv2/highgui/highgui.hpp>//图像的交互界面，视频的捕捉也可写为#include <opencv2/highgui.hpp>
using namespace std;
//using namespace cv;

int main() {

    cout << "now i will open image!" << endl;

    cv::Mat image;

    image = cv::imread("F://computer graphics//Games101//homework//1//opencvTest//konata.jpeg");//打开图像

    if (image.empty()) {
        cout << "Read Error" << endl;
    }

    cout << "This image is " << image.rows << " x " << image.cols << endl;

    namedWindow("Original Image", cv::WINDOW_AUTOSIZE);//若改成WINDOW_NORMAL可以使用鼠标调节图片大小，

    imshow("Original Image", image);

    cv::waitKey(0);

    return 0;
}
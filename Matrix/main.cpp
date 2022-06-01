// Matrix.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "BaseMatrix.h"
#include <time.h>

class CalculatingTime {
public:
    void getStartTime() {
        start_time = clock();
    }

    void getEndTime() {
        end_time = clock();
    }

    void Times() {
        times = ((double)end_time - (double)start_time) / CLOCKS_PER_SEC;
        std::cout << " 花费的时间 = " << times << std::endl;
    }
private:
    clock_t start_time, end_time, times;
};

int main()
{
    std::cout << "Hello World!\n";
    /*double a[3] = { 1, 2, 3 };
    double b[3][3] = { {3, 2, 1}, {3, 2, 1}, {3, 2, 1} };

    CalculatingTime MyTime;

    MyTime.getStartTime();
    Matrix* MyMatrix = new Matrix(3, 3);
    MyMatrix->setByArray(a);
    std::cout << *MyMatrix << std::endl;
    for (int i = 0; i < 10000; i++);
    MyTime.getEndTime();
    MyTime.Times();


    MyTime.getStartTime();
    Matrix MyMatrix001(3, 3);
    MyMatrix001.setByArray(a);
    std::cout << MyMatrix001 << std::endl;
    MyTime.getEndTime();
    MyTime.Times();*/

    /*double array_01[] = { 1,2,3,4 };
    for (int i = 0; i < 4; i++) {
        std::cout << array_01[i] << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
    double array_02[10] = { 1,2,3,4 };
    for (int i = 0; i < 4; i++) {
        std::cout << array_02[i] << std::endl;
    }*/

    double* array_03;
    array_03 = new double[3];  // double* array_04 = new double[4];

    array_03[0] = 1;
    array_03[1] = 2;
    array_03[2] = 3;
    // array_03[3] = 4;   // 这是错的，溢出了
    for (int i = 0; i < 3; i++) {
        std::cout << array_03[i] << std::endl;
    }


    // 用指针的指针定义一个二维数组
    double** array_04;
    array_04 = new double* [10];
    for (int i = 0; i < 10; i++) {
        array_04[i] = new double[10];
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array_04[i][j] = 1.0;
        }
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            std::cout << array_04[i][j] << std::endl;
        }
    }

    Matrix MyMatrix(3, 3);
    Matrix test = MyMatrix.identity(4, 4);

    std::cout << test << std::endl;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

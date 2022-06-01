
// #pragma once

#ifndef  _BASEMATRIX_H_
#define _BASEMATRIX_H_

#include <iostream>
using namespace std;

// 在Matrix类中使用到的模板命名：TT识别二维数组类型；T识别一维数组类型

class Matrix {

	// 矩阵类友元函数
	template<typename TT>
	friend Matrix to_matrix(TT array, int row, int column);
	friend Matrix to_matrix(double* array, int column);//将二维数组转化为矩阵类，需要指明数组的行和列 

	//
	friend Matrix operator*(double coef, Matrix& mat);
	friend Matrix operator*(Matrix& mat, double coef);
	friend istream& operator>>(istream& in, Matrix& mat);
	friend ostream& operator<<(ostream& out, Matrix& mat);//不可置于类内的运算符重载

	//
public:
	Matrix(int column);
	Matrix(int row, int column);
	Matrix(const Matrix& mat);
	//
	Matrix zeros(int row, int column); // 创建任意大小的零矩阵
	Matrix identity(int row, int column); // 创建一个单位矩阵
	Matrix inverse(Matrix& mat); // 矩阵求逆
	~Matrix();

public:
	static double Min(Matrix& mat); //求矩阵元素中的最小值
	static double Max(Matrix& mat); //
	static double Sum(Matrix& mat);
	static Matrix Exp(Matrix& mat);

	//
	const Matrix operator-()const;
	Matrix operator+(Matrix& mat);
	Matrix operator-(Matrix& mat);
	Matrix operator*(Matrix& mat);
	bool operator==(Matrix& mat);
	bool operator!=(Matrix& mat);

	template<typename TT>
	void setByArray(TT array);
	void setByArray(double* array); //通过数组设置矩阵的值

	Matrix get_add_with(Matrix& mat);
	Matrix get_sub_with(Matrix& mat);
	Matrix get_mul_with(Matrix& mat);
	Matrix get_mul_with(double coef); // 矩阵的数学运算

	double at(int column);
	double at(int row, int column); // 返回矩阵中对应元素的标
	void input();					// 从键盘中输入矩阵
	void shape();					// 输出矩阵的形状
	void shape(string matrixName);  // 自定义名字输出
	void print();                   // 打印矩阵
	void print(string matrixName);  // 

private:
	double** matrix_;
	int row_;
	int column_;
};

// 类内模板函数的定义
template<typename TT> // 使用类内模板自动推导数组类型
void Matrix::setByArray(TT array) {
	for (int i = 0; i < row_; i++) {
		for (int j = 0; j < column_; j++) {
			matrix_[i][j] = array[i][j];
		}
	}
}

// 全局模板函数定义及声明
template<typename TT>
Matrix to_matrix(TT array, int row, int column) {
	Matrix mat(row, column);

	try {
		if (mat.row_ != row || mat.column_ != column) {
			throw " row or column error";
		}
		for (int i = 0; i < mat.row_; i++) {
			for (int j = 0; j < mat.column_; j++) {
				mat.matrix_[i][j] = array[i][j];
			}
		}
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return mat;
}

Matrix to_matrix(double* array, int column);

#endif // ! _BASEMATRIX_H_



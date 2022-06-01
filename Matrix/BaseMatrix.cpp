// 
// 
#include <iostream>
#include <cmath>
#include "BaseMatrix.h"

using namespace std;

// 类的构造和析构
// 定义一个一维数组
Matrix::Matrix(int column) {
	row_ = 1;
	column_ = column;
	matrix_ = new double* [1];            // 这句话是什么意思？ 用二维数组定义矩阵，指针的指针的定义二维数组
	matrix_[0] = new double[column];      // 
	for (int j = 0; j < column; j++) {
		matrix_[0][j] = 0.0;
	}
}


// 定义一个二维数组
Matrix::Matrix(int row, int column) {
	row_ = row;
	column_ = column;
	matrix_ = new double* [row];

	for (int i = 0; i < row; i++) {
		matrix_[i] = new double[column];
		for (int j = 0; j < column; j++) {
			matrix_[i][j] = 0.0;
		}
	}
}


// 根据现有的矩阵创建一个一样大小的矩阵
Matrix::Matrix(const Matrix& mat) {
	row_ = mat.row_;
	column_ = mat.column_;
	matrix_ = new double* [mat.row_];

	for (int i = 0; i < mat.row_; i++) {
		matrix_[i] = new double[mat.column_];
		for (int j = 0; j < mat.column_; j++) {
			matrix_[i][j] = mat.matrix_[i][j];
		}
	}
}


// 
Matrix Matrix::zeros(int row, int column) {
	Matrix res(row, column);
	res.row_ = row;
	res.column_ = column;
	for (int i = 0; i < res.row_; i++) {
		res.matrix_[i] = new double[column];
		for (int j = 0; j < res.column_; j++) {
			res.matrix_[i][j] = 0.0;
		}
	}
	return res;
}

Matrix Matrix::identity(int row, int column) {
	Matrix res(row, column);
	res.row_ = row;
	res.column_ = column;
	for (int i = 0; i < res.row_; i++) {
		res.matrix_[i] = new double[column];
		for (int j = 0; j < res.column_; j++) {
			if (i == j) {
				res.matrix_[i][j] = 1;
			}
			else {
				res.matrix_[i][j] = 0.0;
			}
		}
	}
	return res;
}

Matrix Matrix::inverse(Matrix& mat) {

	if (mat.row_ != mat.column_) {
		std::cout << " 矩阵不是方阵 " << std::endl;
	}
	Matrix res(mat.row_, mat.column_);

}

//
Matrix::~Matrix() {
	for (int i = 0; i < row_; i++) {
		delete[] matrix_[i];
	}
}


// 类静态成员函数定义
double Matrix::Min(Matrix& mat) {
	double min = mat.matrix_[0][0];
	for (int i = 0; i < mat.row_; i++) {
		for (int j = 0; j < mat.column_; j++) {
			if (mat.matrix_[i][j] < min) {
				min = mat.matrix_[i][j];
			}
		}
	}
	return min;
}

double Matrix::Max(Matrix& mat) {
	double max = mat.matrix_[0][0];
	for (int i = 0; i < mat.row_; i++) {
		for (int j = 0; j < mat.column_; j++) {
			if (mat.matrix_[i][j] > max) {
				max = mat.matrix_[i][j];
			}
		}
	}
	return max;
}

// 
double Matrix::Sum(Matrix& mat) {
	double sum = 0;
	for (int i = 0; i < mat.row_; i++) {
		for (int j = 0; j < mat.column_; j++) {
			sum += mat.matrix_[i][j];
		}
	}
	return sum;
}

// 
Matrix Matrix::Exp(Matrix& mat) {
	Matrix res = mat;

	for (int i = 0; i < mat.row_; i++) {
		for (int j = 0; j < mat.column_; j++) {
			res.matrix_[i][j] = exp(mat.matrix_[i][j]);
		}
	}
	return res;
}

// 类 public 类接口类函数定义
void Matrix::setByArray(double* array) {
	for (int i = 0; i < column_; i++) {
		matrix_[0][i] = array[i];
	}
}

Matrix Matrix::get_add_with(Matrix& mat) {
	Matrix res = mat;
	try {
		if (row_ != mat.row_ || column_ != mat.column_) {
			throw " row or column of a matrix is error";
		}
		for (int i = 0; i < row_; i++) {
			for (int j = 0; j < column_; j++) {
				res.matrix_[i][j] = matrix_[i][j] + mat.matrix_[i][j];
			}
		}
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return res;
}

Matrix Matrix::get_sub_with(Matrix& mat) {
	Matrix res = mat;
	try {
		if (row_ != mat.row_ || column_ != mat.column_) {
			throw " row or column of a matrix is error";
		}
		for (int i = 0; i < row_; i++) {
			for (int j = 0; j < column_; j++) {
				res.matrix_[i][j] = matrix_[i][j] - mat.matrix_[i][j];
			}
		}
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return res;
}

//
Matrix Matrix::get_mul_with(Matrix& mat) {
	Matrix res(row_, mat.column_);

	try {
		if (column_ != mat.row_) {
			throw " the column of first matrix or the of second matrix is error";
		}
		for (int i = 0; i < row_; i++) {
			for (int j = 0; j < mat.column_; j++) {
				int sum = 0;
				for (int k = 0; k < column_; k++) {
					sum += matrix_[i][k] * mat.matrix_[k][j];
				}
				res.matrix_[i][j] = sum;
			}
		}
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return res;
}


//
Matrix Matrix::get_mul_with(double coef) {
	Matrix res = *this;
	for (int i = 0; i < row_; i++) {
		for (int j = 0; j < column_; j++) {
			res.matrix_[i][j] = res.matrix_[i][j] * coef;
		}
	}
	return res;
}

//
const Matrix Matrix::operator-()const {
	Matrix res = *this;

	for (int i = 0; i < res.row_; i++) {
		for (int j = 0; j < res.column_; j++) {
			res.matrix_[i][j] = -res.matrix_[i][j];
		}
	}
	return res;
}

// 
Matrix Matrix::operator+(Matrix& mat) {
	return Matrix(get_add_with(mat));
}

//
Matrix Matrix::operator-(Matrix& mat) {
	return Matrix(get_sub_with(mat));
}

//
Matrix Matrix::operator*(Matrix& mat) {
	return Matrix(get_mul_with(mat));
}

//
bool Matrix::operator==(Matrix& mat) {
	bool isSame = true;
	if (row_ != mat.row_ || column_ != mat.column_) {
		isSame = false;
	}
	for (int i = 0; i < row_; i++) {
		for (int j = 0; j < column_; j++) {
			if (matrix_[i][j] != mat.matrix_[i][j]) {
				isSame = false;
				break;
			}
		}
	}
	return isSame;
}

//
bool Matrix::operator!=(Matrix& mat) {
	return !(*this == mat);
}

double Matrix::at(int column) {
	int element = 0;
	try {
		if (column < 0 || column >column_ - 1 || row_ != 1) {
			throw "index error";
		}
		element = matrix_[0][column];
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return element;
}

// 
double Matrix::at(int row, int colunm) {
	int element = 0;
	try {
		if (row<0 || row > row_ - 1 || colunm <0 || colunm >column_ - 1) {

		}
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return element;
}

void Matrix::shape() {
	std::cout << '(' << row_ << ',' << column_ << ')' << std::endl;
}

void Matrix::shape(string MatrixName) {
	std::cout << MatrixName << ":" << '(' << row_ << ',' << column_ << ')' << std::endl;

}

//
void Matrix::print() {
	for (int i = 0; i < row_; i++) {
		std::cout << '[';
		for (int j = 0; j < column_; j++) {
			if (j == 0) {
				std::cout << matrix_[i][j];
			}
			else {
				std::cout << "," << matrix_[i][j];
			}
		}
		std::cout << ']' << std::endl;
	}
}


// 
void Matrix::input() {
	for (int i = 0; i < row_; i++) {
		for (int j = 0; j < column_; j++) {
			std::cin >> matrix_[i][j];
		}
	}
}


//
void Matrix::print(string matrixName) {
	std::cout << matrixName << " = " << std::endl;
	for (int i = 0; i < row_; i++) {
		cout << '[';
		for (int j = 0; j < column_; j++) {
			if (j == 0) {
				std::cout << matrix_[i][j];
			}
			else {
				std::cout << "," << matrix_[i][j];
			}
		}
		std::cout << ']';
		std::cout << endl;
	}
}


//
// 全局函数定义
Matrix to_matrix(double* array, int column) {
	Matrix mat(1, column);
	try {
		if (mat.column_ != column) {
			throw "row or column error";
		}
		for (int i = 0; i < mat.column_; i++) {
			mat.matrix_[0][i] = array[i];
		}
	}
	catch (const char* errorMSG) {
		std::cout << errorMSG << std::endl;
	}
	return mat;
}


// 
Matrix operator*(double coef, Matrix& mat) {
	return Matrix(mat.get_mul_with(coef));
}

Matrix operator*(Matrix& mat, double coef) {
	return Matrix(mat.get_mul_with(coef));
}

istream& operator >> (istream& in, Matrix& mat) {
	mat.input();
	return in;
}

ostream& operator << (ostream& out, Matrix& mat) {
	mat.print();
	return out;
}



#include <cmath>
#include "math.h"
#include "constants.hpp"

double radian(const double theta){
	return theta * (constants::pi / 180);
};

Matrix::Matrix(const Matrix &item) : Matrix(item.dimensions, item.values)
{};

Matrix::Matrix(Matrix &&item)
{
	this->dimensions[0] = item.dimensions[0];
	this->dimensions[1] = item.dimensions[1];
	this->values = item.values;	
	item.values = nullptr;
};

Matrix::Matrix(const unsigned short dimensions[2])
{
	this->dimensions[0] = dimensions[0];
	this->dimensions[1] = dimensions[1];
	this->values = new double[dimensions[0] * dimensions[1]]();
};

Matrix::Matrix(const unsigned short dimensions[2], const double *values) : Matrix(dimensions)
{
	for (unsigned int ptr_offset = 0; ptr_offset < dimensions[0] * dimensions[1]; ptr_offset++)
	{
		*(this->values + ptr_offset) = *(values + ptr_offset);
	};
};

Matrix::~Matrix()
{
	if (this->values != nullptr)
	{
		delete[] this->values;
	};
};

Matrix Matrix::operator+(const Matrix &other) const
{
	if (this->dimensions[0] != other.dimensions[0] || this->dimensions[1] != other.dimensions[1])
	{
		throw errors::MatrixDimensionError();
	};
	Matrix result(*this);
	for (unsigned int ptr_offset = 0; ptr_offset < this->dimensions[0] * this->dimensions[1]; ptr_offset++)
	{
		*(result.values + ptr_offset) += *(other.values + ptr_offset);
	};
	return result;
};

Matrix Matrix::operator+(const double other) const
{
	Matrix result(*this);
	for (unsigned int ptr_offset = 0; ptr_offset < this->dimensions[0] * this->dimensions[1]; ptr_offset++)
	{
		*(result.values + ptr_offset) += other;
	};
	return result;
};

Matrix Matrix::operator-() const
{
	Matrix result(*this);
	for (unsigned int ptr_offset = 0; ptr_offset < this->dimensions[0] * this->dimensions[1]; ptr_offset++)
	{
		*(result.values + ptr_offset) += -*(result.values + ptr_offset);
	};
	return result;
};

Matrix Matrix::operator-(const Matrix &other) const
{
	Matrix reversed = -other;
	return *this + reversed;
};

Matrix Matrix::operator-(const double other) const
{
	return *this + -other;
};

Matrix Matrix::operator*(const Matrix &other) const
{
	if (this->dimensions[1] != other.dimensions[0])
	{
		throw errors::MatrixDimensionError();
	};
	unsigned short result_dimensions[2] = {this->dimensions[0], other.dimensions[1]};
	Matrix result(result_dimensions);
	int result_ptr_offset, this_ptr_offset, other_ptr_offset;
	for (short result_row = 0; result_row < result_dimensions[0]; result_row++)
	{
		for (short result_column = 0; result_column < result_dimensions[1]; result_column++)
		{
			result_ptr_offset = result_row * result_dimensions[1] + result_column;
			for (short index = 0; index < this->dimensions[1]; index++)
			{
				this_ptr_offset = result_row * this->dimensions[1] + index;
				other_ptr_offset = index * other.dimensions[1] + result_column;
				*(result.values + result_ptr_offset) += *(this->values + this_ptr_offset) * *(other.values + other_ptr_offset);
			};
		};
	};
	return result;
};

Vector Matrix::operator*(const Vector &other) const
{
	unsigned short vector_matrix_dimensions[2] = {other.dimensions, 1};
	Matrix vector_matrix(vector_matrix_dimensions, other.values);
	Matrix result_matrix = *this * vector_matrix;
	Vector result = Vector(result_matrix.dimensions[0], result_matrix.values);
	return result;
};

Matrix Matrix::operator*(const double other) const
{
	Matrix result(*this);
	for (unsigned int ptr_offset = 0; ptr_offset < this->dimensions[0] * this->dimensions[1]; ptr_offset++)
	{
		*(result.values + ptr_offset) *= other;
	};
	return result;
};

Matrix Matrix::operator/(const double other) const
{
	return *this * (1.0 / other);
};

Matrix Matrix::transpose() const
{
	Matrix result(this->dimensions);
	unsigned short row, column;
	for (unsigned int ptr_offset = 0; ptr_offset < this->dimensions[0] * this->dimensions[1]; ptr_offset++)
	{
		row = ptr_offset / this->dimensions[1];
		column = ptr_offset % this->dimensions[1];
		*(result.values + column + row * this->dimensions[1]) = *(this->values + ptr_offset);
	};
	return result;
};

Vector::Vector(const Vector &item) : Vector(item.dimensions, item.values)
{};

Vector::Vector(const unsigned short dimensions, const double values[])
{
	this->dimensions = dimensions;
	this->values = new double[dimensions]();
	for (unsigned short index = 0; index < dimensions; index++)
	{
		this->values[index] = values[index];
	};
};

Vector::Vector(Vector &&item)
{
	this->dimensions = item.dimensions;
	this->values = item.values;
	item.values = nullptr;
};

Vector::~Vector()
{
	if (this->values != nullptr)
	{
		delete[] this->values;
	};
};

Vector Vector::operator+(const Vector &other) const
{
	if (this->dimensions != other.dimensions)
	{
		throw errors::VectorDimensionError();
	};
	Vector result(*this);
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result.values[index] += other.values[index];
	};
	return result;
};

Vector Vector::operator+(const double other) const
{
	Vector result(*this);
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result.values[index] += other;
	};
	return result;
};

Vector Vector::operator-() const
{
	Vector result = *this;
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result.values[index] = -result.values[index];
	};
	return result;
};

Vector Vector::operator-(const Vector &other) const
{
	Vector reversed = -other;
	return *this + reversed;
};

Vector Vector::operator-(const double other) const
{
	return *this + -other;
};

Vector Vector::operator*(const Vector &other) const
{
	if (this->dimensions != 3 || other.dimensions != 3)
	{
		throw errors::VectorDimensionError();
	};
	Vector result(*this);
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result.values[index] = this->values[(index + 1) % 3] * other.values[(index + 2) % 3] - this->values[(index + 2) % 3] * other.values[(index + 1) % 3];
	};
	return result;
};

Vector Vector::operator*(const double other) const
{
	Vector result(*this);
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result.values[index] *= other;
	};
	return result;
};

Vector Vector::operator/(const double other) const
{
	return *this * (1.0 / other);
};

double Vector::dot(const Vector &other) const
{
	if (this->dimensions != other.dimensions)
	{
		throw errors::VectorDimensionError();
	};
	double result = 0;
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result += this->values[index] * other.values[index];
	};
	return result;
};

double Vector::length() const
{
	double result_squared = 0;
	for (unsigned short index = 0; index < this->dimensions; index++)
	{
		result_squared += this->values[index] * this->values[index];
	};
	return sqrt(result_squared);
};

Vector Vector::length(const int scale) const
{
	return *this / (this->length() * scale);
};

double Vector::angle(const Vector &other) const
{
	return acos(this->dot(other) / (this->length() * other.length()));
};
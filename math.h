#pragma once

class Vector
{
	public:
		unsigned short dimensions = 0;
		double *values = nullptr;

		Vector(const Vector &);
		Vector(unsigned short, double []);
		Vector operator+(Vector &);
		Vector operator+(double);
		Vector operator-();
		Vector operator-(Vector &);
		Vector operator-(double);
		Vector operator*(Vector &);
		Vector operator*(double);
		Vector operator/(double);
		double dot(Vector &);
		double length();
		Vector length(int);
		double angle(Vector &);
};

class Matrix
{
	public:
		unsigned short dimensions[2];
		double *values = nullptr; // "*" creates a ptr pointing at an array with two dimensions.

		Matrix(const Matrix &);
		Matrix(unsigned short [2]);
		Matrix(unsigned short [2], double *); // "*" is actually a ptr pointing at the first element of an array with two dimensions.
		Matrix operator+(Matrix &);
		Matrix operator+(double);
		Matrix operator-();
		Matrix operator-(Matrix &);
		Matrix operator-(double);
		Matrix operator*(Matrix &);
		Vector operator*(Vector &);
		Matrix operator*(double);
		Matrix operator/(double);
};

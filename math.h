#pragma once

double radian(const double);

class Vector
{
	public:
		unsigned short dimensions = 0;
		double *values = nullptr;

		Vector(const Vector &);
		Vector(Vector &&);
		Vector(const unsigned short, const double []);
		~Vector();
		Vector operator+(const Vector &) const;
		Vector operator+(const double) const;
		Vector operator-() const;
		Vector operator-(const Vector &) const;
		Vector operator-(const double) const;
		Vector operator*(const Vector &) const;
		Vector operator*(const double) const;
		Vector operator/(const double) const;
		double dot(const Vector &) const;
		double length() const;
		Vector length(const int) const;
		double angle(const Vector &) const;
};

class Matrix
{
	public:
		unsigned short dimensions[2] = {0, 0};
		double *values = nullptr; // "*" creates a ptr pointing at an array with two dimensions.

		Matrix(const Matrix &);
		Matrix(Matrix &&);
		Matrix(const unsigned short [2]);
		Matrix(const unsigned short [2], const double *); // "*" is actually a ptr pointing at the first element of an array with two dimensions.
		~Matrix();
		Matrix operator+(const Matrix &) const;
		Matrix operator+(const double) const;
		Matrix operator-() const;
		Matrix operator-(const Matrix &) const;
		Matrix operator-(const double) const;
		Matrix operator*(const Matrix &) const;
		Vector operator*(const Vector &) const;
		Matrix operator*(const double) const;
		Matrix operator/(const double) const;
		Matrix transpose() const;
};

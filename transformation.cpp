#include <cmath>
#include "constants.hpp"
#include "math.h"
#include "transformation.h"

static unsigned short MATRIX_DIMENSIONS[2] = {4, 4};

Matrix rotate(choices::axis axis, double theta)
{
	theta = radian(theta);
	double items[16];
	switch (axis)
	{
		case choices::axis_x: {
			double items[16] = {
				1.0,	0.0,	0.0,	0.0,
				0.0,	cos(theta),	-sin(theta),	0.0,
				0.0,	sin(theta),	cos(theta),	0.0,
				0.0,	0.0,	0.0,	1.0
			};
			return Matrix(MATRIX_DIMENSIONS, items);
		}
		case choices::axis_y: {
			double items[16] = {
				cos(theta),	0.0,	sin(theta),	0.0,
				0.0,	1.0,	0.0,	0.0,
				-sin(theta),	0.0,	cos(theta),	0.0,
				0.0,	0.0,	0.0,	1.0
			};
			return Matrix(MATRIX_DIMENSIONS, items);
		}
		case choices::axis_z: {
			double items[16] = {
				cos(theta),	-sin(theta),	0.0,	0.0,
				sin(theta),	cos(theta),	0.0,	0.0,
				0.0,	0.0,	1.0,	0.0,
				0.0,	0.0,	0.0,	1.0
			};
			return Matrix(MATRIX_DIMENSIONS, items);
		}
		default:
			throw errors::ChoiceError();
	};
};

Matrix rotate(double theta_x, double theta_y, double theta_z)
{
	Matrix x_rotate = rotate(choices::axis_x, theta_x);
	Matrix y_rotate = rotate(choices::axis_y, theta_y);
	Matrix z_rotate = rotate(choices::axis_z, theta_z);
	return x_rotate * y_rotate * z_rotate;
};

Matrix scale(double ratio)
{
	double items[16] = {
		ratio,	0.0,	0.0,	0.0,
		0.0,	ratio,	0.0,	0.0,
		0.0,	0.0,	ratio,	0.0,
		0.0,	0.0,	0.0,	1.0
	};
	return Matrix(MATRIX_DIMENSIONS, items);
};

Matrix translate(double x, double y, double z)
{
	double items[16] = {
		1.0,	0.0,	0.0,	x,
		0.0,	1.0,	0.0,	y,
		0.0,	0.0,	1.0,	z,
		0.0,	0.0,	0.0,	1.0
	};
	return Matrix(MATRIX_DIMENSIONS, items);
};
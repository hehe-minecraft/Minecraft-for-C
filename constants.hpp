#pragma once
#include <exception>

namespace choices
{
	enum axis {axis_x, axis_y, axis_z};
};

namespace errors
{
	class BasicError: public std::exception{};
	class MathError: public BasicError{};
	class VectorDimensionError: public MathError{};
	class MatrixDimensionError: public MathError{};
	class ChoiceError: public BasicError{};
};
#pragma once
#include <exception>

namespace constants
{
	const double pi = 3.14159265358979323846;
};

namespace choices
{
	enum axis 
	{
		axis_x,
		axis_y,
		axis_z
	};
	enum state 
	{
		state_running,
		state_free,
		state_paused,
		state_stopped
	};
};

namespace errors
{
	class BasicError: public std::exception{};
	class MathError: public BasicError{};
	class VectorDimensionError: public MathError{};
	class MatrixDimensionError: public MathError{};
	class ChoiceError: public BasicError{};
	class ThreadExistError: public BasicError{};
};
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
	enum json
	{
		json_null,
		json_bool,
		json_int,
		json_float,
		json_string,
		json_list,
		json_map
	};
};

namespace errors
{
	class BasicError: public std::exception{};
	class MathError: public BasicError{};
	class VectorDimensionError: public MathError{};
	class MatrixDimensionError: public MathError{};
	class ChoiceError: public BasicError{};
	class UnexpectedControlError: public BasicError{};
	class ThreadExistError: public BasicError{};
	class IndexError: public BasicError{};
	class FileError: public BasicError{};
	class FileNotFoundError: public FileError{};
	class NullPointerError: public BasicError{};
	class JsonError: public BasicError{};
	class JsonTypeError: public JsonError{};
	class JsonSyntaxError: public JsonError{};
	class JsonEOFError: public JsonError{};
	class JsonBackSlashError: public JsonSyntaxError{};
};
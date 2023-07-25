#include <exception>

namespace errors
{
	class BasicError: public std::exception{};
	class MathError: public BasicError{};
	class VectorDimensionError: public MathError{};
	class MatrixDimensionError: public MathError{};
};
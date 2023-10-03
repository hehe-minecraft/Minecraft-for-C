#pragma once
#include "constants.hpp"
#include "math.h"

Matrix rotate(choices::axis, double);
Matrix rotate(double, double, double);
Matrix scale(double);
Matrix translate(double, double, double);
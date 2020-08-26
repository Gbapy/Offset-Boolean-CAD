#pragma once

#include <cmath>

struct VERTEX
{
    double x;
    double y;
    double z;

    VERTEX();
    VERTEX(double xx, double yy, double zz);
	VERTEX & operator=(const VERTEX &other);
	VERTEX & operator+(const VERTEX &other);
	VERTEX & operator-(const VERTEX &other);
	VERTEX & operator*(const double &other);
	VERTEX & operator/(const double &other);

    VERTEX  crossProduct(const VERTEX &b) const;

    double  magnitude() const;
    double  distanceTo(const VERTEX &p2) const;
    void    normalize();

};

#include "vertex.h"
#include "terminal.h"

#include <cmath>

VERTEX::VERTEX() {
    x = 0; y = 0; z = 0;
}

VERTEX::VERTEX(double xx, double yy, double zz) {
    x = xx; y = yy; z = zz;
}

VERTEX & VERTEX::operator=(const VERTEX &other) {
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;

	return *this;
}

VERTEX & VERTEX::operator+(const VERTEX &other) {
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;

	return *this;
}

VERTEX & VERTEX::operator-(const VERTEX &other) {
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;

	return *this;
}

VERTEX & VERTEX::operator*(const double &other) {
	this->x *= other;
	this->y *= other;
	this->z *= other;

	return *this;
}

VERTEX & VERTEX::operator/(const double &other) {
	this->x /= other;
	this->y /= other;
	this->z /= other;

	return *this;
}

double VERTEX::magnitude() const {
    return sqrt(x * x + y * y + z * z);
}

double VERTEX::distanceTo(const VERTEX &p2) const {
    const double dx = p2.x - x;
    const double dy = p2.y - y;
    const double dz = p2.z - z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

VERTEX VERTEX::crossProduct(const VERTEX &b) const {
    VERTEX r = VERTEX(y * b.z - z * b.y,
        z * b.x - x * b.z,
        x * b.y - y * b.x);

    return r;
}

void VERTEX::normalize() {
    const double m = sqrt(x * x + y * y + z * z);
    if (m == 0) {
        x = 0;
        y = 0;
        z = 0;
    }
    else{
        x /= m;
        y /= m;
        z /= m;
    }
}

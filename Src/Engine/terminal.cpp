#include "terminal.h"
#include "global.h"

#include <cmath>

TERMINAL::TERMINAL() {
    x = 0;
    y = 0;
    isValid = true;
}

TERMINAL::TERMINAL(double ix, double iy) {
    x = ix; y = iy;
    isValid = true;
}

TERMINAL::TERMINAL(int ix, int iy) {
    x = (double)ix; y = (double)iy;
    isValid = true;
}

bool TERMINAL::isEqual(const TERMINAL &p) const {
    if (std::abs(p.x - x) < EP && std::abs(p.y - y) < EP) return true;
    return false;
}

void TERMINAL::ensureRectContains(TERMINAL *mn, TERMINAL *mx) const {
    if (x < mn->x) mn->x = x;
    if (y < mn->y) mn->y = y;
    if (x > mx->x) mx->x = x;
    if (y > mx->y) mx->y = y;
}

double TERMINAL::magnitude() const {
    return sqrt(x * x + y * y);
}

double TERMINAL::distanceTo(const TERMINAL &p2) const {
    const double dx = p2.x - x;
    const double dy = p2.y - y;
    return sqrt(dx * dx + dy * dy);
}

// TODO maybe use atan2 instead of atan with a bunch of extra logic
double TERMINAL::angleTo(const TERMINAL &t2) const {
    const double dx = t2.x - x;
    const double dy = t2.y - y;

    if (dx == 0) {
        return dy > 0 ? 90.0f : 270.0f;
    }
    else{
        if (dy == 0) {
            return dx > 0 ? 0 : 180.0f;
        }
        else{
            if (dx > 0) {
                return dy > 0 ? atan(dy / dx) * 180.0f / M_PI : 360.0f + atan(dy / dx) * 180.0f / M_PI;
            }
            else{
                return 180.0f + atan(dy / dx) * 180.0f / M_PI;
            }
        }
    }
    return atan(dy / dx) * 180.0f / M_PI;
}

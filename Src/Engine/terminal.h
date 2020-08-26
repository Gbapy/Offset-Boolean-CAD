#pragma once

struct TERMINAL
{
    double x;
    double y;
    bool isValid;

    TERMINAL();
    TERMINAL(double ix, double iy);
    TERMINAL(int ix, int iy);
    bool isEqual(const TERMINAL &p) const;
    void ensureRectContains(TERMINAL *mn, TERMINAL *mx) const;
    double magnitude() const;
    double distanceTo(const TERMINAL &p2) const;
    double angleTo(const TERMINAL &t2) const;
};

typedef TERMINAL* PTERMINAL;

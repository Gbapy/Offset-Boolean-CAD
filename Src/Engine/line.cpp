#include "line.h"
#include "global.h"

#include <QTextStream>

LINE::LINE() : PRIMITIVE() {
    nKind = GBAPY_LINE;
    radius = 0;
}

LINE::~LINE() = default;

LINE::LINE(TERMINAL t1, TERMINAL t2) : PRIMITIVE() {
    nKind = GBAPY_LINE;
    terms[0] = t1;
    terms[1] = t2;
    center.x = (t1.x + t2.x) / 2.0f;
    center.y = (t1.y + t2.y) / 2.0f;
    radius = M_INFINITE;
}

bool LINE::isFlipped() {
	VERTEX v1 = VERTEX(terms[1].x - terms[0].x, terms[1].y - terms[0].y, 0);
	VERTEX v2 = VERTEX(offsets[1].x - offsets[0].x, offsets[1].y - offsets[0].y, 0);
	v1.normalize();
	v2.normalize();
	v1.x -= v2.x; v1.y -= v2.y;
	if (v1.magnitude() > 0.5f) return true;
	return false;
}

std::unique_ptr<PRIMITIVE> LINE::clone() const
{
    return std::make_unique<LINE>(this->terms[0], this->terms[1]);
}

std::unique_ptr<PRIMITIVE> LINE::clone(const TERMINAL &p) const
{
	TERMINAL sp = p;
	if (p.isEqual(this->terms[1])) return NULL;
	if (this->isContainedPoint(p) == false) return NULL;
	if (p.isEqual(this->terms[0])) sp = this->terms[0];
    return std::make_unique<LINE>(TERMINAL(sp.x, sp.y), this->terms[1]);
}

std::unique_ptr<PRIMITIVE> LINE::clone(const TERMINAL &p1, const TERMINAL &p2) const
{
	TERMINAL sp = p1;
	TERMINAL ep = p2;
    if (p1.isEqual(p2)) return NULL;
	if (this->isContainedPoint(p1) == false) return NULL;
	if (this->isContainedPoint(p2) == false) return NULL;
	if (sp.isEqual(this->terms[0])) sp = this->terms[0];
	if (sp.isEqual(this->terms[1])) sp = this->terms[1];
	if (ep.isEqual(this->terms[0])) ep = this->terms[0];
	if (ep.isEqual(this->terms[1])) ep = this->terms[1];

	VERTEX v1 = VERTEX(terms[1].x - terms[0].x, terms[1].y - terms[0].y, 0);
	VERTEX v2 = VERTEX(ep.x - sp.x, ep.y - sp.y, 0);
	v1.normalize();
	v2.normalize();
	v1.x -= v2.x; v1.y -= v2.y;
	if (v1.magnitude() > 0.5f) return NULL;
    return std::make_unique<LINE>(sp, ep);
}

VERTEX LINE::getPositiveDirection() const
{
    VERTEX v = VERTEX(terms[1].x - terms[0].x, terms[1].y - terms[0].y, 0.0f);
    v.normalize();
    return v;
}

VERTEX LINE::getNegativeDirection() const
{
    VERTEX v = VERTEX(terms[0].x - terms[1].x, terms[0].y - terms[1].y, 0.0f);
    v.normalize();
    return v;
}

void LINE::swapTerminals()
{
    TERMINAL t = terms[1];
    terms[1] = terms[0];
    terms[0] = t;
}

bool LINE::hasSamePivot(const TERMINAL &p)
{
    if (terms[0].isEqual(p)) {
        return true;
    }
    if (terms[1].isEqual(p)) {
        this->swapTerminals();
        return true;
    }
    return false;
}

bool LINE::hasSamePivot(const TERMINAL &p, PTERMINAL ret)
{
    if (terms[0].isEqual(p)) {
        ret->x = terms[0].x;
        ret->y = terms[0].y;
        return true;
    }
    if (terms[1].isEqual(p)) {
        this->swapTerminals();
        ret->x = terms[0].x;
        ret->y = terms[0].y;
        return true;
    }
    return false;
}

double LINE::getDistance(TERMINAL t, PTERMINAL ret) const
{
    VERTEX v1 = VERTEX(t.x - this->terms[1].x, t.y - this->terms[1].y, 0);
    VERTEX v0 = VERTEX(t.x - this->terms[0].x, t.y - this->terms[0].y, 0);
    VERTEX v2 = VERTEX(this->terms[1].x - this->terms[0].x,
        this->terms[1].y - this->terms[0].y, 0);
    VERTEX v3 = v1.crossProduct(v0);
    double a = v2.magnitude();
    double b = v0.magnitude();
    double c = v1.magnitude();
    double h = std::abs(v3.z) / a;

    if (h > b) h = b;
    if (h > c) h = c;

    double l0 = sqrt(b * b - h * h);
    double l1 = sqrt(c * c - h * h);
    if (l0 <= a && l1 <= a) {
        VERTEX v = this->getPositiveDirection();
        ret->x = this->terms[0].x + v.x * l0;
        ret->y = this->terms[0].y + v.y * l0;
    }
    else{
        if (l0 > l1) {
            VERTEX v = this->getPositiveDirection();
            ret->x = terms[0].x + v.x * l0;
            ret->y = terms[0].y + v.y * l0;
        }
        else{
            VERTEX v = this->getNegativeDirection();
            ret->x = terms[1].x + v.x * l1;
            ret->y = terms[1].y + v.y * l1;
        }
    }
    return h;
}

bool LINE::isConvex() const
{
    return true;
}

void LINE::doOffsetOperation()
{
    terms[0] = offsets[0]; terms[1] = offsets[1];
}

bool LINE::isContainedPoint(TERMINAL p) const
{
    TERMINAL v1 = TERMINAL(p.x - terms[0].x, p.y - terms[0].y);
    TERMINAL v2 = TERMINAL(p.x - terms[1].x, p.y - terms[1].y);
    TERMINAL v3 = TERMINAL(terms[1].x - terms[0].x, terms[1].y - terms[0].y);
    double f1 = v1.magnitude();
    double f2 = v2.magnitude();
    double f3 = v3.magnitude();

    if (std::abs(f3 - f1 - f2) <= EP) return true;
    return false;
}

std::unique_ptr<PRIMITIVE> LINE::tryOffset(double offset) const
{
    VERTEX up = VERTEX(0, 0, 1);
    std::unique_ptr<LINE> ret = std::make_unique<LINE>(terms[0], terms[1]);
    VERTEX v = VERTEX(terms[1].x - terms[0].x, terms[1].y - terms[0].y, 0);
    v = v.crossProduct(up);
    v.normalize();
    ret->terms[0].x += (v.x * offset); ret->terms[0].y += (v.y * offset);
    ret->terms[1].x += (v.x * offset); ret->terms[1].y += (v.y * offset);

    return ret;
}

VERTEX LINE::getTangent(TERMINAL p) const
{
    return this->getPositiveDirection();
}

void LINE::write2Stream(FILE *pFile) const
{
	fwrite(&(this->nKind), 1, sizeof(int), pFile);
	fwrite(&(this->terms[0].x), 1, sizeof(double), pFile);
	fwrite(&(this->terms[0].y), 1, sizeof(double), pFile);
	fwrite(&(this->terms[1].x), 1, sizeof(double), pFile);
	fwrite(&(this->terms[1].y), 1, sizeof(double), pFile);
}

void LINE::readFromStream(FILE *pFile) const
{
	fread((void *)&(this->terms[0].x), 1, sizeof(double), pFile);
	fread((void *)&(this->terms[0].y), 1, sizeof(double), pFile);
	fread((void *)&(this->terms[1].x), 1, sizeof(double), pFile);
	fread((void *)&(this->terms[1].y), 1, sizeof(double), pFile);
}

double LINE::getPositiveDelta(TERMINAL t)
{
    if (this->isContainedPoint(t) == false) return -1;
    return this->terms[0].distanceTo(t);
}

double LINE::getNegativeDelta(TERMINAL t)
{
    if (this->isContainedPoint(t) == false) return -1;
    return this->terms[1].distanceTo(t);
}

bool LINE::isEqual(PRIMITIVE *pr)
{
    if (this->nKind != pr->nKind) return false;
    if (this->terms[0].isEqual(pr->terms[0]) == false ||
        this->terms[1].isEqual(pr->terms[1]) == false) return false;
    return true;
}

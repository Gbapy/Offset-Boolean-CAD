#include "arc.h"
#include "global.h"

#include <QTextStream>
#include <math.h>

ARC::ARC() : PRIMITIVE() {
    this->nKind = GBAPY_ARC;
    this->radius = 0;
    this->startAngle = 0;
    this->endAngle = 0;
    this->clockWise = true;
}

ARC::~ARC() = default;

ARC::ARC(TERMINAL c, TERMINAL t1, TERMINAL t2, bool cw) : PRIMITIVE() {
    this->nKind = GBAPY_ARC;
    VERTEX v1 = VERTEX(t1.x - c.x, t1.y - c.y, 0);
    VERTEX v2 = VERTEX(t2.x - c.x, t2.y - c.y, 0);
    this->center = c;
    this->radius = (v1.magnitude() + v2.magnitude()) / 2.0f;
    this->startAngle = center.angleTo(t1);
    this->endAngle = center.angleTo(t2);
    this->terms[0] = t1;
    this->terms[1] = t2;
    this->clockWise = cw;
}

ARC::ARC(TERMINAL c, double r, double sa, double ea, bool cw) : PRIMITIVE() {
    nKind = GBAPY_ARC;
    center = c;
    radius = r;
    startAngle = sa;
    endAngle = ea;
    terms[0] = TERMINAL(c.x + r * cos(sa * M_PI / 180.0f), c.y + r * sin(sa * M_PI / 180.0f));
    terms[1] = TERMINAL(c.x + r * cos(ea * M_PI / 180.0f), c.y + r * sin(ea * M_PI / 180.0f));
    clockWise = cw;
}

ARC::ARC(TERMINAL c, double r, double sa, double ea, TERMINAL t1, TERMINAL t2, bool cw) : PRIMITIVE() {
    nKind = GBAPY_ARC;
    center = c;
    radius = r;
    startAngle = sa;
    endAngle = ea;
    terms[0] = t1;
    terms[1] = t2;
    clockWise = cw;
}

bool ARC::isFlipped() {
	VERTEX v1 = VERTEX(terms[1].x - terms[0].x, terms[1].y - terms[0].y, 0);
	VERTEX v2 = VERTEX(offsets[1].x - offsets[0].x, offsets[1].y - offsets[0].y, 0);
	v1.normalize();
	v2.normalize();
	v1.x -= v2.x; v1.y -= v2.y;
	if (v1.magnitude() > 0.5f) return true;
	return false;
}

void ARC::makeAbsoluteAngles(double *sa, double *ea) const {
    if (this->clockWise && *sa < *ea) *sa += 360.0f;
    if (this->clockWise == false && *sa > *ea) *ea += 360;
}

std::unique_ptr<PRIMITIVE> ARC::clone() const
{
    std::unique_ptr<ARC> prim = std::make_unique<ARC>();
    prim->center = this->center;
    prim->radius = this->radius;
    prim->startAngle = this->startAngle;
    prim->endAngle = this->endAngle;
    prim->clockWise = this->clockWise;
    prim->terms[0] = this->terms[0];
    prim->terms[1] = this->terms[1];

    return prim;
}

std::unique_ptr<PRIMITIVE> ARC::clone(const TERMINAL &p) const
{
	if (this->isContainedPoint(p) == false) return NULL;
    double sa = this->center.angleTo(p);
    double ea = this->endAngle;
	if (std::abs(sa - ea) <= EP_A) return NULL;
	if (std::abs(this->startAngle - sa) <= EP_A) sa = this->startAngle;
    std::unique_ptr<ARC> prim = std::make_unique<ARC>();
    prim->center = this->center;
    prim->radius = this->radius;
    prim->clockWise = this->clockWise;
    prim->startAngle = sa;
    prim->endAngle = ea;
    prim->terms[0].x = p.x; prim->terms[0].y = p.y;
    prim->terms[1] = this->terms[1];

    return prim;
}

std::unique_ptr<PRIMITIVE> ARC::clone(const TERMINAL &p1, const TERMINAL &p2) const
{
	if (this->isContainedPoint(p1) == false) return NULL;
	if (this->isContainedPoint(p2) == false) return NULL;
    double sa = this->center.angleTo(p1);
    double ea = this->center.angleTo(p2);
	double ssa = this->startAngle;
	double eea = this->endAngle;
	if (std::abs(ssa - sa) <= EP_A) sa = ssa;
	if (std::abs(ssa - ea) <= EP_A) ea = ssa;
	if (std::abs(eea - sa) <= EP_A) sa = eea;
	if (std::abs(eea - ea) <= EP_A) ea = eea;

    this->makeAbsoluteAngles(&sa, &ea);
	this->makeAbsoluteAngles(&ssa, &eea);

    if (std::abs(ea - sa) <= EP_A) return NULL;
	if (ssa > eea && sa < ea) return NULL;
	if (ssa < eea && sa > ea) return NULL;

    std::unique_ptr<ARC> prim = std::make_unique<ARC>();
    prim->center = this->center;
    prim->radius = this->radius;

    prim->clockWise = this->clockWise;
    prim->startAngle = this->center.angleTo(p1);
    prim->endAngle = this->center.angleTo(p2);
    prim->terms[0].x = p1.x; prim->terms[0].y = p1.y;
    prim->terms[1].x = p2.x; prim->terms[1].y = p2.y;

    return prim;
}

bool ARC::isInsideAngle(double a) const {
    double sa = startAngle;
    double ea = endAngle;

    this->makeAbsoluteAngles(&sa, &ea);
    double range = std::abs(sa - ea);
    double d1 = std::abs(a - sa);
    double d2 = std::abs(a - ea);

    if (std::abs(range - (d1 + d2)) <= EP_A) return true;
    a += 360.0f;
    d1 = std::abs(a - sa);
    d2 = std::abs(a - ea);
    if (std::abs(range - (d1 + d2)) <= EP_A) return true;

    return false;
}

double ARC::getDistance(TERMINAL t, PTERMINAL ret) const
{
    VERTEX v = VERTEX(t.x - this->center.x, t.y - this->center.y, 0);
    double a = abs(v.magnitude() - this->radius);
    double angle = this->center.angleTo(t);
    ret->x = this->center.x + this->radius * std::cos(angle * M_PI / 180.0f);
    ret->y = this->center.y + this->radius * std::sin(angle * M_PI / 180.0f);

    return a;
}

bool ARC::isConvex() const
{
    double sa = startAngle;
    double ea = endAngle;

    this->makeAbsoluteAngles(&sa, &ea);

    double angle = sa + (ea - sa) / 10.0f;
    VERTEX v1 = VERTEX(center.x + radius * cos(angle * M_PI / 180.0f),
        center.y + radius * sin(angle * M_PI / 180.0f), 0.0f);
    VERTEX v2 = VERTEX(terms[0].x, terms[0].y, 0);
    v1.x -= v2.x; v1.y -= v2.y;
    v2.x = center.x - v2.x; v2.y = center.y - v2.y;
    v1 = v2.crossProduct(v1);
    if (v1.z > 0) {
        return false;
    }
    else{
        return true;
    }
}

VERTEX ARC::getPositiveDirection() const
{
    double sa = startAngle;
    double ea = endAngle;
    this->makeAbsoluteAngles(&sa, &ea);
    double angle = ea < sa ? sa - 1 : sa + 1;
    VERTEX v1 = VERTEX(center.x + radius * cos(angle * M_PI / 180.0f) - terms[0].x,
        center.y + radius * sin(angle * M_PI / 180.0f) - terms[0].y, 0.0f);
    v1.normalize();

    return v1;
}

VERTEX ARC::getNegativeDirection() const
{
    double sa = startAngle;
    double ea = endAngle;
    this->makeAbsoluteAngles(&sa, &ea);
    double angle = sa > ea ? ea + 1 : ea - 1;
    VERTEX v1 = VERTEX(center.x + radius * cos(angle * M_PI / 180.0f) - terms[1].x,
                       center.y + radius * sin(angle * M_PI / 180.0f) - terms[1].y, 0.0f);
    v1.normalize();

    return v1;
}

void ARC::swapTerminals()
{
    TERMINAL t = terms[1];
    terms[1] = terms[0];
    terms[0] = t;
    double a = startAngle; startAngle = endAngle; endAngle = a;
    this->clockWise = !this->clockWise;
}


bool ARC::hasSamePivot(const TERMINAL &p)
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

bool ARC::hasSamePivot(const TERMINAL &p, PTERMINAL ret)
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

void ARC::doOffsetOperation()
{
    this->terms[0] = offsets[0];
    this->terms[1] = offsets[1];
    this->radius = offRadius;
    double sa = center.angleTo(terms[0]);
    double ea = center.angleTo(terms[1]);
    this->startAngle = sa;
    this->endAngle = ea;
}

bool ARC::isContainedPoint(TERMINAL p) const
{
    double alpha = center.angleTo(p);
    double r = this->center.distanceTo(p);
    if (std::abs(this->radius - r) > EP) return false;
    if (this->isInsideAngle(alpha)) return true;

    return false;
}

std::unique_ptr<PRIMITIVE> ARC::tryOffset(double offset) const
{
    if (!this->isConvex()) offset = -offset;

    std::unique_ptr<ARC> ret = std::make_unique<ARC>();
    ret->center = this->center;
    ret->radius = this->radius + offset;
    ret->startAngle = this->startAngle;
    ret->endAngle = this->endAngle;
    ret->clockWise = this->clockWise;
    ret->terms[0] = TERMINAL(ret->center.x + ret->radius * cos(ret->startAngle
        * M_PI / 180.0f), ret->center.y + ret->radius * sin(ret->startAngle * M_PI / 180.0f));
    ret->terms[1] = TERMINAL(ret->center.x + ret->radius * cos(ret->endAngle
        * M_PI / 180.0f), ret->center.y + ret->radius * sin(ret->endAngle * M_PI / 180.0f));

    return ret;
}

VERTEX ARC::getTangent(TERMINAL p) const
{
    double sa = startAngle;
    double ea = endAngle;
    this->makeAbsoluteAngles(&sa, &ea);
    double sign = ea < sa ? -1 : +1;
    double angle = this->center.angleTo(p);

    VERTEX v1 = VERTEX(this->center.x + this->radius * cos((angle + sign) * M_PI / 180.0f) -
        (this->center.x + this->radius * cos(angle * M_PI / 180.0f)),
        this->center.y + this->radius * sin((angle + sign) * M_PI / 180.0f) -
        (this->center.y + this->radius * sin(angle * M_PI / 180.0f)), 0.0f);

    v1.normalize();
    return v1;
}

void ARC::write2Stream(FILE *pFile) const
{
	fwrite(&(this->nKind), 1, sizeof(int), pFile);
	fwrite(&(this->terms[0].x), 1, sizeof(double), pFile);
	fwrite(&(this->terms[0].y), 1, sizeof(double), pFile);
	fwrite(&(this->terms[1].x), 1, sizeof(double), pFile);
	fwrite(&(this->terms[1].y), 1, sizeof(double), pFile);
	fwrite(&(this->center.x), 1, sizeof(double), pFile);
	fwrite(&(this->center.y), 1, sizeof(double), pFile);
	fwrite(&(this->radius), 1, sizeof(double), pFile);
	fwrite(&(this->startAngle), 1, sizeof(double), pFile);
	fwrite(&(this->endAngle), 1, sizeof(double), pFile);
	fwrite(&(this->clockWise), 1, sizeof(bool), pFile);
}

void ARC::readFromStream(FILE *pFile) const
{
	fread((void *)&(this->terms[0].x), 1, sizeof(double), pFile);
	fread((void *)&(this->terms[0].y), 1, sizeof(double), pFile);
	fread((void *)&(this->terms[1].x), 1, sizeof(double), pFile);
	fread((void *)&(this->terms[1].y), 1, sizeof(double), pFile);
	fread((void *)&(this->center.x), 1, sizeof(double), pFile);
	fread((void *)&(this->center.y), 1, sizeof(double), pFile);
	fread((void *)&(this->radius), 1, sizeof(double), pFile);
	fread((void *)&(this->startAngle), 1, sizeof(double), pFile);
	fread((void *)&(this->endAngle), 1, sizeof(double), pFile);
	fread((void *)&(this->clockWise), 1, sizeof(bool), pFile);
}

double ARC::getPositiveDelta(TERMINAL t)
{
    if (this->isContainedPoint(t) == false) return -1;
    double a = this->center.angleTo(t);
    double sa = startAngle;
    double ea = endAngle;
    this->makeAbsoluteAngles(&sa, &ea);
    double range = std::abs(sa - ea);
    double d1 = std::abs(a - sa);
    double d2 = std::abs(a - ea);

    if (std::abs(range - (d1 + d2)) < EP_A) {
        a = std::abs(a - sa);
        return 2 * M_PI * this->radius * a / 360.0f;
    }
    a += 360.0f;
    d1 = std::abs(a - sa);
    d2 = std::abs(a - ea);
    if (std::abs(range - (d1 + d2)) < EP_A) {
        a = std::abs(a - sa);
        return 2 * M_PI * this->radius * a / 360.0f;
    }
    return -1;
}

double ARC::getNegativeDelta(TERMINAL t)
{
    if (this->isContainedPoint(t) == false) return -1;
    double a = this->center.angleTo(t);
    double sa = startAngle;
    double ea = endAngle;
    this->makeAbsoluteAngles(&sa, &ea);
    double range = std::abs(sa - ea);
    double d1 = std::abs(a - sa);
    double d2 = std::abs(a - ea);

    if (std::abs(range - (d1 + d2)) < EP_A) {
        a = std::abs(a - ea);
        return 2 * M_PI * this->radius * a / 360.0f;
    }
    a += 360.0f;
    d1 = std::abs(a - sa);
    d2 = std::abs(a - ea);
    if (std::abs(range - (d1 + d2)) < EP_A) {
        a = std::abs(a - ea);
        return 2 * M_PI * this->radius * a / 360.0f;
    }
    return -1;
}

bool ARC::isEqual(PRIMITIVE *pr)
{
    if (this->nKind != pr->nKind) return false;
    ARC *prim = (ARC *)pr;
    if (this->terms[0].isEqual(prim->terms[0]) == false ||
        this->terms[1].isEqual(prim->terms[1]) == false) return false;
    if (this->center.isEqual(prim->center) == false) return false;
    if (std::abs(this->radius - prim->radius)) return false;
    if (std::abs(this->startAngle - prim->startAngle) > EP_A
        || std::abs(this->endAngle - prim->endAngle) > EP_A) return false;
    return true;
}

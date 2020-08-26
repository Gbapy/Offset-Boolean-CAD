#include "primitive.h"
#include "arc.h"
#include "line.h"
#include "global.h"

PRIMITIVE::PRIMITIVE() {
    radius = 0;
    offRadius = 0;
    clockWise = false;
    isValid = true;
}

PRIMITIVE::~PRIMITIVE() = default;

static int GetSharePoint(const ARC *arc, const LINE *line, TERMINAL *p1, TERMINAL *p2) {
    TERMINAL p0;
    double dist = line->getDistance(arc->center, &p0);
    if (dist - arc->radius > EP) return 0;
    if (dist > arc->radius) dist = arc->radius;
    double a = sqrt(arc->radius * arc->radius - dist * dist);
    VERTEX v = line->getPositiveDirection();
    p1->x = p0.x + v.x * a; p1->y = p0.y + v.y * a;
    p2->x = p0.x - v.x * a; p2->y = p0.y - v.y * a;
    return 1;
}

static int GetSharePoint(const LINE *l1, const LINE *l2, PTERMINAL p1, PTERMINAL p2) {
    VERTEX equ1, equ2;
    VERTEX up = VERTEX(0, 0, 1);

    equ1.x = l1->terms[1].x - l1->terms[0].x;
    equ1.y = l1->terms[1].y - l1->terms[0].y; equ1.z = 0;
    equ2.x = l2->terms[1].x - l2->terms[0].x;
    equ2.y = l2->terms[1].y - l2->terms[0].y; equ2.z = 0;
    equ1 = equ1.crossProduct(up);
    equ2 = equ2.crossProduct(up);
    equ1.normalize();
    equ2.normalize();

    if (equ1.x == 0 && equ1.y == 0) return 0;
    if (equ2.x == 0 && equ2.y == 0) return 0;
    equ1.z = -equ1.x * l1->terms[0].x - equ1.y * l1->terms[0].y;
    equ2.z = -equ2.x * l2->terms[0].x - equ2.y * l2->terms[0].y;

    VERTEX c = equ1.crossProduct(equ2);

    if (c.z == 0) {
        if (l1->terms[0].x * equ2.x + l1->terms[0].y * equ2.y + equ2.z == 0) {
            return 2;
        }
        else{
            return 0;
        }
    }

    p1->y = c.y / c.z;
	if (std::abs(equ1.x) > std::abs(equ2.x)) {
		p1->x = (-equ1.z - equ1.y * p1->y) / equ1.x;
		p2->x = p1->x; p2->y = p1->y;
		return 1;
	}
	else {
		p1->x = (-equ2.z - equ2.y * p1->y) / equ2.x;
		p2->x = p1->x; p2->y = p1->y;
		return 1;
	}
    return 0;
}

static int GetSharePoint(const ARC *arc1, const ARC *arc2, TERMINAL *p1, TERMINAL *p2) {
    double a = arc1->center.distanceTo(arc2->center);
    if (a - arc1->radius - arc2->radius > EP) return 0;
    if (arc1->radius - a - arc2->radius > EP) return 0;
    if (arc2->radius - a - arc1->radius > EP) return 0;
    if (a > arc1->radius + arc2->radius) a = arc1->radius + arc2->radius;
    if (a == 0 && arc1->radius != arc2->radius) return 0;
    if (a == 0 && arc1->radius == arc2->radius) {
        return 2;
    }
    double b = arc1->radius;
    double c = arc2->radius;
    double d = (a + b + c) / 2.0f;
    if (d < a) d = a;
    if (d < b) d = b;
    if (d < c) d = c;
    d = sqrt(d * (d - a) * (d - b) * (d - c)) * 2.0f;
    d = d / a;

    double angle = arc1->center.angleTo(arc2->center);
    double da = std::abs(asin(d / b)) * 180.0f / M_PI;
    if (c * c > (a * a + b * b)) da = 180 - da;
    p1->x = arc1->center.x + arc1->radius * cos((angle + da) * M_PI / 180.0f);
    p1->y = arc1->center.y + arc1->radius * sin((angle + da) * M_PI / 180.0f);
    p2->x = arc1->center.x + arc1->radius * cos((angle - da) * M_PI / 180.0f);
    p2->y = arc1->center.y + arc1->radius * sin((angle - da) * M_PI / 180.0f);

    return 1;
}

// TODO move this function to a different file
int GetSharePoint(PRIMITIVE *obj1, PRIMITIVE *obj2, TERMINAL *p1, TERMINAL *p2) {
    if (obj1->nKind == GBAPY_ARC && obj2->nKind == GBAPY_ARC) {
        return GetSharePoint((ARC *)obj1, (ARC *)obj2, p1, p2);
    }
    else if (obj1->nKind == GBAPY_ARC && obj2->nKind == GBAPY_LINE) {
        return GetSharePoint((ARC *)obj1, (LINE *)obj2, p1, p2);
    }
    else if (obj1->nKind == GBAPY_LINE && obj2->nKind == GBAPY_ARC) {
        return GetSharePoint((ARC *)obj2, (LINE *)obj1, p1, p2);
    }
    else if (obj1->nKind == GBAPY_LINE && obj2->nKind == GBAPY_LINE) {
        return GetSharePoint((LINE *)obj1, (LINE *)obj2, p1, p2);
    }

    return 0;
}

// TODO move this function to a different file
int isConflict(PRIMITIVE *obj1, PRIMITIVE *obj2, TERMINAL *p1, TERMINAL *p2) {
    int ret = GetSharePoint(obj1, obj2, p1, p2);
    if(ret == 0 || ret == 2) return ret;

    p1->isValid = obj1->isContainedPoint(*p1) && obj2->isContainedPoint(*p1) ? true : false;
    p2->isValid = obj1->isContainedPoint(*p2) && obj2->isContainedPoint(*p2) ? true : false;
    if (p1->isValid == false && p2->isValid == false) ret = 0;

    return ret;
}

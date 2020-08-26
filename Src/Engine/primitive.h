#pragma once

#include "terminal.h"
#include "vertex.h"

#include <memory>

#include <QtGlobal>
QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

struct PRIMITIVE
{
    TERMINAL    terms[2];
    TERMINAL    offsets[2];
    TERMINAL    center;
    double      radius;
    bool        clockWise;
    double      offRadius;
    int         nKind = -1;
    bool        isValid;

    PRIMITIVE();
    virtual ~PRIMITIVE();

    virtual std::unique_ptr<PRIMITIVE> clone() const = 0;
    virtual std::unique_ptr<PRIMITIVE> clone(const TERMINAL &p) const = 0;
    virtual std::unique_ptr<PRIMITIVE> clone(const TERMINAL &p1, const TERMINAL &p2) const = 0;
    virtual std::unique_ptr<PRIMITIVE> tryOffset(double offset) const = 0;

    virtual bool hasSamePivot(const TERMINAL &p, PTERMINAL ret) = 0;
    virtual bool hasSamePivot(const TERMINAL &p) = 0;
    virtual bool isConvex() const = 0;
    virtual bool isContainedPoint(TERMINAL p) const = 0;
    virtual bool isEqual(PRIMITIVE *pr) = 0;
	virtual bool isFlipped() = 0;

    virtual VERTEX getPositiveDirection() const = 0;
    virtual VERTEX getNegativeDirection() const = 0;
    virtual VERTEX getTangent(TERMINAL p) const = 0;

    virtual void doOffsetOperation() = 0;
    virtual void swapTerminals() = 0;
    virtual void write2Stream(FILE *pFile) const = 0;
	virtual void readFromStream(FILE *pFile) const = 0;

    virtual double getDistance(TERMINAL t, PTERMINAL ret) const = 0;
    virtual double getPositiveDelta(TERMINAL t) = 0;
    virtual double getNegativeDelta(TERMINAL t) = 0;


};

int GetSharePoint(PRIMITIVE *obj1, PRIMITIVE *obj2, TERMINAL *p1, TERMINAL *p2);
int isConflict(PRIMITIVE *obj1, PRIMITIVE *obj2, TERMINAL *p1, TERMINAL *p2);

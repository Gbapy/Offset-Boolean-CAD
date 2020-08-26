#pragma once

#include "primitive.h"
#include "vertex.h"

struct LINE : public PRIMITIVE
{
    LINE();
    LINE(TERMINAL t1, TERMINAL t2);
    virtual ~LINE();
    virtual std::unique_ptr<PRIMITIVE> clone() const override;
    virtual std::unique_ptr<PRIMITIVE> clone(const TERMINAL &p) const override;
    virtual std::unique_ptr<PRIMITIVE> clone(const TERMINAL &p1, const TERMINAL &p2) const override;
    virtual std::unique_ptr<PRIMITIVE> tryOffset(double offset) const override;

    virtual VERTEX getPositiveDirection() const override;
    virtual VERTEX getNegativeDirection() const override;
    virtual VERTEX getTangent(TERMINAL p) const override;

    virtual bool hasSamePivot(const TERMINAL &p) override; // TODO rename this, as it actually mutates the object
    virtual bool hasSamePivot(const TERMINAL &p, PTERMINAL ret) override; // TODO rename this, as it actually mutates the object
    virtual bool isContainedPoint(TERMINAL p) const override;
    virtual bool isConvex() const override;
    virtual bool isEqual(PRIMITIVE *pr) override;
	virtual bool isFlipped() override;

    virtual double getDistance(TERMINAL t, PTERMINAL ret) const override;
    virtual double getPositiveDelta(TERMINAL t) override;
    virtual double getNegativeDelta(TERMINAL t) override;

    virtual void doOffsetOperation() override;
    virtual void swapTerminals() override;
    virtual void write2Stream(FILE *pFile) const override;
	virtual void readFromStream(FILE *pFile) const override;
};

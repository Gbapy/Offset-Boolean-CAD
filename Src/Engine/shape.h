#pragma once

#include "primitive.h"

#include <vector>

#include <QtGlobal>
QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

struct SHAPE
{
    bool isValid = true;
    bool isIntersected = true;
    bool isCompleted = false;
    bool isPositive = false;

    std::vector<std::unique_ptr<PRIMITIVE>> prims;

    SHAPE();
    SHAPE(const SHAPE &other);
    SHAPE(SHAPE &&other);
    SHAPE & operator=(const SHAPE &other);
    SHAPE & operator=(SHAPE &&other);
    SHAPE *clone();

    int findFrozenPrimitive();
    int findFrozenTerm();

    bool isSortedShape();
    bool sortPremitives();
    bool isInsidePoint(PRIMITIVE *pr, int index);
    bool isInsidePoint(TERMINAL p);
    bool isInsideShape(SHAPE *shp);
    bool makePositive();
    bool getSelfIntersection(int index, PRIMITIVE *pr, PTERMINAL ret, int *retIndex);
    bool isPositiveShape();
    bool doOffsetOperation(double offsetVal, std::vector<SHAPE> *subShapes);

    void turnPrimitiveOut();
    void insertPrimitive(std::unique_ptr<PRIMITIVE> pr, int index);
    void removePrimitives();
    void write2Stream(FILE *pFile);
    void readFromStream(FILE * pFile);
    void clear();
    void update();
};

void removeDuplicated(std::vector<SHAPE> *subShapes);
void ClearShapes(std::vector<SHAPE> *subShapes);

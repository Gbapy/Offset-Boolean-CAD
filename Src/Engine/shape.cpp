#include "shape.h"
#include "arc.h"
#include "global.h"
#include "line.h"
#include "primitive.h"

#include <QTextStream>

SHAPE::SHAPE() {
    this->prims.clear();
    this->isValid = true;
    this->isIntersected = true;
    this->isCompleted = false;
    this->isPositive = false;
}

SHAPE::SHAPE(const SHAPE &other) {
    operator=(other);
}

SHAPE::SHAPE(SHAPE &&other) = default;

SHAPE & SHAPE::operator=(const SHAPE &other) {
    this->clear();
    this->prims.resize(other.prims.size());
    for (std::size_t i = 0; i < other.prims.size(); i++) {
        this->prims[i] = other.prims[i]->clone();
    }
    this->isValid = other.isValid;
    this->isIntersected = other.isIntersected;
    this->isCompleted = other.isCompleted;
    this->isPositive = other.isPositive;
    return *this;
}

SHAPE & SHAPE::operator=(SHAPE &&other) = default;

SHAPE* SHAPE::clone() {
    SHAPE *shp = new SHAPE();
    for (std::size_t i = 0; i < this->prims.size(); i++) {
        shp->prims.push_back(this->prims[i]->clone());
    }
    shp->isValid = this->isValid;
    shp->isIntersected = this->isIntersected;
    shp->isCompleted = this->isCompleted;
    shp->isPositive = this->isPositive;
    return shp;
}

int SHAPE::findFrozenTerm()
{
    for (std::size_t i = 0; i < this->prims.size(); i++) {
        TERMINAL t1 = this->prims[i]->terms[0];
        TERMINAL t2 = this->prims[i]->terms[1];
        int n1 = 0;
        int n2 = 0;
        for (std::size_t j = 0; j < this->prims.size(); j++) {
            if (i == j) continue;
            if (this->prims[j]->hasSamePivot(t1)) 
				n1++;
            if (this->prims[j]->hasSamePivot(t2)) 
				n2++;
        }
        if (n1 == 0 || n2 == 0) 
			return i;
        if (n1 > 1 || n2 > 1) 
			return i;
    }
    return -1;
}

int SHAPE::findFrozenPrimitive() {
    for (std::size_t i = 0; i < this->prims.size(); i++) {
        std::unique_ptr<PRIMITIVE> pr = this->prims[i]->clone();
        TERMINAL t;
        int index;

        if (this->getSelfIntersection(i, pr.get(), &t, &index) == true) {
            pr.reset(nullptr);
            return i;
        }
        pr.reset(nullptr);
    }
    return -1;
}

bool SHAPE::getSelfIntersection(int index, PRIMITIVE *pr, PTERMINAL ret, int *retIndex)
{
    bool flag = false;
    double mn = 0;
    int n0 = index;
    int n1 = n0 >= 0 && n0 < (int)(this->prims.size()) ? index + 1 : -1;
    int n2 = n0 >= 0 && n0 < (int)(this->prims.size()) ? index - 1 : -1;
    if (n0 >= 0 && n0 < (int)(this->prims.size())) {
        if (n1 == static_cast<int>(this->prims.size())) n1 = 0;
        if (n2 < 0) n2 = static_cast<int>(this->prims.size()) - 1;
    }
    for (std::size_t i = 0; i < this->prims.size(); i++) {
        if (n0 == static_cast<int>(i)) continue;
        TERMINAL p1, p2;
        if (isConflict(pr, this->prims[i].get(), &p1, &p2) == 1) {
            if (p1.isValid && !(pr->terms[0].isEqual(p1)) && !(pr->terms[1].isEqual(p1)))
            {
                double m = pr->getPositiveDelta(p1);
                if (flag == false) {
                    flag = true;
                    mn = m;
                    ret->x = p1.x;
                    ret->y = p1.y;
                    *retIndex = i;
                }
                else{
                    if (m < mn) {
                        ret->x = p1.x;
                        ret->y = p1.y;
                        *retIndex = i;
                        mn = m;
                    }
                }
            }
            if (p2.isValid && !(pr->terms[0].isEqual(p2)) && !(pr->terms[1].isEqual(p2)))
            {
                double m = pr->getPositiveDelta(p2);
                if (flag == false) {
                    flag = true;
                    mn = m;
                    ret->x = p2.x;
                    ret->y = p2.y;
                    *retIndex = i;
                }
                else{
                    if (m < mn) {
                        ret->x = p2.x;
                        ret->y = p2.y;
                        *retIndex = i;
                        mn = m;
                    }
                }
            }
        }
    }
    return flag;
}

bool SHAPE::isSortedShape() {
    for (std::size_t i = 0; i < this->prims.size(); i++) {
        int n = (int)i + 1;
        if (n == (int)(this->prims.size())) n = 0;
        if (this->prims[i]->terms[1].isEqual(this->prims[n]->terms[0]) == false) {
            return false;
        }
    }
    return true;
}

static std::vector<PRIMITIVE*> UniquePtrVectorToPtrVector(std::vector<std::unique_ptr<PRIMITIVE> > &src)
{
    std::vector<PRIMITIVE*> result;
    result.reserve(src.size());
    for (std::size_t i = 0; i < src.size(); i++)
    {
        result.push_back(src[i].release());
    }
    src.clear();
    return result;
}

// TODO make this function use std::unique_ptr<PRIMITIVE> correctly
bool SHAPE::sortPremitives()
{
    if (this->findFrozenTerm() != -1) return false;
    if (this->prims.size() < 2) return false;
    int n = 0;
    TERMINAL t = this->prims[0]->terms[1];
    std::vector<PRIMITIVE*> myPrims = UniquePtrVectorToPtrVector(this->prims);
    std::vector<PRIMITIVE*> ps;

    ps.push_back(myPrims[0]);

    while (true) {
        bool flag = false;
        for (std::size_t i = 0; i < myPrims.size(); i++) {
            if (n == static_cast<int>(i)) continue;
            if (myPrims[i]->hasSamePivot(t)) {

                ps.push_back(myPrims[i]);
                t = myPrims[i]->terms[1];
                n = i;
                if (ps.size() == myPrims.size()) {
                    flag = false;
                }
                else{
                    flag = true;
                }
                break;
            }
        }
        if (flag == false) break;
    }
    for (std::size_t i = 0; i < ps.size(); i++) {
        this->prims.push_back(std::unique_ptr<PRIMITIVE>(ps[i]));
    }

    return true;
}


bool SHAPE::isPositiveShape() {
    VERTEX up = VERTEX(0, 0, 1);

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        int n = (int)i - 1;
        if (n < 0) n = this->prims.size() - 1;
        TERMINAL t = this->prims[i]->terms[0];
        VERTEX v1 = this->prims[i]->getPositiveDirection();
        VERTEX v2T = this->prims[n]->getNegativeDirection();
        VERTEX v2 = VERTEX(-v2T.x, -v2T.y, 0);

        v2T.x += v1.x; v2T.y += v1.y;
        v2T.normalize();
        v1 = v2.crossProduct(v1);
        if (v1.z == 0) {
            v2T = up.crossProduct(v2);
        }
        else if (v1.z < 0) {
            v2T.x = -v2T.x; v2T.y = -v2T.y;
        }
        v2T.normalize();

        LINE l1(t, TERMINAL(t.x + v2T.x * M_INFINITE, t.y + v2T.y * M_INFINITE));
        TERMINAL p;
        double mn = M_INFINITE;
        int mnI = -1;

        for (std::size_t j = 0; j < this->prims.size(); j++) {
            TERMINAL p1, p2;
            if (isConflict(&l1, this->prims[j].get(), &p1, &p2) == 1) {
                if (p1.isValid && !p1.isEqual(t)) {
                    double m = t.distanceTo(p1);
                    if (m < mn) {
                        p = p1;
                        mn = m;
                        mnI = j;
                    }
                }
                if (p2.isValid && !p2.isEqual(t)) {
                    double m = t.distanceTo(p2);
                    if (m < mn) {
                        p = p2;
                        mn = m;
                        mnI = j;
                    }
                }
            }
        }

        if (mnI == -1) 
			return false;
        v1 = this->prims[mnI]->getTangent(p);
        v2.x = t.x - p.x; v2.y = t.y - p.y; v2.z = 0;
        v2.normalize();
        v1 = v1.crossProduct(v2);
        if (v1.z < 0) 
			return false;
    }

    return true;
}

void SHAPE::turnPrimitiveOut() {
    std::vector<std::unique_ptr<PRIMITIVE>> ps;
    for (int i = (int)this->prims.size() - 1; i >= 0; i--) {
        this->prims[i]->swapTerminals();
        ps.push_back(std::move(this->prims[i]));
    }
    this->prims.clear();
    for (std::size_t i = 0; i < ps.size(); i++) {
        this->prims.push_back(std::move(ps[i]));
    }
}

bool SHAPE::makePositive() {
    if (this->isPositiveShape() == false) {
        this->turnPrimitiveOut();
    }
    this->isPositive = true;
    return true;
}

void SHAPE::update() {
    if (this->isSortedShape() == false) {
        if (!this->sortPremitives()) return;
        makePositive();
    }
    else{
        this->isPositive = this->isPositiveShape();
    }
    if (this->findFrozenPrimitive() != -1) return;
    this->isCompleted = true;
}

bool SHAPE::isInsidePoint(PRIMITIVE *pr, int index)
{
    VERTEX v1 = this->prims[index]->getTangent(pr->terms[1]);
    VERTEX v2 = pr->getNegativeDirection();
    v1 = v1.crossProduct(v2);
    return v1.z < 0 ? false : true;
}

bool SHAPE::isInsidePoint(TERMINAL p) {
    TERMINAL t;
    int index;
    LINE *ln = new LINE(TERMINAL(p.x, p.y), TERMINAL(p.x + M_INFINITE, p.y));
    bool ret = this->getSelfIntersection(-1, ln, &t, &index);

    delete ln;
    if (ret == false) return false;
    ln = new LINE(p, t);
    ret = this->isInsidePoint(ln, index);
    delete ln;
    return ret;
}

bool SHAPE::isInsideShape(SHAPE *shp) {
    for (std::size_t i = 0; i < shp->prims.size(); i++) {
        if (this->isInsidePoint(shp->prims[i]->terms[0]) == false) return false;
    }
    return true;
}

void SHAPE::insertPrimitive(std::unique_ptr<PRIMITIVE> pr, int index)
{
    SHAPE sp;

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        if ((int)i == index) {
            sp.prims.push_back(std::move(pr));
        }
        sp.prims.push_back(std::move(this->prims[i]));
    }
    this->prims.clear();
    for (std::size_t i = 0; i < sp.prims.size(); i++) {
        this->prims.push_back(std::move(sp.prims[i]));
    }
}

// TODO make this function use std::unique_ptr<PRIMITIVE> correctly
void SHAPE::removePrimitives() {
    //std::remove_if(prims.begin(), prims.end(), [&](const std::unique_ptr<PRIMITIVE> &prim) -> bool {
    //    return !prim->isValid;
    //});
    SHAPE sp;

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        if (this->prims[i]->isValid) {
            sp.prims.push_back(std::move(this->prims[i]));
        }
        else{
            this->prims[i].reset(nullptr);
        }

    }
    this->prims.clear();
    for (std::size_t i = 0; i < sp.prims.size(); i++) {
        this->prims.push_back(std::move(sp.prims[i]));
    }
}


void SHAPE::write2Stream(FILE *pFile)
{
	int n = (int)(this->prims.size());
	fwrite(&n, 1, sizeof(int), pFile);
	for (std::size_t i = 0; i < this->prims.size(); i++) {
		this->prims[i]->write2Stream(pFile);
	}
}

void SHAPE::readFromStream(FILE * pFile) {
	int n;
	fread(&n, 1, sizeof(int), pFile);
	for (int i = 0; i < n; i++) {
		int m;
		fread(&m, 1, sizeof(int), pFile);
		std::unique_ptr<PRIMITIVE> new_primitive;
		if (m == GBAPY_LINE) {
			new_primitive = std::make_unique<LINE>();
			new_primitive->readFromStream(pFile);
		}
		else if (m == GBAPY_ARC) {
			new_primitive = std::make_unique<ARC>();
			new_primitive->readFromStream(pFile);
		}
		this->prims.push_back(std::move(new_primitive));
	}
	this->update();
}

void SHAPE::clear() {
    for (std::size_t i = 0; i < this->prims.size(); i++) {
        this->prims[i].reset(nullptr);
    }
    prims.clear();
}

bool SHAPE::doOffsetOperation(double offsetVal, std::vector<SHAPE> *subShapes) {
    if (this->isCompleted == false) return false;

    bool bPositive = this->isPositive;
    bool bCW = offsetVal > 0 ? false : true;

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        this->prims[i]->isValid = true;
        std::unique_ptr<PRIMITIVE> pr1 = this->prims[i]->tryOffset(offsetVal);
        if (pr1->radius < EP)
            this->prims[i]->isValid = false;
        pr1.reset(nullptr);
    }

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        int n = i + 1 >= this->prims.size() ? 0 : (int)i + 1;
        if (this->prims[i]->nKind != GBAPY_LINE || this->prims[n]->nKind != GBAPY_LINE) continue;
        VERTEX v1 = this->prims[i]->getPositiveDirection();
        VERTEX v2 = this->prims[n]->getPositiveDirection();

        v1 = v1.crossProduct(v2);
        if (std::abs(v1.z) < 0.1f) {
            this->prims[i]->terms[1] = this->prims[n]->terms[1];
            this->prims[n]->isValid = false;
        }
    }

    this->removePrimitives();

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        int n = (int)i - 1;
        if (n < 0) n = this->prims.size() - 1;
        std::unique_ptr<PRIMITIVE> pr1 = this->prims[i]->tryOffset(offsetVal);
        std::unique_ptr<PRIMITIVE> pr2 = this->prims[n]->tryOffset(offsetVal);

        TERMINAL p1, p2;
        int ret = GetSharePoint(pr1.get(), pr2.get(), &p1, &p2);
        if (ret == 1) {
            double d1 = p1.distanceTo(this->prims[i]->terms[0]);
            double d2 = p2.distanceTo(this->prims[i]->terms[0]);

            p1 = d1 < d2 ? p1 : p2;
            p2 = p1;
        }
        else if (ret == 2){
            p1 = pr1->terms[0];
            p2 = p1;
        }
        else{
            p1 = pr1->terms[0];
            p2 = pr2->terms[1];
        }

        this->prims[i]->offsets[0] = p1;
        this->prims[n]->offsets[1] = p2;
        this->prims[i]->offRadius = pr1->radius;
        this->prims[n]->offRadius = pr2->radius;
        pr1.reset(nullptr);
        pr2.reset(nullptr);
    }

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        this->prims[i]->isValid = true;
        if (this->prims[i]->isFlipped())
            this->prims[i]->isValid = false;
    }

    this->removePrimitives();

    if (this->prims.size() < 2) {
        this->isValid = false;
        return true;
    }

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        if (this->prims[i]->isValid == false) continue;
        int n = i - 1;
        if (n == -1) n = this->prims.size() - 1;
        std::unique_ptr<PRIMITIVE> pr1 = this->prims[i]->tryOffset(offsetVal);
        std::unique_ptr<PRIMITIVE> pr2 = this->prims[n]->tryOffset(offsetVal);

        TERMINAL p1, p2;

		if (pr1->terms[0].isEqual(pr2->terms[1])) {
			p1 = pr1->terms[0];
			p2 = pr2->terms[1];
		}
		else {
			int ret = isConflict(pr1.get(), pr2.get(), &p1, &p2);
			if (ret == 1) {
				double d1 = p1.isValid ? p1.distanceTo(this->prims[i]->terms[0]) : M_INFINITE;
				double d2 = p2.isValid ? p2.distanceTo(this->prims[i]->terms[0]) : M_INFINITE;

				p1 = d1 < d2 ? p1 : p2;
				p2 = p1;
			}
			else if (ret == 2) {
				p1 = pr1->terms[0];
				p2 = p1;
			}
			else {
				if (this->prims[i]->terms[0].isEqual(this->prims[n]->terms[1])) {
					this->insertPrimitive(std::make_unique<ARC>(this->prims[i]->terms[0], pr2->terms[1], pr1->terms[0], bCW), i);
				}
				else {
					TERMINAL ct = TERMINAL((this->prims[i]->terms[0].x + this->prims[n]->terms[1].x) / 2.0f,
						(this->prims[i]->terms[0].y + this->prims[n]->terms[1].y) / 2.0f);
					this->insertPrimitive(std::make_unique<ARC>(ct, pr2->terms[1], pr1->terms[0], bCW), i);
				}
				this->prims[i]->isValid = false;
				if ((int)i < n) n++;
				i++;
				p1 = pr1->terms[0];
				p2 = pr2->terms[1];
			}
		}

        this->prims[i]->offsets[0] = p1;
        this->prims[n]->offsets[1] = p2;
        this->prims[i]->offRadius = pr1->radius;
        this->prims[n]->offRadius = pr2->radius;
        pr1.reset(nullptr);
        pr2.reset(nullptr);
    }

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        if (this->prims[i]->isValid == false) {
            this->prims[i]->isValid = true;
        }
        else{
            this->prims[i]->doOffsetOperation();
        }
    }

    for (std::size_t i = 0; i < this->prims.size(); i++) {
        std::unique_ptr<PRIMITIVE> pr = this->prims[i]->clone();
        TERMINAL t;
        int index;

        if (pr == NULL) continue;
        while (this->getSelfIntersection(i, pr.get(), &t, &index)) {
            SHAPE tshp;
            TERMINAL st = pr->terms[0];
            TERMINAL et = t;
            std::unique_ptr<PRIMITIVE> prim = this->prims[i]->clone(st, et);

            pr.reset(nullptr);

			if (prim == NULL) break;
            if (this->isInsidePoint(prim.get(), index) != bCW) {
                prim.reset(nullptr);
                pr = this->prims[i]->clone(et);
                if(pr == NULL) break;
                continue;
            }

            this->isValid = false;
            tshp.prims.push_back(std::move(prim));
            while (true)
            {
                int m = index;
                std::unique_ptr<PRIMITIVE> pr1 = this->prims[index]->clone(t);
                if (pr1 == NULL) break;
                if (this->getSelfIntersection(m, pr1.get(), &t, &index)) {
					std::unique_ptr<PRIMITIVE> pr2 = pr1->clone(pr1->terms[0], t);
					if (pr2 == NULL) break;
                    tshp.prims.push_back(std::move(pr2));
                }
                else{
                    tshp.prims.push_back(pr1->clone());
                    index = m;
                    index++;
                    if ((std::size_t)index >= this->prims.size()) index = 0;
                    t = pr1->terms[1];
                }
                pr1.reset(nullptr);
                if (t.isEqual(st)) break;
            }
            tshp.update();
			if (tshp.isCompleted)
				subShapes->push_back(tshp);
			else {
				tshp.clear();
				tshp.prims.clear();
			}
            pr = this->prims[i]->clone(et);
            if (pr == NULL) break;
        }
        if(pr != NULL) pr.reset(nullptr);
    }

    if (this->isValid) {
        this->update();
        if (this->isPositive != bPositive) this->isValid = false;
    }
    else if (subShapes->size() > 1) {
        removeDuplicated(subShapes);
    }
    return true;
}

void removeDuplicated(std::vector<SHAPE> *subShapes) {
    for (std::size_t i = 0; i < subShapes->size(); i++) {
        for (std::size_t j = i + 1; j < subShapes->size(); j++) {
            if (i == j) continue;
            for (std::size_t k = 0; k < subShapes->at(j).prims.size(); k++) {
                if (subShapes->at(i).prims[0]->terms[0].isEqual(subShapes->at(j).prims[k]->terms[0])
                    && subShapes->at(i).prims[0]->terms[1].isEqual(subShapes->at(j).prims[k]->terms[1]))
                {
                    subShapes->at(j).isValid = false;
                    break;
                }
            }
        }
    }
    ClearShapes(subShapes);
}

void ClearShapes(std::vector<SHAPE> *subShapes) {
    std::vector<SHAPE> shps;

    for (std::size_t i = 0; i < subShapes->size(); i++) {
        if (subShapes->at(i).isValid == false) {
            subShapes->at(i).clear();
        }
        else{
            shps.push_back(subShapes->at(i));
        }
    }
    subShapes->clear();
    for (std::size_t i = 0; i < shps.size(); i++) {
        subShapes->push_back(shps[i]);
    }
    shps.clear();
}

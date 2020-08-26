#include "GeometryPlot.h"
#include "engine/core.h"
#include "engine/global.h"

#include "engine/terminal.h"
#include "engine/shape.h"

#include <QPainter>
#include <QMouseEvent>
#include <QFile>
#include <QTextStream>

#include <vector>
#include <iterator>
#include <algorithm>

namespace BooleanOffset {

// forward declarations
static QPointF TERMINALtoQPointF(const TERMINAL &pt);
static QLineF LINEtoQLineF(const LINE &line);
static QPainterPath SHAPEtoQPainterPath(const SHAPE &shape);

GeometryPlot::GeometryPlot(QWidget *parent) : QWidget(parent), m_curP(0, 0), m_nShapeKind(-1)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
    setFixedSize(QSize(800, 600));
    setMouseTracking(true);
}

GeometryPlot::~GeometryPlot()
{
}

QPointF GeometryPlot::mapToPlot(const QPointF &pt) const
{
    return QPointF(pt.x(), height() - 1 - pt.y());
}

QPointF GeometryPlot::mapFromPlot(const QPointF &pt) const
{
    return QPointF(pt.x(), height() - 1 - pt.y());
}

void GeometryPlot::setTool(int tool)
{
    if (m_nShapeKind != tool)
    {
        m_nShapeKind = tool;
        emit toolChanged(tool);
        m_pivots.clear();
        update();
    }
}

void GeometryPlot::open(const QString &filePath)
{
	FILE *pFile = fopen(filePath.toLatin1(), "rb");
	if (pFile == NULL) return;
	int n = 0;

	fread(&n, 1, sizeof(int), pFile);
	if (n < 0) {
		fclose(pFile);
		return;
	}

	clear();

	for (int i = 0; i < n; i++) {
		SHAPE shp;
		shp.readFromStream(pFile);
		m_shapes.push_back(shp);
	}
	fclose(pFile);
    BackupShape();
    ExtractSnapPivots();
    update();
}

void GeometryPlot::save(const QString &filePath)
{
	FILE *pFile = fopen(filePath.toLatin1(), "wb");
	if (pFile == NULL) return;
	int n = (int)m_shapes.size();

	fwrite(&n, 1, sizeof(int), pFile);
	for (std::size_t i = 0; i < m_shapes.size(); i++) {
		m_shapes[i].write2Stream(pFile);
	}
	fclose(pFile);
}

void GeometryPlot::clear()
{
    setTool(-1);
    m_pivots.clear();;
    m_snapPivots.clear();
    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        m_shapes[i].clear();
    }
    for (std::size_t i = 0; i < m_reloadShapes.size(); i++) {
        m_reloadShapes[i].clear();
    }
    for (std::size_t i = 0; i < m_GhostShapes.size(); i++) {
        m_GhostShapes[i].clear();
    }
    m_shapes.clear();
    m_reloadShapes.clear();
    m_GhostShapes.clear();
    m_snap = false;
    update();
}

void GeometryPlot::BooleanButtonFunction()
{
    // TODO does this function actually do anything?
    setTool(-1);
    ExtractSnapPivots();
}

void GeometryPlot::BackupShape() {
    m_reloadShapes = m_shapes;
}

void GeometryPlot::RestoreShape() {
    m_shapes = m_reloadShapes;
}

void GeometryPlot::offset(double r)
{
    for(int n = 0;n < 10;n++) {
        std::vector<SHAPE> subShapes;
        for (std::size_t i = 0; i < m_shapes.size(); i++) {
            m_shapes[i].doOffsetOperation(r/10.0, &subShapes);
        }
        if(subShapes.size() > 0) {
            removeDuplicated(&subShapes);
        }
        for (std::size_t i = 0; i < subShapes.size(); i++) {
            m_shapes.push_back(subShapes[i]);
        }
        subShapes.clear();
        ClearShapes(&m_shapes);
        doBooleanOPT();
    }
    ExtractSnapPivots();
    update();
}

void GeometryPlot::ghostOffset(double r)
{
    for(std::size_t i = 0;i < m_shapes.size();i++) {
        SHAPE sp;
        for(std::size_t j = 0;j < m_shapes[i].prims.size();j++) {
            sp.prims.push_back(m_shapes[i].prims[j]->clone());
        }
        m_GhostShapes.push_back(sp);
    }

    for(int n = 0;n < 10;n++) {
        std::vector<SHAPE> subShapes;
        for (std::size_t i = 0; i < m_shapes.size(); i++) {
            m_shapes[i].doOffsetOperation(r/10.0, &subShapes);
        }
        if(subShapes.size() > 0) {
            removeDuplicated(&subShapes);
        }
        for (std::size_t i = 0; i < subShapes.size(); i++) {
            m_shapes.push_back(subShapes[i]);
        }
        subShapes.clear();
        ClearShapes(&m_shapes);
        doBooleanOPT();
    }

    ExtractSnapPivots();
    update();
}

void GeometryPlot::reload()
{
	for (std::size_t i = 0; i < m_GhostShapes.size(); i++) {
		m_GhostShapes[i].clear();
	}
	m_GhostShapes.clear();
    RestoreShape();
    ExtractSnapPivots();
    update();
}

void GeometryPlot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // setup drawing with Y positive being upwards not downwards
    QPainter painter(this);
    painter.translate(0, height()-1);
    painter.scale(1.0, -1.0);

    // draw existing shapes
    painter.setPen(QColor(200, 200, 200));
    for(std::size_t i = 0;i < m_GhostShapes.size();i++) {
        painter.drawPath(SHAPEtoQPainterPath(m_GhostShapes[i]));
    }
    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        painter.drawPath(SHAPEtoQPainterPath(m_shapes[i]));
    }

    // draw currently "pen" tool
    DrawShape(&painter);
}

void GeometryPlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (!rect().contains(event->pos()))
            return;
        const QPointF pt(mapToPlot(event->pos()));
        m_curP.x = static_cast<double>(pt.x());
        m_curP.y = static_cast<double>(pt.y());
        m_snap = GetNearestTerminal(&m_curP);
        m_pivots.push_back(m_curP);
        UpdateShapes();
        update();
    }
    else if (event->button() == Qt::RightButton)
    {
        setTool(-1);
    }
}

void GeometryPlot::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (!rect().contains(event->pos()))
        return;
    const QPointF pt = mapToPlot(event->pos());
    m_curP = TERMINAL(pt.x(), pt.y());
    m_snap = GetNearestTerminal(&m_curP);
    emit pointHovered(TERMINALtoQPointF(m_curP));
    update();
}

/*void GeometryPlot::mouseReleaseEvent(QMouseEvent *event)
{
}*/

void GeometryPlot::UpdateShapes() {
    bool bCreated = false;

    switch (m_nShapeKind)
    {
    case GBAPY_LINE:
        {
            if (m_pivots.size() < 2) break;
            SHAPE sp;
            TERMINAL t1 = m_pivots[0];
            TERMINAL t2 = m_pivots[1];
            TERMINAL r1, r2;
            if (t1.isEqual(t2)) break;

            int	prevShapeIndex1 = FindShape(t1, &r1);
            int prevShapeIndex2 = FindShape(t2, &r2);

            if (prevShapeIndex1 == -1 && prevShapeIndex2 == -1) {
                sp.prims.push_back(std::make_unique<LINE>(t1, t2));
                m_shapes.push_back(sp);
            }
            else if (prevShapeIndex1 == -1) {
                m_shapes[prevShapeIndex2].prims.push_back(std::make_unique<LINE>(r1, r2));
                m_shapes[prevShapeIndex2].update();
            }
            else if (prevShapeIndex2 == -1) {
                m_shapes[prevShapeIndex1].prims.push_back(std::make_unique<LINE>(r1, r2));
                m_shapes[prevShapeIndex1].update();
            }
            else if (prevShapeIndex1 == prevShapeIndex2) {
                m_shapes[prevShapeIndex1].prims.push_back(std::make_unique<LINE>(r1, r2));
                m_shapes[prevShapeIndex1].update();
            }
            else{
                m_shapes[prevShapeIndex1].prims.push_back(std::make_unique<LINE>(r1, r2));
                MergeShape(prevShapeIndex1, prevShapeIndex2);
                m_shapes[prevShapeIndex1].update();
            }
            m_pivots.clear();
            m_pivots.push_back(t2);
            bCreated = true;
        }
        break;
    case GBAPY_BOX:
        if (m_pivots.size() > 1) {
            TERMINAL cur = m_pivots[1];
            TERMINAL piv = m_pivots[0];
            SHAPE shp;

            if (cur.isEqual(piv)) break;
            if(std::abs(cur.x - piv.x) < EP) break;
            if(std::abs(cur.y - piv.y) < EP) break;
            double x1 = cur.x < piv.x ? cur.x : piv.x;
            double x2 = cur.x > piv.x ? cur.x : piv.x;
            double y1 = cur.y < piv.y ? cur.y : piv.y;
            double y2 = cur.y > piv.y ? cur.y : piv.y;

            shp.prims.push_back(std::make_unique<LINE>(TERMINAL(x1, y1), TERMINAL(x2, y1)));
            shp.prims.push_back(std::make_unique<LINE>(TERMINAL(x2, y1), TERMINAL(x2, y2)));
            shp.prims.push_back(std::make_unique<LINE>(TERMINAL(x2, y2), TERMINAL(x1, y2)));
            shp.prims.push_back(std::make_unique<LINE>(TERMINAL(x1, y2), TERMINAL(x1, y1)));
            shp.update();

            m_shapes.push_back(shp);
            m_pivots.clear();
            bCreated = true;
        }
        break;
    case GBAPY_ROUND:
        {
            if (m_pivots.size() < 2) break;
            TERMINAL cur = m_pivots[1];
            TERMINAL piv = m_pivots[0];
            TERMINAL sp, ep;

            if (cur.isEqual(piv)) break;
            if(std::abs(cur.x - piv.x) < EP) break;
            if(std::abs(cur.y - piv.y) < EP) break;

            if (piv.x < cur.x) {
                sp.x = piv.x; ep.x = cur.x;
            }
            else{
                sp.x = cur.x; ep.x = piv.x;
            }
            if (piv.y < cur.y) {
                sp.y = piv.y; ep.y = cur.y;
            }
            else{
                sp.y = cur.y; ep.y = piv.y;
            }
            double width = (ep.x - sp.x) * 0.1f;
            double height = (ep.y - sp.y) * 0.1f;
            SHAPE shp;

            width = width < height ? width : height;
            height = width;

            shp.prims.push_back(
                      std::make_unique<ARC>(TERMINAL(sp.x + width, sp.y + height), width, 180.0f, 270.0f, false));
            shp.prims.push_back(
                      std::make_unique<ARC>(TERMINAL(ep.x - width, sp.y + height), width, 270.0f, 360.0f, false));
            shp.prims.push_back(
                      std::make_unique<ARC>(TERMINAL(ep.x - width, ep.y - height), width, 0.0f, 90.0f, false));
            shp.prims.push_back(
                      std::make_unique<ARC>(TERMINAL(sp.x + width, ep.y - height), width, 90.0f, 180.0f, false));
            shp.prims.push_back(
                      std::make_unique<LINE>(shp.prims[0]->terms[1], shp.prims[1]->terms[0]));
            shp.prims.push_back(
                      std::make_unique<LINE>(shp.prims[1]->terms[1], shp.prims[2]->terms[0]));
            shp.prims.push_back(
                      std::make_unique<LINE>(shp.prims[2]->terms[1], shp.prims[3]->terms[0]));
            shp.prims.push_back(
                      std::make_unique<LINE>(shp.prims[3]->terms[1], shp.prims[0]->terms[0]));
            shp.update();

            m_shapes.push_back(shp);
            m_pivots.clear();
            bCreated = true;
        }
        break;
    case GBAPY_CIRCLE:
        {
            if (m_pivots.size() < 2) break;
            TERMINAL cur = m_pivots[1];
            TERMINAL piv = m_pivots[0];

            if (cur.isEqual(piv)) break;
            TERMINAL r = TERMINAL(cur.x - piv.x, cur.y - piv.y);
            double rd = sqrt(r.x * r.x + r.y * r.y);
            SHAPE shp;

            shp.prims.push_back(
                        std::make_unique<ARC>(piv, rd, 0.0f, 90.0f, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, rd, 90.0f, 180.0f, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, rd, 180.0f, 270.0f, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, rd, 270.0f, 0.0f, false));
            shp.prims[0]->terms[0] = shp.prims[3]->terms[1];
            shp.prims[1]->terms[0] = shp.prims[0]->terms[1];
            shp.prims[2]->terms[0] = shp.prims[1]->terms[1];
            shp.prims[3]->terms[0] = shp.prims[2]->terms[1];
            shp.update();

            m_shapes.push_back(shp);
            m_pivots.clear();
            bCreated = true;
        }
        break;
    case GBAPY_STICK:
        {
            if (m_pivots.size() < 2) break;
            SHAPE shp;
            TERMINAL p1 = m_pivots[m_pivots.size() - 2];
            TERMINAL p2 = m_pivots[m_pivots.size() - 1];
            VERTEX up = VERTEX(0, 0, 1);
            VERTEX v = VERTEX(p2.x - p1.x, p2.y - p1.y, 0);
            double r = 10.0f;

            v = up.crossProduct(v);
            TERMINAL tp = TERMINAL(p1.x + v.x, p1.y + v.y);
            double a1 = p1.angleTo(tp);
            tp = TERMINAL(p1.x - v.x, p1.y - v.y);
            double a2 = p1.angleTo(tp);
            shp.prims.push_back(
                        std::make_unique<ARC>(p1, r, a1, a2, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(p2, r, a2, a1, false));
            shp.insertPrimitive(
                        std::make_unique<LINE>(shp.prims[0]->terms[1], shp.prims[1]->terms[0]), 1);
            shp.prims.push_back(
                        std::make_unique<LINE>(shp.prims[2]->terms[1], shp.prims[0]->terms[0]));
            shp.update();
            m_shapes.push_back(shp);
            m_pivots.clear();
            bCreated = true;
        }
        break;
    case GBAPY_DONUT:
        {
            if (m_pivots.size() < 2) break;
            TERMINAL cur = m_pivots[1];
            TERMINAL piv = m_pivots[0];
            double r = piv.distanceTo(cur);
            double delta = 10.0f;

            SHAPE shp1;

            shp1.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 0.0f, 90.0f, false));
            shp1.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 90.0f, 180.0f, false));
            shp1.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 180.0f, 270.0f, false));
            shp1.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 270.0f, 0.0f, false));
            shp1.update();

            m_shapes.push_back(shp1);
            if (r - delta > 0) {
                SHAPE shp2;
                r -= delta;
                shp2.prims.push_back(
                            std::make_unique<ARC>(piv, r, 0.0f, 270.0f, true));
                shp2.prims.push_back(
                            std::make_unique<ARC>(piv, r, 270.0f, 180.0f, true));
                shp2.prims.push_back(
                            std::make_unique<ARC>(piv, r, 180.0f, 90.0f, true));
                shp2.prims.push_back(
                            std::make_unique<ARC>(piv, r, 90.0f, 0.0f, true));
                shp2.update();

                m_shapes.push_back(shp2);
            }
            m_pivots.clear();
            bCreated = true;
        }
        break;
    case GBAPY_ARC:
        {
            if (m_pivots.size() < 3) break;
            TERMINAL cur = m_pivots[2];
            TERMINAL p1 = m_pivots[0];
            TERMINAL p2 = m_pivots[1];
            TERMINAL r1, r2, r3;
            VERTEX up = VERTEX(0, 0, 1);
            if (p1.isEqual(p2)) break;
            int	prevShapeIndex1 = FindShape(p1, &r1);
            int prevShapeIndex2 = FindShape(p2, &r2);
            FindShape(cur, &r3);

            p1 = r1; p2 = r2; cur = r3;

            TERMINAL p0;
            TERMINAL p1_2 = TERMINAL((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);
            VERTEX v1 = VERTEX(p1.x - p2.x, p1.y - p2.y, 0);
            VERTEX v2 = VERTEX(cur.x - p2.x, cur.y - p2.y, 0);
            VERTEX c_p1, c_p2;
            double r = sqrt(v1.x * v1.x + v1.y * v1.y);
            SHAPE shp;

            v2 = v1.crossProduct(v2);
            r = v2.z / r;
            v1 = v1.crossProduct(up);
            v1.normalize();
            p0.x = p1_2.x - v1.x * r; p0.y = p1_2.y - v1.y * r;

            r = sqrt((p0.x - p1.x) * (p0.x - p1.x) + (p0.y - p1.y) * (p0.y - p1.y));

            if (prevShapeIndex1 == -1 && prevShapeIndex2 == -1) {
                shp.prims.push_back(
                            std::make_unique<ARC>(p0, p1, p2, true));
                m_shapes.push_back(shp);
            }
            else if (prevShapeIndex1 == -1) {
                m_shapes[prevShapeIndex2].prims.push_back(
                            std::make_unique<ARC>(p0, p1, p2, true));
                m_shapes[prevShapeIndex2].update();
            }
            else if (prevShapeIndex2 == -1) {
                m_shapes[prevShapeIndex1].prims.push_back(
                            std::make_unique<ARC>(p0, p1, p2, true));
                m_shapes[prevShapeIndex1].update();
            }
            else if (prevShapeIndex1 == prevShapeIndex2) {
                m_shapes[prevShapeIndex1].prims.push_back(
                            std::make_unique<ARC>(p0, p1, p2, true));
                m_shapes[prevShapeIndex1].update();
            }
            else{
                m_shapes[prevShapeIndex1].prims.push_back(
                            std::make_unique<ARC>(p0, p1, p2, true));
                MergeShape(prevShapeIndex1, prevShapeIndex2);
                m_shapes[prevShapeIndex1].update();
            }

            m_pivots.clear();
            bCreated = true;
        }
        break;
    default:
        break;
    }
    if (bCreated) {
        doBooleanOPT();
        BackupShape();
        ExtractSnapPivots();
    }
}

int GeometryPlot::FindShape(const TERMINAL &t, PTERMINAL ret) {
    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        for (std::size_t j = 0; j < m_shapes[i].prims.size(); j++) {
            if (m_shapes[i].prims[j]->terms[0].isEqual(t)) {
                ret->x = m_shapes[i].prims[j]->terms[0].x;
                ret->y = m_shapes[i].prims[j]->terms[0].y;
                return static_cast<int>(i);
            }
            if (m_shapes[i].prims[j]->terms[1].isEqual(t)) {
                ret->x = m_shapes[i].prims[j]->terms[1].x;
                ret->y = m_shapes[i].prims[j]->terms[1].y;
                return static_cast<int>(i);
            }
        }
    }
    ret->x = t.x; ret->y = t.y;
    return -1;
}

void GeometryPlot::MergeShape(int index1, int index2) {
    m_shapes[index1].prims.insert(m_shapes[index1].prims.end(),
                                  std::make_move_iterator(m_shapes[index2].prims.begin()),
                                  std::make_move_iterator(m_shapes[index2].prims.end()));
    RemoveShape(index2);
}

void GeometryPlot::RemoveShape(int index) {
    m_shapes.erase(m_shapes.begin() + index);
}

bool GeometryPlot::GetNearestTerminal(PTERMINAL p) {
    TERMINAL mnt;
    double mn = -1.0f;

    if (m_snapPivots.size() == 0) return false;

    for (std::size_t i = 0; i < m_snapPivots.size(); i++) {
        TERMINAL t = m_snapPivots[i];
        double d = t.distanceTo(*p);

        if (i == 0) {
            mn = d;
            mnt = t;
        }
        else{
            if (d < mn) {
                mn = d;
                mnt = t;
            }
        }
    }
    if (mn < 10.0f) {
        p->x = mnt.x;
        p->y = mnt.y;
        return true;
    }
    return false;
}

void GeometryPlot::ExtractSnapPivots() {
    if (m_shapes.size() == 0) return;
    m_snapPivots.clear();
    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        for (std::size_t j = 0; j < m_shapes[i].prims.size(); j++) {
            TERMINAL st1;
            TERMINAL st2;

            m_snapPivots.push_back(m_shapes[i].prims[j]->terms[0]);
            m_snapPivots.push_back(m_shapes[i].prims[j]->terms[1]);
            m_snapPivots.push_back(m_shapes[i].prims[j]->center);
            for (std::size_t k = 0; k < m_shapes[i].prims.size(); k++) {
                if (j == k) continue;


                if (isConflict(m_shapes[i].prims[j].get(), m_shapes[i].prims[k].get(), &st1, &st2)) {
                    if (st1.isValid) m_snapPivots.push_back(st1);
                    if (st2.isValid) m_snapPivots.push_back(st2);
                }
            }
            for (std::size_t k = 0; k < m_shapes.size(); k++) {
                if (i == k) continue;
                for (std::size_t n = 0; n < m_shapes[k].prims.size(); n++) {
                    if (isConflict(m_shapes[i].prims[j].get(), m_shapes[k].prims[n].get(), &st1, &st2)) {
                        if (st1.isValid) m_snapPivots.push_back(st1);
                        if (st2.isValid) m_snapPivots.push_back(st2);
                    }
                }
            }
        }
    }
}

void GeometryPlot::DrawShape(QPainter *painter)
{
    DrawCurrentPen(painter); // draw actively used cursor/tool
}

static QPointF TERMINALtoQPointF(const TERMINAL &pt)
{
    return QPointF(pt.x, pt.y);
}

static QLineF LINEtoQLineF(const LINE &line)
{
    return QLineF(TERMINALtoQPointF(line.terms[0]), TERMINALtoQPointF(line.terms[1]));
}

static QPainterPath SHAPEtoQPainterPath(const SHAPE &shape)
{
    QPainterPath path;
    for (std::vector<std::unique_ptr<PRIMITIVE>>::const_iterator it = shape.prims.begin(); it != shape.prims.end(); ++it)
    {
        const PRIMITIVE * const prim = it->get();
        switch (prim->nKind)
        {
            case GBAPY_LINE:
            {
                const LINE * const line = dynamic_cast<const LINE*>(prim);
                // if (it == shape.prims.begin())
                    path.moveTo(TERMINALtoQPointF(line->terms[0]));
                path.lineTo(TERMINALtoQPointF(line->terms[1]));
            }
            break;

            case GBAPY_ARC:
            {
                const ARC * const arc = dynamic_cast<const ARC*>(prim);
                double sa = arc->startAngle;
                double ea = arc->endAngle;

                arc->makeAbsoluteAngles(&sa, &ea);
                const QRectF circleRect(arc->center.x - arc->radius, arc->center.y - arc->radius, arc->radius*2.0, arc->radius*2.0);
                sa = -sa;
                ea = -ea;
                // if (it == shape.prims.begin())
                    path.arcMoveTo(circleRect, sa);
                path.arcTo(circleRect, sa, ea - sa);
            }
            break;
        }
    }
    return path;
}

void GeometryPlot::DrawCurrentPen(QPainter *painter)
{
    VERTEX up = VERTEX(0, 0, 1);

    // draw crosshairs
    painter->setPen(QColor(100, 100, 100));
    painter->drawLine(QPointF(0, m_curP.y), QPointF(width()-1, m_curP.y));
    painter->drawLine(QPointF(m_curP.x, 0), QPointF(m_curP.x, height()-1));

    // possibly draw snap indicator box
    if (m_snap) {
        painter->setPen(Qt::red);
        const QRectF snapBox(TERMINALtoQPointF(m_curP) - QPointF(10, 10), QSizeF(20, 20));
        painter->drawRect(snapBox);
    }

    // draw current tool
    painter->setPen(Qt::white);
    switch (m_nShapeKind)
    {
        case GBAPY_LINE:
        {
            if (m_pivots.size() < 1) break;
            const TERMINAL cur = m_curP;
            const TERMINAL piv = m_pivots[m_pivots.size() - 1];
            const LINE ln = LINE(cur, piv);
            painter->drawLine(LINEtoQLineF(ln));
        }
        break;

        case GBAPY_BOX:
        {
            if (m_pivots.size() < 1) break;
            TERMINAL cur = m_curP;
            TERMINAL piv = m_pivots[m_pivots.size() - 1];
            painter->drawRect(QRectF(TERMINALtoQPointF(cur), TERMINALtoQPointF(piv)).normalized());
        }
        break;
        case GBAPY_ROUND:
        {
            if (m_pivots.size() < 1) break;
            TERMINAL cur = m_curP;
            TERMINAL piv = m_pivots[m_pivots.size() - 1];
            TERMINAL sp, ep;

            if (piv.x < cur.x) {
                sp.x = piv.x; ep.x = cur.x;
            }
            else{
                sp.x = cur.x; ep.x = piv.x;
            }
            if (piv.y < cur.y) {
                sp.y = piv.y; ep.y = cur.y;
            }
            else{
                sp.y = cur.y; ep.y = piv.y;
            }
            double width = (ep.x - sp.x) * 0.1f;
            double height = (ep.y - sp.y) * 0.1f;
            SHAPE shp;

            width = width < height ? width : height;
            height = width;

            shp.prims.push_back(std::make_unique<ARC>(TERMINAL(sp.x + width, sp.y + height), width, 180.0f, 270.0f, false));
            shp.prims.push_back(std::make_unique<ARC>(TERMINAL(ep.x - width, sp.y + height), width, 270.0f, 360.0f, false));
            shp.prims.push_back(std::make_unique<ARC>(TERMINAL(ep.x - width, ep.y - height), width, 0.0f, 90.0f, false));
            shp.prims.push_back(std::make_unique<ARC>(TERMINAL(sp.x + width, ep.y - height), width, 90.0f, 180.0f, false));
            shp.prims.push_back(std::make_unique<LINE>(shp.prims[0]->terms[1], shp.prims[1]->terms[0]));
            shp.prims.push_back(std::make_unique<LINE>(shp.prims[1]->terms[1], shp.prims[2]->terms[0]));
            shp.prims.push_back(std::make_unique<LINE>(shp.prims[2]->terms[1], shp.prims[3]->terms[0]));
            shp.prims.push_back(std::make_unique<LINE>(shp.prims[3]->terms[1], shp.prims[0]->terms[0]));

            painter->drawPath(SHAPEtoQPainterPath(shp));
            shp.clear();
        }
        break;
        case GBAPY_STICK:
        {
            if (m_pivots.size() < 1) break;
            SHAPE shp;
            TERMINAL p1 = m_pivots[m_pivots.size() - 1];
            TERMINAL p2 = m_curP;
            VERTEX up = VERTEX(0, 0, 1);
            VERTEX v = VERTEX(p2.x - p1.x, p2.y - p1.y, 0);
            double r = 10.0f;

            v = up.crossProduct(v);
            TERMINAL tp = TERMINAL(p1.x + v.x, p1.y + v.y);
            double a1 = p1.angleTo(tp);
            tp = TERMINAL(p1.x - v.x, p1.y - v.y);
            double a2 = p1.angleTo(tp);
            shp.prims.push_back(
                        std::make_unique<ARC>(p1, r, a1, a2, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(p2, r, a2, a1, false));
            shp.insertPrimitive(
                        std::make_unique<LINE>(shp.prims[0]->terms[1], shp.prims[1]->terms[0]), 1);
            shp.prims.push_back(
                        std::make_unique<LINE>(shp.prims[2]->terms[1], shp.prims[0]->terms[0]));
            painter->drawPath(SHAPEtoQPainterPath(shp));
            shp.clear();
        }
        break;
        case GBAPY_DONUT:
        {
            if (m_pivots.size() < 1) break;
            TERMINAL cur = m_curP;
            TERMINAL piv = m_pivots[m_pivots.size() - 1];
            double r = piv.distanceTo(m_curP);
            double delta = 10.0f;

            SHAPE shp;

            shp.prims.push_back(
                        std::make_unique<LINE>(cur, piv));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 0.0f, 90.0f, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 90.0f, 180.0f, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 180.0f, 270.0f, false));
            shp.prims.push_back(
                        std::make_unique<ARC>(piv, r + delta, 270.0f, 0.0f, false));
            if (r - delta > 0) {
                r -= delta;
                shp.prims.push_back(
                            std::make_unique<ARC>(piv, r, 0.0f, 270.0f, true));
                shp.prims.push_back(
                            std::make_unique<ARC>(piv, r, 270.0f, 180.0f, true));
                shp.prims.push_back(
                            std::make_unique<ARC>(piv, r, 180.0f, 90.0f, true));
                shp.prims.push_back(
                            std::make_unique<ARC>(piv, r, 90.0f, 0.0f, true));
            }
            painter->drawPath(SHAPEtoQPainterPath(shp));
            shp.clear();
        }
        break;
        case GBAPY_CIRCLE:
        {
            if (m_pivots.size() < 1) break;
            TERMINAL cur = m_curP;
            TERMINAL piv = m_pivots[m_pivots.size() - 1];
            TERMINAL r = TERMINAL(cur.x - piv.x, cur.y - piv.y);
            double rd = sqrt(r.x * r.x + r.y * r.y);
            SHAPE shp;

            shp.prims.push_back(std::make_unique<LINE>(cur, piv));
            shp.prims.push_back(std::make_unique<ARC>(piv, rd, 0.0f, 90.0f, false));
            shp.prims.push_back(std::make_unique<ARC>(piv, rd, 90.0f, 180.0f, false));
            shp.prims.push_back(std::make_unique<ARC>(piv, rd, 180.0f, 270.0f, false));
            shp.prims.push_back(std::make_unique<ARC>(piv, rd, 270.0f, 0.0f, false));
            painter->drawPath(SHAPEtoQPainterPath(shp));
            shp.clear();
        }
        break;
        case GBAPY_ARC:
        {
            if (m_pivots.size() < 1) break;
            if (m_pivots.size() < 2) {
                TERMINAL cur = m_curP;
                TERMINAL piv = m_pivots[m_pivots.size() - 1];
                LINE ln = LINE(cur, piv);
                painter->drawLine(LINEtoQLineF(ln));
            }
            if (m_pivots.size() > 1) {
                TERMINAL cur = m_curP;
                TERMINAL p1 = m_pivots[m_pivots.size() - 2];
                TERMINAL p2 = m_pivots[m_pivots.size() - 1];
                TERMINAL p1_2 = TERMINAL((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);
                TERMINAL p0;
                VERTEX v1 = VERTEX(p1.x - p2.x, p1.y - p2.y, 0);
                VERTEX v2 = VERTEX(cur.x - p2.x, cur.y - p2.y, 0);
                VERTEX c_p1, c_p2;
                double r = sqrt(v1.x * v1.x + v1.y * v1.y);
                SHAPE shp;

                if (r == 0) break;

                v2 = v1.crossProduct(v2);
                r = v2.z / r;
                v1 = v1.crossProduct(up);
                v1.normalize();
                p0.x = p1_2.x - v1.x * r; p0.y = p1_2.y - v1.y * r;

                r = sqrt((p0.x - p1.x) * (p0.x - p1.x) + (p0.y - p1.y) * (p0.y - p1.y));

                shp.prims.push_back(std::make_unique<ARC>(p0, p1, p2, true));
                shp.prims.push_back(std::make_unique<LINE>(p1, p2));
                shp.prims.push_back(std::make_unique<LINE>(p0, p1));
                shp.prims.push_back(std::make_unique<LINE>(p0, p2));

                painter->drawPath(SHAPEtoQPainterPath(shp));
                shp.clear();
            }
        }
        break;
    }
}

bool GeometryPlot::getIntersection(int shpIndex, PRIMITIVE *pr, TERMINAL *t, int *retIndex, int *retShapeIndex, SHAPE **shp) {
    double mn = M_INFINITE;
    bool ret = false;
    int idx;
    TERMINAL p;
    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        if ((int)i == shpIndex) continue;
        if (m_shapes[i].isIntersected == false) continue;
        if (m_shapes[i].isCompleted == false) continue;
        if (m_shapes[i].getSelfIntersection(-1, pr, &p, &idx)) {
            double m = p.distanceTo(pr->terms[0]);
            if (m < mn) {
                *shp = &(m_shapes[i]);
                mn = m;
                *retIndex = idx;
                *retShapeIndex = i;
                *t = p;
                ret = true;
            }
        }
    }
    return ret;
}

bool GeometryPlot::IsIntersected(SHAPE *shp1, SHAPE *shp2) {
    for (std::size_t i = 0; i < shp1->prims.size(); i++) {
        TERMINAL t;
        int index;
        if (shp2->getSelfIntersection(-1, shp1->prims[i].get(), &t, &index)) {
            return true;
        }
    }
    return false;
}

bool GeometryPlot::doShapeBooleanOPT(SHAPE *shp, int shpIndex, std::vector<SHAPE> *subShapes) {
    bool retFlag = false;

    for (std::size_t i = 0; i < shp->prims.size(); i++) {
        TERMINAL t;
        SHAPE *intersected = NULL;
        int primIndex;
        int otherShapeIndex;
        std::unique_ptr<PRIMITIVE> pr = shp->prims[i]->clone();

        if (pr == NULL) continue;
        while(getIntersection(shpIndex, pr.get(), &t, &primIndex, &otherShapeIndex, &intersected)) {
            SHAPE tshp;
            TERMINAL st = pr->terms[0];
            TERMINAL et = t;
            std::unique_ptr<PRIMITIVE> prim = shp->prims[i]->clone(st, et);

            pr.reset(nullptr);
            retFlag = true;

            if (prim == NULL) break;
            if (m_shapes[otherShapeIndex].isInsidePoint(prim.get(), primIndex))
            {
                prim.reset(nullptr);
                pr = shp->prims[i]->clone(et);
                if (pr == NULL) break;
                continue;
            }

            tshp.prims.push_back(std::move(prim));
            while (true)
            {
                int m = primIndex;
                std::unique_ptr<PRIMITIVE> pr1 = intersected->prims[primIndex]->clone(t);
                if (pr1 == NULL) break;
                if (getIntersection(otherShapeIndex, pr1.get(), &t, &primIndex, &otherShapeIndex, &intersected)) {
					std::unique_ptr<PRIMITIVE> pr2 = pr1->clone(pr1->terms[0], t);
					if (pr2 == NULL) break;
                    tshp.prims.push_back(std::move(pr2));
                }
                else{
                    tshp.prims.push_back(pr1->clone());
                    primIndex = m;
                    primIndex++;
                    if ((std::size_t)primIndex >= intersected->prims.size()) primIndex = 0;
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
            pr = shp->prims[i]->clone(et);
            if (pr == NULL) break;
        }
        if(pr != NULL) pr.reset(nullptr);
    }
    if (subShapes->size() > 1) {
        removeDuplicated(subShapes);
    }
    if (retFlag == false) {
        int n = 0;
        for (std::size_t i = 0; i < m_shapes.size(); i++) {
            if (static_cast<int>(i) == shpIndex) continue;
            SHAPE *cloneShape = m_shapes[i].clone();
            if (cloneShape->isPositive == false) {
                cloneShape->turnPrimitiveOut();
            }
            if (cloneShape->isInsideShape(shp)) {
                n += m_shapes[i].isPositive ? +1 : -1;
            }
            cloneShape->clear();
            delete cloneShape;
        }
        if (shp->isPositive) n++;
        if(n > 1) shp->isValid = false;
    }
    return retFlag;
}

void GeometryPlot::doBooleanOPT() {
    if (m_shapes.size() < 2) return;
    std::vector<SHAPE> newShapes;

    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        m_shapes[i].isIntersected = true;
    }

    for (std::size_t i = 0; i < m_shapes.size(); i++) {
        if (m_shapes[i].isCompleted == false) {
            newShapes.push_back(m_shapes[i]);
            continue;
        }
        std::vector<SHAPE> subShapes;

        bool ret = doShapeBooleanOPT(&(m_shapes[i]), i, &subShapes);

        if (subShapes.size() > 0 || ret == true) {
            m_shapes[i].isValid = false;
            m_shapes[i].isIntersected = true;
            for (std::size_t j = 0; j < subShapes.size(); j++) {
                newShapes.push_back(subShapes[j]);
            }
        }
        else if(m_shapes[i].isValid) {
            newShapes.push_back(m_shapes[i]);
            m_shapes[i].isIntersected = false;
        }
    }
    ClearShapes(&m_shapes);
    ExtractSnapPivots();
    m_shapes.clear();
    removeDuplicated(&newShapes);
    for (std::size_t i = 0; i < newShapes.size(); i++) {
        m_shapes.push_back(newShapes[i]);
    }
}

} // namespace BooleanOffset

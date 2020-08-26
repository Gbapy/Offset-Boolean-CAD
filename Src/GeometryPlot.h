#ifndef BOOLEANOFFSET_GEOMETRYPLOT_H
#define BOOLEANOFFSET_GEOMETRYPLOT_H

#include "engine/terminal.h"
#include "engine/shape.h"

#include <QWidget>

#include <vector>

namespace BooleanOffset {

class GeometryPlot : public QWidget
{
    Q_OBJECT
public:
    GeometryPlot(QWidget *parent = nullptr);
    ~GeometryPlot();

    QPointF mapToPlot(const QPointF &pt) const;
    QPointF mapFromPlot(const QPointF &pt) const;
    void setTool(int tool);

    void open(const QString &filePath);
    void save(const QString &filePath);
public slots:
    void clear();
    void BooleanButtonFunction();
    void offset(double r);
    void ghostOffset(double r);
    void reload();
signals:
    void pointHovered(const QPointF &);
    void toolChanged(int);
protected:

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;

    void UpdateShapes();
    int FindShape(const TERMINAL &t, PTERMINAL ret);
    void MergeShape(int index1, int index2);
    void RemoveShape(int index);
    bool GetNearestTerminal(PTERMINAL p);
    void ExtractSnapPivots();
    void DrawShape(QPainter *painter);
    void DrawCurrentPen(QPainter *painter);
    bool getIntersection(int shpIndex, PRIMITIVE *pr, TERMINAL *t, int *retIndex, int *retShapeIndex, SHAPE **shp);
    bool IsIntersected(SHAPE *shp1, SHAPE *shp2);
    bool doShapeBooleanOPT(SHAPE *shp, int shpIndex, std::vector<SHAPE> *subShapes);

    void doBooleanOPT();
    void BackupShape();
    void RestoreShape();

    TERMINAL m_curP;
    int m_nShapeKind;
    std::vector<TERMINAL> m_pivots;
    std::vector<TERMINAL> m_snapPivots;
    std::vector<SHAPE> m_shapes;
    std::vector<SHAPE> m_reloadShapes;
    std::vector<SHAPE> m_GhostShapes;
    bool m_snap;
};

} // namespace BooleanOffset

#endif // BOOLEANOFFSET_GEOMETRYPLOT_H

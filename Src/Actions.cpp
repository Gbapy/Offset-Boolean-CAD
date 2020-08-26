#include "Actions.h"
#include "engine/global.h"

namespace BooleanOffset {

Actions::Actions(QObject *parent) : QObject(parent)
{
    // file menu
    fileNew = new QAction("&New", this);
    fileOpen = new QAction("&Open", this);
    fileSaveAs = new QAction("Save &As", this);
    fileQuit = new QAction("&Quit", this);
    // fileOpen->setEnabled(false);
    // fileSaveAs->setEnabled(false);

    // tool menu
    toolsGroup = new QActionGroup(this);
    toolsGroup->setExclusive(true);
    toolLine = toolsGroup->addAction("Line");
    toolBox = toolsGroup->addAction("Box");
    toolRoundedBox = toolsGroup->addAction("Rounded Box");
    toolCircle = toolsGroup->addAction("Circle");
    toolArc = toolsGroup->addAction("Arc");
    toolStick = toolsGroup->addAction("Stick");
    toolDonut = toolsGroup->addAction("Donut");
    toolLine->setIcon(QIcon(":/resources/icons/line.svg"));
    toolBox->setIcon(QIcon(":/resources/icons/box.svg"));
    toolRoundedBox->setIcon(QIcon(":/resources/icons/rounded-box.svg"));
    toolCircle->setIcon(QIcon(":/resources/icons/circle.svg"));
    toolArc->setIcon(QIcon(":/resources/icons/arc.svg"));
    toolStick->setIcon(QIcon(":/resources/icons/stick.svg"));
    toolDonut->setIcon(QIcon(":/resources/icons/donut.svg"));
    toolLine->setCheckable(true);
    toolBox->setCheckable(true);
    toolRoundedBox->setCheckable(true);
    toolCircle->setCheckable(true);
    toolArc->setCheckable(true);
    toolStick->setCheckable(true);
    toolDonut->setCheckable(true);
    toolLine->setData(GBAPY_LINE);
    toolBox->setData(GBAPY_BOX);
    toolRoundedBox->setData(GBAPY_ROUND);
    toolCircle->setData(GBAPY_CIRCLE);
    toolArc->setData(GBAPY_ARC);
    toolStick->setData(GBAPY_STICK); // TODO make GBAPY_NONE, GPABY_INVALID, etc.
    toolDonut->setData(GBAPY_DONUT); // TODO implement

    operationBoolean = new QAction("Boolean", this);
    operationOffsetOut = new QAction("Offset Out", this);
    operationOffsetIn = new QAction("Offset In", this);
    operationReload = new QAction("Reload", this);
    operationGhostMode = new QAction("Ghost Mode", this);
    operationGhostMode->setCheckable(true);
    operationGhostMode->setChecked(true);
}

} // namespace BooleanOffset

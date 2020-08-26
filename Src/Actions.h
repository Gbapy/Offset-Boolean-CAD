#ifndef BOOLEANOFFSET_ACTIONS_H
#define BOOLEANOFFSET_ACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>

namespace BooleanOffset {

class Actions : public QObject
{
    Q_OBJECT
public:
    Actions(QObject *parent = nullptr);
public:
    QAction *fileNew;
    QAction *fileOpen;
    QAction *fileSaveAs;
    QAction *fileQuit;

    QActionGroup *toolsGroup;
    QAction *toolLine;
    QAction *toolBox;
    QAction *toolRoundedBox;
    QAction *toolCircle;
    QAction *toolArc;
    QAction *toolStick;
    QAction *toolDonut;

    QAction *operationBoolean;
    QAction *operationOffsetOut;
    QAction *operationOffsetIn;
    QAction *operationReload;
    QAction *operationGhostMode;

};

} // namespace BooleanOffset

#endif // BOOLEANOFFSET_ACTIONS_H

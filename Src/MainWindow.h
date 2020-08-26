#ifndef BOOLEANOFFSET_MAINWINDOW_H
#define BOOLEANOFFSET_MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QActionGroup;
class QSettings;
class QLabel;
QT_END_NAMESPACE

namespace BooleanOffset {

class Actions;
class GeometryPlot;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected slots:
    void slot_FileOpen();
    void slot_FileSaveAs();
    void slot_ToolSelected(QAction *act);
    void slot_ToolChanged(int newTool);
    void slot_CoordinateHovered(const QPointF &pt);
protected:
    void closeEvent(QCloseEvent *event);
    void _saveSettings();
    void _loadSettings();

    Actions *_actions;
    GeometryPlot *_geomPlot;
    QLabel *_coordinateLabel;
    QString _currentDirectory;
    QSettings *_settings;
};

} // namespace BooleanOffset

#endif // BOOLEANOFFSET_MAINWINDOW_H

#ifndef TOOL_HPP_VP_2006_04_03
#define TOOL_HPP_VP_2006_04_03

#include "canvas.h"

#include <QSettings>
#include <QtWidgets/QWidget>

#include <boost/shared_ptr.hpp>

class QAction;
class QToolBar;

namespace vis4 {

class Trace_model;
class Event_model;
class MainWindow;

/** ������� ����� ��� ������������, ����������� � ����� sidebar.

   � ������ ������ ������� ����� ������ ���� ����������. Toolbar
   �������� ��������� ������� � ��������, �������������� �������
   ����������.

   ����� Tool ���������� �� QWidget � ��������������� ������������ �
   ����� sidebar.
*/
class Tool : public QWidget
{
    Q_OBJECT
public:
    Tool(QWidget* parent, Canvas* canvas);

    /** ���������� ������ QAction, ������� ���������� �� toolbar. */
    QAction* action();

    /** ������ ����� ���������� � ������ ����������� �����������,
        ����� �� ���������� ������� �� sidebar. */
    virtual void activate() = 0;

    /** ������ ����� ���������� � ������ ������������� �����������,
    ����� �� ���������� ��������� �� sidebar. */
    virtual void deactivate() = 0;

    /** Reset tool to initial state. */
    virtual void reset() {}

    /**
     *  ���������� �����, �� ������� ������ ���������� ����� ��������.
     */
    Canvas* canvas() const { return canvas_; }

    /** ����� ���������� ��� ������� ������ ���� �� ��������� ���������.
        ������� ����� ���������� � �������� �����������. ���� ��� ����
        ������������ true, �� ��������� �����������. � ��������� ������
        ���������� ������ ���� ��������� ������������. ������� ������ ��
        ��������� � ������������ �������� ������������.  */
    virtual bool mouseEvent(QEvent* event,
                          Canvas::clickTarget target,
                          int component,
                          State_model* state,
                          const common::Time& time,
                          bool events_near) { return false; }

    /** ����� ���������� ��� ����������� ���� �� ��������� ���������.
        ��������� ����� �� ��� � � canvasMouseEvent.  */
    virtual bool mouseMoveEvent(QMouseEvent* ev, Canvas::clickTarget target,
                                int component, const common::Time& time) { return false; }

signals:

    /** This signal is emitted when tool wants to be activated.
        This must be the only way to activate itself.
        __DO NOT CALL trigger() METHOD OF SELF ACTION__ */
    void activateMe();

    /**
     * ������ ������ ����� �������������� ��������� �������� ��� ����,
     * ����� �������� ��������� ���������� � ������� event. ������������
     * ������������ ��� ���� ���������� �������������.
     */
    void showEvent(Event_model* event);

    void showState(State_model* state);

    /**
     *  ������ ������ ����� �������������� ��������� �������� ��� ������������
     *  �� ��������� "������ ������ ������".
     */
    void browse();

protected:

    /** Preparing current path in QSettings to store settings per trace. */
    void prepare_settings(QSettings & settings) const;

    /**
     *  ���������� ������, � ������� �������� ������ ����������.
     */
    Trace_model::Ptr & model() const;

    /** Prevents handle keys like arrows as shortcut.  */
    bool event(QEvent * event);

private:
    /** ������� ������ ������� ������ Action, ������� ����� �������
        �� ������ ������������. ������ ������ ����� ���������  ������,
        � ��� ������� (objectName). ������� ������� ������ ��������������
        �� ������. */
    virtual QAction* createAction() = 0;

private:
    Canvas* canvas_;
    MainWindow * mainWnd_;
    QAction* action_;
};

/** ����������, ��������������� ��� ��������� �� ��������� ��������. ���� �� ������������
 ������ ������ ���� ������������ �� ����� ����, � �������� � MainWindow �������
������ MainWindow::installBrowser.
��������� �������� ���� � �����, �������� �� ������� ����� � ������, ��������� ��������,
� ����� ������ ������� � ��������� ���������� �� ������.
*/
class Browser : public Tool
{
    Q_OBJECT
public:
    Browser(QWidget* parent, Canvas* c);

    /** ���������� ������� MainWindow. ������ �������� ��������, �����������
    ��� ���������, � ����� toolbar. */
    virtual void addToolbarActions(QToolBar* toolbar) = 0;
    /** ��� ������ ������ �������� ��������� ���������� � ������� event. */
    virtual void doShowEvent(Event_model* event) = 0;

    virtual void doShowState(State_model* state) = 0;

};




Browser* createBrowser(QWidget* parent, Canvas* canvas);
Tool* createGoto(QWidget* parent, Canvas* canvas);
Tool* createMeasure(QWidget* parent, Canvas* canvas);
Tool* createFilter(QWidget* parent, Canvas* canvas);
Tool* createFind(QWidget* parent, Canvas* canvas);



}

#endif

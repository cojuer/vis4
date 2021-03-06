#ifndef CANVAS_H
#define CANVAS_H

#include <QtWidgets/QScrollArea>
#include <QPair>
#include <QTime>

#include "trace_model.h"
#include "trace_painter.h"
#include "time_vis.h"

class QKeyEvent;

namespace vis4 {

class StateModel;

/**
 * Класс для показа и взаимодействия с трассой.
 *
 * Показывает связанный с собой экземпляр Trace_model в виде временной
 * диаграммы и предоставляет интерфейсы, позволяющие инструментам получать уведомления о
 * действиях пользователя и добавлять дополнительные графические объекты.
 */
class Canvas : public QScrollArea
{
Q_OBJECT
public:
    Canvas(QWidget* parent);

    void setModel(TraceModelPtr model);
    void setCursor(const QCursor& c);

    TraceModelPtr& getModel() const;

    int getNearestLifeline(int y);


    QPoint lifeline_point(int component, const Time& time);

    QPair<Time, Time> nearby_range(const Time& time);

    QRect boundingRect(int component, const Time& min_time, const Time& max_time);

    /**
     * Тип объекта, находящегося под курсором
     * nothingClicked -- ничего интересного
     * rootClicked -- имя "корневого" компонента
     * componentClicked -- имя "обычного" компонента
     * stateClicked -- состояние
     * lifelinesClicked -- область линий жизни (исключая область
     *    где рисуются имена компонетов). Положение курсора в этом
     *    случае соответсвуюет показанному моменту времени.
     */
    enum clickTarget
    {
        nothingClicked = 1,
        componentClicked,
        stateClicked,
        lifelinesClicked
    };


    void addItem(class CanvasItem* item);
public slots:

    /** If true, uses maximally portable drawing mechanims, that
        don't rely on OS being decent, and not broken.  */
    void setPortableDrawing(bool);
signals:
    /** Сигнал, генерируемый при изменении модели методом setModel. */
    void modelChanged(TraceModelPtr & new_model);

    /**
     * Сигнал, генерируемый при нажатии на какую-либо кнопку мышки.
     * Этот сигнал не должен использоваться инструментами. Вместо
     * этого, они должны переопределять соответствующий метод класса Tool.
     * @param event  Исходное событие Qt
     * @param target Тип объекта, находящегося под курсором
     * @param component Номер ближайшего с курсору компонента
     * @param state Указатель на состояние под курсором (если есть)
     * @param time Время, которому соответсвует положение курсора
     * @param events_near Флаг, говорящий есть ли поблизости к курсору события
     */
    void mouseEvent(QEvent* event,
                    Canvas::clickTarget target,
                    int component,
                    StateModel* state,
                    const Time& time,
                    bool events_near);

    /**
     * Сигнал, генерируемый при движении мышкой.
     * Этот сигнал не должен использоваться инструментами. Вместо
     * этого, они должны переопределять соответвстующий метод класса Tool.
     * @param event  Исходное событие Qt
     * @param target Тип объекта, находящегося под курсором
     * @param component Номер ближайшего с курсору компонента
     * @param time Время, которому соответсвует положение курсора
     */
    void mouseMoveEvent(QMouseEvent* event, Canvas::clickTarget target,
                        int component, const Time& time);

private slots:

    void timeSettingsChanged();

private: /** overloaded methods */

    void resizeEvent(QResizeEvent* event);
    void timerEvent(QTimerEvent* event);
    void closeEvent(QCloseEvent* event);
    void scrollContentsBy(int dx, int dy);

private: /** members */

    class Contents_widget* contents_;
    class Timeline* timeline_;

    /** Timer is used to prevent canvas redrawing while
        resizing is not finished. */
    int updateTimer;

    friend class MainWindow;
    friend class Contents_widget;

};

class Contents_widget : public QWidget
{
    Q_OBJECT
public:

    Contents_widget(Canvas* parent);

    /**
     * If force is true, always redraw, don't suppress redraw
     * if the model seem unchanged.
     */
    void setModel(TraceModelPtr model, bool force = false);

    TraceModelPtr model() const;

    void scrolledBy(int dx, int dy);

    QRect boundingRect(int component,
                       const Time& min_time, const Time& max_time);

private: /** methods */

    QRect drawBaloon(QPainter* painter);

    void targetUnderCursor(
        const QPoint& pos, Canvas::clickTarget* target,
        int* component, StateModel** state, bool* events_near);

private: /** QWidget overrides */

    /** Widget's events hook.
     * This is overloaded virtual function, that cathes QEvet::ToolTip event
     * and processes it.
     *
     * For more help see Qt documentation.
    */
    bool event(QEvent *event);

    void paintEvent(QPaintEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

private: /** members */

    Canvas* parent_;
    TraceModelPtr model_;
    std::auto_ptr<TracePainter> trace_painter;
    std::auto_ptr<TraceGeometry> trace_geometry;

    QPaintDevice* paintBuffer;
    bool portable_drawing;

    int visir_position;
    QRect ballon;

    // FIXME: delete all items in destructor.
    std::vector<CanvasItem*> items;

private slots: /** support for background drawing */

    /** Periodically updates canvas and show current state of drawing. */
    void timerTick();

private:

    void doDrawing(bool start_in_background);

    bool pendingRedraw;

    /** Painter timer. Involves repaint for showing intermediate results of drawing. */
    QTimer* painter_timer;

    friend class Canvas;
};

} /** namespaces */

#endif

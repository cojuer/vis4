#ifndef TIMELINE_H
#define TIMELINE_H

#include <QtWidgets/QWidget>

namespace vis4 {

class Trace_painter;
class TimeUnitControl;

class Timeline : public QWidget
{
    Q_OBJECT
public:
    Timeline(QWidget* parent, Trace_painter* painter);
    /** QWidget overides */
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
signals:
    void timeSettingsChanged();
protected:
    void paintEvent(QPaintEvent*);
private:
    Trace_painter* tp;
    TimeUnitControl* timeUnitControl;
};

}
#endif

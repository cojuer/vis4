#ifndef TIMELINE_H
#define TIMELINE_H

#include <QtWidgets/QWidget>

namespace vis4 {

class TracePainter;
class TimeUnitControl;

class Timeline : public QWidget
{
    Q_OBJECT
public:
    Timeline(QWidget* parent, TracePainter* painter);
    /** QWidget overides */
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
signals:
    void timeSettingsChanged();
protected:
    void paintEvent(QPaintEvent*);
private:
    TracePainter* tp;
    TimeUnitControl* timeUnitControl;
};

}
#endif

#ifndef TIMEUNIT_CONTROL_H
#define TIMEUNIT_CONTROL_H

#include <QtWidgets/QWidget>

class QStyleOptionComboBox;
class QMenu;

namespace vis4 {

/**
 * Widget that displays and controls unit and format settings.
 */
class TimeUnitControl : public QWidget
{
    Q_OBJECT
public: /* methods */
    TimeUnitControl(QWidget *parent);
    QSize sizeHint() const;
signals:
    void timeSettingsChanged();
protected: /* methods */
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void initStyleOption(QStyleOptionComboBox *option) const;
private slots:
    void menuActionTriggered(QAction * action);
private: /* members */
    QMenu* menu;
};

} // namespaces

#endif

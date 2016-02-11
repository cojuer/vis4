#ifndef EVENT_LIST_H
#define EVENT_LIST_H

#include <QtWidgets/QTreeView>
#include <QStandardItemModel>
#include <QVector>

#include "trace_model.h"

namespace vis4 {

class EventModel;

class EventList : public QTreeView
{
    Q_OBJECT
public:
    EventList(QWidget* parent);

    void showEvents(TraceModelPtr& model, const Time& time);

    void updateTime();

    QSize sizeHint() const;
    void scrollTo(const QModelIndex & index, ScrollHint hint = EnsureVisible);

    EventModel* currentEvent();
    void setCurrentEvent(EventModel* event);

signals:
    void currentEventChanged(EventModel*);//? what slot is it used with
private slots:
    void eventListRowChanged();
private:
    QVector<EventModel*> events;
};

}
#endif

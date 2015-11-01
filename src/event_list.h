#ifndef EVENT_LIST_H
#define EVENT_LIST_H

#include <QtWidgets/QTreeView>
#include <QStandardItemModel>
#include <QVector>

#include "trace_model.h"

namespace vis4 {

class Event_model;

class Event_list : public QTreeView
{
    Q_OBJECT
public:
    Event_list(QWidget* parent);

    void showEvents(TraceModelPtr& model, const Time & time);

    void updateTime();

    QSize sizeHint() const;

    void scrollTo(const QModelIndex & index, ScrollHint hint = EnsureVisible);

    Event_model* currentEvent();
    void setCurrentEvent(Event_model* event);

signals:
    void currentEventChanged(Event_model*);
private slots:
    void eventListRowChanged(const QModelIndex& index);
private:
    QStandardItemModel* model_;
    QVector<Event_model*> events;
};

}
#endif

#include "event_list.h"

#include "trace_model.h"
#include "event_model.h"

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace vis4 {

EventList::EventList(QWidget* parent) :
    QTreeView(parent)
{
    /** some Qt decorations */
    setRootIsDecorated(false);
    setEditTriggers(NoEditTriggers);
    setHeaderHidden(true);
}

/** time is used to select first event with this time */
void EventList::showEvents(TraceModelPtr& traceModel, const Time& time)
{
    //? maybe better to clear model, not to delete it
    delete model();
    events.clear();

    QStandardItemModel* modelPtr = new QStandardItemModel(this);
    modelPtr->insertColumns(0, 2);
    QTreeView::setModel(modelPtr);
    connect(this->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),this,
                                    SLOT(eventListRowChanged()));

    int nearestEvent = -1;
    Time minDistance;//? min time range not initialized
    int row = 0;

    for(auto eventPtr = traceModel->getNextEvent(); eventPtr ;eventPtr = traceModel->getNextEvent(), ++row)
    {
        model()->insertRow(row);

        if (nearestEvent == -1 || distance(time, eventPtr->time) < minDistance)
        {
            nearestEvent = row;
            minDistance = distance(time, eventPtr->time);
        }

        QModelIndex index = model()->index(row, 0, QModelIndex());
        model()->setData(index, eventPtr->time.toString());
        index = model()->index(row, 1, QModelIndex());
        model()->setData(index, eventPtr->shortDescription());
        model()->setData(index, eventPtr->shortDescription(), Qt::ToolTipRole);
        events.push_back(eventPtr);
    }

    setAlternatingRowColors(true);

    resizeColumnToContents(0);
    resizeColumnToContents(1);

    if (nearestEvent != -1)
    {
        selectionModel()->setCurrentIndex(this->model()->index(nearestEvent, 0), QItemSelectionModel::Select);
        selectionModel()->select(this->model()->index(nearestEvent, 0), QItemSelectionModel::Select);
        selectionModel()->select(this->model()->index(nearestEvent, 1), QItemSelectionModel::Select);
    }
}

void EventList::updateTime()
{
    for(int i = 0; i < events.size(); ++i)
    {
        QModelIndex index = model()->index(i, 0, QModelIndex());
        model()->setData(index, events[i]->time.toString());
    }
    resizeColumnToContents(0);
}

EventModel* EventList::currentEvent()
{
    int row = currentIndex().row();
    if (row != -1)
    {
        Q_ASSERT(row < events.size());
        return events[row];
    }
    return nullptr;
}

void EventList::setCurrentEvent(EventModel* eventPtr)
{
    for(int i = 0; i < events.size(); ++i)
    {
        if ((*events[i]) == *eventPtr)
        {
            QModelIndex index = model()->index(i, 0, QModelIndex());
            setCurrentIndex(index);
            scrollTo(index);
            return;
        }
    }
    qFatal("Can't find given event in the list");
}

void EventList::eventListRowChanged()
{
    EventModel* eventPtr = currentEvent();
    if (!eventPtr)
    {
        return;
    }
    emit currentEventChanged(eventPtr);
}

void EventList::scrollTo(const QModelIndex & index, ScrollHint hint)
{
    /** Save state of horizontal bar while vertical scrolling */
    int hScrollBarState = horizontalScrollBar()->value();
    QTreeView::scrollTo(index, hint);
    horizontalScrollBar()->setValue(hScrollBarState);
}

QSize EventList::sizeHint() const
{
    return QSize(100, 192);
}

}

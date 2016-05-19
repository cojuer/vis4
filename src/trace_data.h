#ifndef TRACE_DATA_H
#define TRACE_DATA_H

#include <QString>
#include <QVector>

#include "time_vis.h"
#include "event_model.h"
#include "state_model.h"
#include "group_model.h"
#include "selection.h"

namespace vis4 {

class TraceData
{
public:
    TraceData();
    TraceData(Selection* componentsPtr, Selection* stateTypesPtr, Selection* eventTypesPtr, QVector<StateModel*>* states, QVector<EventModel*>* events, QVector<GroupModel*>* groups);
    ~TraceData();

    /** Returns number of lifeline adjusted to location number. */
    int getLifeline(int location) const;

    /** Returns location name. Full name contains all parents of the location. */
    QString getLocationName(int location, bool full = false) const;

    bool hasChildren(int location) const;

    Time getMinTime() const;
    Time getMaxTime() const;

    StateModel* getNextState();
    GroupModel* getNextGroup();
    EventModel* getNextEvent();

    const Selection getComponents() const;//?
    const Selection getEventTypes() const;
    const Selection getStateTypes() const;

private:
    Selection* componentsPtr;
    Selection* stateTypesPtr;
    Selection* eventTypesPtr;
    Time start, end;
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;
    QVector<GroupModel*>* groups;

    int currentState;
    int currentEvent;
    int currentGroup;
};

}

#endif // TRACE_DATA


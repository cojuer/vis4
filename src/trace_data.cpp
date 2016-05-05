#include "trace_data.h"

namespace vis4 {

TraceData::TraceData(Selection* componentsPtr, Selection* stateTypesPtr, Selection* eventTypesPtr, QVector<StateModel*>* states, QVector<EventModel*>* events) :
    componentsPtr(componentsPtr),
    stateTypesPtr(stateTypesPtr),
    eventTypesPtr(eventTypesPtr),
    states(states),
    events(events)
{}

TraceData::~TraceData()
{}

/** Returns number of lifeline adjusted to location number. */
int TraceData::getLifeline(int location) const
{
    return 0;
}

/** Returns location name. Full name contains all parents of the location. */
QString TraceData::getLocationName(int location, bool full) const
{
    return QString();
}

bool TraceData::hasChildren(int location) const
{
    return false;
}

Time TraceData::getMinTime() const
{
    return start;
}

Time TraceData::getMaxTime() const
{
    return end;
}

StateModel* TraceData::getNextState()
{
    if (currentState < states->size())
    {
        return (*states)[currentState++];
    }
    else
    {
        currentState = 0;
        return nullptr;
    }
}

GroupModel* TraceData::getNextGroup()
{
    if (currentGroup < groups->size())
    {
        return (*groups)[currentGroup++];
    }
    else
    {
        currentGroup = 0;
        return nullptr;
    }
}

EventModel* TraceData::getNextEvent()
{
    if (currentEvent < events->size())
    {
        return (*events)[currentEvent++];
    }
    else
    {
        currentEvent = 0;
        return nullptr;
    }
}

const Selection& TraceData::getComponents() const
{
    return *componentsPtr;
}

const Selection& TraceData::getEventTypes() const
{
    return *eventTypesPtr;
}

const Selection& TraceData::getStateTypes() const
{
    return *stateTypesPtr;
}

}

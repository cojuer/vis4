#include "trace_data.h"

namespace vis4 {

TraceData::TraceData() {}

TraceData::TraceData(Selection* componentsPtr, Selection* stateTypesPtr, Selection* eventTypesPtr, QVector<StateModel*>* states, QVector<EventModel*>* events, QVector<GroupModel*>* groups) :
    componentsPtr(componentsPtr),
    stateTypesPtr(stateTypesPtr),
    eventTypesPtr(eventTypesPtr),
    states(states),
    events(events),
    groups(groups),
    currentState(0),
    currentEvent(0),
    currentGroup(0)
{
    groups = new QVector<GroupModel*>();//should be arg
    start = (*events)[0]->time;
    end = (*events)[events->size() - 1]->time;

    std::cout << "TraceData constructor:" << std::endl;
    std::cout << start.toULL() << " : " << end.toULL() << std::endl;
    std::cout << events->size() << " events" << std::endl;
    std::cout << stateTypesPtr->size() << " state types" << std::endl;
    std::cout << componentsPtr->size() << " components" << std::endl;
}

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

const Selection TraceData::getComponents() const
{
    return *componentsPtr;
}

const Selection TraceData::getEventTypes() const
{
    return *eventTypesPtr;
}

const Selection TraceData::getStateTypes() const
{
    return *stateTypesPtr;
}

}

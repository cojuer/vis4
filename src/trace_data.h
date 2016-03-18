#ifndef TRACE_DATA_H
#define TRACE_DATA_H

class TraceData
{
public:
private:
    Selection* locationsPtr;
    Selection* statesPtr;
    Time start, end;
    QVector<StateModel*> states;
    QVector<EventModel*> events;
};

#endif // TRACE_DATA


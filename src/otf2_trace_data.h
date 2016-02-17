#ifndef OTF2_TRACE_DATA_H
#define OTF2_TRACE_DATA_H

#include <otf2/otf2.h>

#include "selection.h"
#include "state_model.h"
#include "event_model.h"

namespace vis4
{

class OTF2_TraceData
{
public:
    OTF2_TraceData();
private:
    Selection* locationsPtr;
    Selection* statesPtr;//?
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;

};

}

#endif // OTF2_TRACE_DATA_H

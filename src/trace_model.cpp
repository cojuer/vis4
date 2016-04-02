#include "trace_model.h"

namespace vis4 {

int delta(const TraceModel& a, const TraceModel& b)
{
    int result = 0;

    if (a.getParentComponent() != b.getParentComponent())
        result |= Trace_model_delta::component_position;
    if (a.getComponents() != b.getComponents())
        result |= Trace_model_delta::components;
    if (a.getEvents() != b.getEvents())
        result |= Trace_model_delta::event_types;
    if (a.groupsEnabled() != b.groupsEnabled())
        result |= Trace_model_delta::event_types;
    if (a.getStates() != b.getStates())
        result |= Trace_model_delta::state_types;
    if (a.getAvailableStates() != b.getAvailableStates())
        result |= Trace_model_delta::state_types;
    if (a.getMinTime() != b.getMinTime() || a.getMaxTime() != b.getMaxTime())
        result |= Trace_model_delta::time_range;
    return result;
}

}

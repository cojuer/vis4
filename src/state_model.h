#ifndef STATE_MODEL_H
#define STATE_MODEL_H

#include <QString>
#include <QColor>

#include "time_vis.h"

namespace vis4 {

/**
 * State description for visualization purposes.
 */
class StateModel
{
public:
    Time start;
    Time end;

    /** State id. Link to state in Trace_model::states(). */
    int type;

    unsigned component;
    QColor color;

    StateModel() {}

    StateModel(unsigned int component, int type, Time begin, Time end, QColor color) :
        component(component),
        type(type),
        start(begin),
        end(end),
        color(color)
    {}

    virtual ~StateModel() {}
};

}
#endif

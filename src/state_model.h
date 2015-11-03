#ifndef STATE_MODEL_H
#define STATE_MODEL_H

#include <QString>
#include <QColor>

#include "time_vis.h"

namespace vis4 {

/**
 * State description for visualization purposes.
 */
class State_model
{
public:
    Time begin;
    Time end;

    /** State id. Link to state in Trace_model::states(). */
    int type;

    /** Номер компонента, к которому относится состояние. */
    unsigned component;

    /** The color to be used when drawing it.  */
    QColor color;

    virtual ~State_model() {}
};

}
#endif

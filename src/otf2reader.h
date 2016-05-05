#ifndef OTF2READER_H
#define OTF2READER_H

#include <otf2/otf2.h>

#include "trace_reader.h"
#include "trace_model.h"

namespace vis4 {

typedef struct {
    Selection* components;
    Selection* stateTypes;
    Selection* eventTypes;
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;
} OTF2_NewHandlerArgument;

class OTF2Reader : TraceReader
{
public:
    TraceData* read(QString tracePath) override;
};

}

#endif // OTF2READER_H

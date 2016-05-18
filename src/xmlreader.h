#ifndef XMLREADER_H
#define XMLREADER_H

#include "trace_reader.h"

namespace vis4 {

class XMLReader : public TraceReader
{
public:
    TraceData* read(QString tracePath) override;
};

typedef struct {
    Selection* components;
    Selection* stateTypes;
    Selection* eventTypes;
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;
} XMLHandlerArgument;

}

#endif // XMLREADER_H

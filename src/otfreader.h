#ifndef OTFREADER_H
#define OTFREADER_H

#include <QString>
#include <QTextCodec>

#include <otf.h>

#include "trace_reader.h"

namespace vis4 {

typedef struct {
    Selection* components;
    Selection* stateTypes;
    Selection* eventTypes;
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;
    QVector<GroupModel*>* groups;
} NewHandlerArgument;

class OTFReader : public TraceReader
{
public:
    TraceData* read(QString tracePath) override;
};

}

#endif // OTFREADER_H

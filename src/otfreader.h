#ifndef OTFREADER_H
#define OTFREADER_H

#include <QString>
#include <QTextCodec>

#include <otf.h>

#include "trace_reader.h"
#include "trace_model.h"

namespace vis4 {

typedef struct {
    Selection* components;
    Selection* stateTypes;
    Selection* eventTypes;
    QVector<StateModel*>* states;
    QVector<EventModel*>* events;
} NewHandlerArgument;

class OTFReader : TraceReader
{
public:
    TraceData* read(QString tracePath) override;
};

static int handleDefProcess (void* userData, uint32_t stream, uint32_t process, const char *name, uint32_t parent)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    int current_link = arg->components->addItem(QString(name), static_cast<int>(parent));
    qDebug() << "stream = " << stream << " process: " << process << " name: " << name << " parent: " << parent;
    return OTF_RETURN_OK;
}

static int handleDefFunction (void* userData, uint32_t stream, uint32_t func, const char *name, uint32_t funcGroup, uint32_t source)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    QTextCodec* codec = QTextCodec::codecForName( "KOI8-R" );
    QString tr_name = codec->toUnicode(name);
    QTextCodec* codec1 = QTextCodec::codecForName( "UTF-8" );
    tr_name = codec1->fromUnicode(tr_name);
    arg->stateTypes->addItem(tr_name, -1);
    qDebug() << "stream = " << stream << " func: " << func << " name: " << tr_name << " funcGroup: " << funcGroup << " source: " << source;
    return OTF_RETURN_OK;
}

/** Обработчики событий и состояний */
static int handleEnter (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    StateModel* sm = new StateModel(process, function, Time(time), Time(0), Qt::yellow);
    arg->states->push_back(sm);

    EventModel* em = new EventModel(Time(time), process, "ENTER", 'E');
    arg->events->push_back(em);

    qDebug() << "ENTER: " << function << "time: " << time << " type: " << function;
    return OTF_RETURN_OK;
}

static int handleLeave (void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
{
    auto arg = static_cast<NewHandlerArgument*>(userData);
    for (int i = arg->states->size() - 1; i > 0; --i)
    {
        //TEST
        if (process == (*arg->states)[i]->component)
        {
            (*arg->states)[i]->end = Time(time);
        }
    }

    EventModel* em = new EventModel(Time(time), process, "LEAVE", 'L');
    arg->events->push_back(em);

    qDebug() << "LEAVE: " << function << "time: " << time << " type: " << function;
    return OTF_RETURN_OK;
}

}

#endif // OTFREADER_H

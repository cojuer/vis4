#include "xmlreader.h"

#include <QFile>
#include <QXmlStreamReader>

namespace vis4 {

TraceData* XMLReader::read(QString tracePath)
{
    Selection* componentsPtr = new Selection();
    Selection* stateTypesPtr = new Selection();
    Selection* eventTypesPtr = new Selection();

    QVector<EventModel*>* eventsPtr = new QVector<EventModel*>;
    QVector<StateModel*>* statesPtr = new QVector<StateModel*>;

    XMLHandlerArgument ha = {componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr};

    QFile* filePtr = new QFile(tracePath);
    filePtr->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(filePtr);

    stateTypesPtr->addItem("state", -1);
    eventTypesPtr->addItem("event", -1);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "trace")
            {
                continue;
            }
            if (xml.name() == "component")
            {
                componentsPtr->addItem(xml.attributes().value("", "name").toString(), -1);
            }
            if (xml.name() == "events")
            {
                continue;
            }
        }
        if (xml.name() == "event")
        {
            int time = xml.attributes().value("time").toInt();
            QString kind = xml.attributes().value("kind").toString();

            if (!kind.compare("Send"))
            {
                eventsPtr->push_back(new EventModel(Time(time), 0, kind, 'S'));
                statesPtr->push_back(new StateModel(0, 0, Time(time), Time(0), Qt::yellow));
            }
            else if (!kind.compare("Receive"))
            {
                eventsPtr->push_back(new EventModel(Time(time), 0, kind, 'R'));
                (*statesPtr)[statesPtr->size() - 1]->end = Time(time);
            }

        }
    }

    return new TraceData(componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr);
}

}


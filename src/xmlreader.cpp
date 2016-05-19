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
    QVector<GroupModel*>* groupsPtr = new QVector<GroupModel*>();

    XMLHandlerArgument ha = {componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr};

    QFile* filePtr = new QFile(tracePath);
    filePtr->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(filePtr);

    stateTypesPtr->addItem("state", -1);
    eventTypesPtr->addItem("event", -1);
    int comp = -1;
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
                ++comp;
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
            char letter = xml.attributes().value("letter").toString().data()->toLatin1();

            if (letter == 'E')
            {
                eventsPtr->push_back(new EventModel(Time(time), comp, kind, letter));
                statesPtr->push_back(new StateModel(comp, 0, Time(time), Time(0), Qt::yellow));
            }
            else if (letter == 'L')
            {
                eventsPtr->push_back(new EventModel(Time(time), comp, kind, letter));
                (*statesPtr)[statesPtr->size() - 1]->end = Time(time);
            }
        }
        else if (xml.name() == "group")
        {
            GroupModel* gm = new GroupModel();
            Time stm = Time(xml.attributes().value("time").toInt());
            Time rtm = Time(xml.attributes().value("target_time").toInt());
            int sender = comp;
            int receiver = xml.attributes().value("target_component").toInt();
            GroupModel::Point from;
            from.component = sender;
            from.time = stm;
            GroupModel::Point to;
            to.component = receiver;
            to.time = rtm;
            gm->points.push_back(from);
            gm->points.push_back(to);
            gm->type = GroupModel::arrow;
            groupsPtr->push_back(gm);
        }
    }

    return new TraceData(componentsPtr, stateTypesPtr, eventTypesPtr, statesPtr, eventsPtr, groupsPtr);
}

}


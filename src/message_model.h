#ifndef MESSAGE_MODEL_H
#define MESSAGE_MODEL_H

#include "time_vis.h"

namespace vis4 {

class MessageModel
{
public:
    struct Point
    {
        int location;
        Time time;
    };

    Point from;
    QVector<Point> to;
};

}

#endif // MESSAGE_MODEL_H

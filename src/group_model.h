#ifndef GROUP_MODEL_H
#define GROUP_MODEL_H

#include <vector>
#include <QVector>

#include "time_vis.h"

namespace vis4 {

/**
 * GroupModel is a collection of timeline points that must be
 * specially decorated as a group.
 */
class GroupModel
{
public:
    static const int arrow = 1;

    struct Point
    {
        int component;
        Time time;
    };

    int type;//? what is this time?
    QVector<Point> points;
    unsigned int id;
};

}
#endif

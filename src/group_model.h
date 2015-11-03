#ifndef GROUP_MODEL_H
#define GROUP_MODEL_H

#include <vector>

#include "time_vis.h"

namespace vis4 {

/**
 * GroupModel is a collection of timeline points that must be
 * specially decorated as a group.
 */
class Group_model
{
public:
    static const int arrow = 1;

    struct point
    {
        int component;
        Time time;
    };

    int type;//? what is this time?
    std::vector<point> points;
};

}
#endif

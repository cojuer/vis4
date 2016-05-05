#ifndef TRACEREADER_H
#define TRACEREADER_H

#include "trace_data.h"

namespace vis4 {

class TraceReader
{
public:
    virtual TraceData* read(QString tracePath) = 0;
};

}

#endif // TRACEREADER_H

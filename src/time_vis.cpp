#include "time_vis.h"
#include <QtWidgets/QApplication>

namespace vis4 {

//? global vars? srsly?
QStringList Time::units_;
Time::Format Time::format_;
QList<long long> Time::scales_;

int Time::unit = -1;

namespace
{
    /** translation */
    QString tr(const char* s)
    {
        QString result = qApp->translate("vis4::Time", s);
        return result;
    }
}

Time::Time()
{
    if (unit != -1) return;

    // If we create time object  for the first time,
    // we have to initialize units.

    units_ << tr("us"); scales_ << 1;
    units_ << tr("ms"); scales_ << 1000;
    units_ << tr("s"); scales_ << 1000000;
    units_ << tr("m"); scales_ << 60ll*1000000;
    units_ << tr("h"); scales_ << 60ll*60ll*1000000;

    unit = 0; format_ = Plain;
}

} // namespaces

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

unsigned long long getUs(const Time & t)//? ...
{
    try
    {
        unsigned long long us = boost::any_cast<unsigned long long>(t.data());
        return us;
    }
    catch (boost::bad_any_cast &)
    {
        try
        {
            long long us = boost::any_cast<long long>(t.data());
            return (unsigned long long)us;
        }
        catch (boost::bad_any_cast &)
        {
            try
            {
               unsigned long us = boost::any_cast<unsigned long>(t.data());
               return (unsigned long long)us;
            }
            catch (boost::bad_any_cast &)
            {
                try
                {
                    long us = boost::any_cast<long>(t.data());
                    return (unsigned long long)us;
                }
                catch (boost::bad_any_cast &)
                {
                    try
                    {
                        unsigned int us = boost::any_cast<unsigned int>(t.data());
                        return (unsigned long long)us;
                    }
                    catch (boost::bad_any_cast &)
                    {
                        try
                        {
                            int us = boost::any_cast<int>(t.data());
                            return (unsigned long long)us;
                        }
                        catch (boost::bad_any_cast &)
                        {
                            qWarning("Warning! Unknown time implementation.");
                            return (unsigned long long)-1;
                        }
                    }
                }
            }
        }
    }
}

} // namespaces

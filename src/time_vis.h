#ifndef TIME_H
#define TIME_H

#include <cmath>
#include <typeinfo>
#include <cassert>

#include <QStringList>
#include <QDebug>

#include <boost/shared_ptr.hpp>
#include <boost/operators.hpp>
#include <boost/any.hpp>

#include <time.h>

namespace vis4 {

#define MAX_NSEC 1000000000

/**
 * Class-wrapper for struct timespec.
 */
class Time : public boost::less_than_comparable<Time>
{
public: /* static methods */

    enum UnitType { us, ms, sec, min, hour, ns };
    enum Format { Plain, Advanced };

    static const QStringList& units()  { return units_; }

    static int getUnit()  { return unit; }
    static void setUnit(int new_unit) { unit = new_unit; }

    static Format format() { return format_; }
    static void setFormat(Format format) { format_ = format; }

    static QString unit_name(int new_unit = -1)
    {
        if (new_unit == -1)
        {
            new_unit = unit;
        }
        return units_[new_unit];
    }

    static long long unit_scale(int new_unit = -1)
    {
        if (new_unit == -1)
        {
            new_unit = unit;
        }
        return scales_[new_unit];
    }

public: /* static members */

    static QStringList units_;
    static QList<long long> scales_;

    static int unit;
    static Format format_;

public:
    Time();

    Time(const timespec time) : data(time) {}
    Time(const Time& another) : data(another.data) {}
    Time(unsigned long long time)
    {
        struct timespec new_data;
        new_data.tv_sec = time / MAX_NSEC;
        new_data.tv_nsec = time - new_data.tv_sec * MAX_NSEC;
        data = new_data;
    }

    Time& operator=(const Time& another)
    {
        data = another.data;
        return *this;
    }

    Time operator+(const Time& another) const
    {
        auto sec = data.tv_sec + another.data.tv_sec;
        auto nsec = data.tv_nsec + another.data.tv_nsec;
        if (nsec >= MAX_NSEC)
        {
            nsec -= MAX_NSEC;
            ++sec;
        }
        timespec new_data = {.tv_sec = sec, .tv_nsec = nsec};
        return Time(new_data);
    }

    Time operator-(const Time& another) const
    {
        auto sec = (data.tv_sec >= another.data.tv_sec) ? data.tv_sec - another.data.tv_sec : 0;
        auto nsec = data.tv_nsec;
        if (nsec >= another.data.tv_nsec)
        {
            nsec -= another.data.tv_nsec;
        }
        else if (sec > 0)
        {
            --sec;
            nsec += MAX_NSEC - another.data.tv_nsec;
        }
        else
        {
            nsec = 0;
        }
        timespec new_data = {.tv_sec = sec, .tv_nsec = nsec};
        return Time(new_data);
    }

    Time operator*(double arg) const
    {
        //TO UPDATE
        auto sec = data.tv_sec;
        sec *= arg;
        auto nsec = data.tv_nsec;
        nsec *= arg;
        if (nsec >= MAX_NSEC)
        {
            sec += static_cast<int>(nsec / MAX_NSEC);
            nsec -= static_cast<int>(nsec / MAX_NSEC) * MAX_NSEC;
        }
        timespec new_data = {.tv_sec = sec, .tv_nsec = nsec};
        return Time(new_data);
    }

    Time operator/(double arg) const
    {
        //TO UPDATE
        auto sec = data.tv_sec;
        sec /= arg;
        auto nsec = data.tv_nsec;
        nsec /= arg;
        timespec new_data = {.tv_sec = sec, .tv_nsec = nsec};
        return Time(new_data);
    }

    double operator/(const Time& another) const
    {
        return 0;
        //? why we need this?
        //return object->div(another.object.get());
    }

    bool operator<(const Time& another) const
    {
        if (data.tv_sec < another.data.tv_sec)
        {
            return true;
        }
        else if (data.tv_sec == another.data.tv_sec)
        {
            return (data.tv_nsec < another.data.tv_nsec);
        }
    }

    bool operator==(const Time& another) const
    {
        return (data.tv_sec == another.data.tv_sec && data.tv_nsec == another.data.tv_nsec);
    }

    bool operator!=(const Time& another) const
    {
        return !(data.tv_sec == another.data.tv_sec && data.tv_nsec == another.data.tv_nsec);
    }

    bool sameType(const Time& another) const
    {
        return true;//TO DELETE
    }

    Time fromString(const QString& timeString) const
    {
        //TO UPDATE
        unsigned long long data = timeString.toULongLong();
        timespec time = {.tv_sec = data / MAX_NSEC, .tv_nsec = data - static_cast<int>(data / MAX_NSEC) * MAX_NSEC};
        return Time(time);//TO UPDATE
    }

    QString toString(bool also_unit = false) const
    {
        QString timeString = QString::number(data.tv_sec * MAX_NSEC + data.tv_nsec);
        return timeString;
    }

    timespec getData() const
    {
        return data;
    }

    Time setData(timespec data) const
    {
        //?
        return Time(data);
    }

    static Time scale(const Time& point1, const Time& point2, double pos)
    {
        return point1 + (point2 - point1) * pos;
    }

    unsigned long long toULL() const
    {
        return data.tv_sec * MAX_NSEC + data.tv_nsec;
    }

private:
    timespec data;
};

inline Time distance(const Time& t1, const Time& t2)
{
    return (t1 > t2) ? t1 - t2 : t2 - t1;
}

} // namespaces

#endif

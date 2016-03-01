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

namespace vis4 {

/**
 * Interface for different time representations.
 */
class TimeInterface
{
public:
    virtual TimeInterface* fromString(const QString & time) const = 0;
    virtual QString toString() const = 0;
    virtual TimeInterface* add(const TimeInterface* another) = 0;
    virtual TimeInterface* sub(const TimeInterface* another) = 0;
    virtual TimeInterface* mul(double a) = 0;

    /*
    virtual TimeInterface* operator+(const TimeInterface* another) = 0;
    virtual TimeInterface* operator-(const TimeInterface* another) = 0;
    virtual TimeInterface* operator*(const double a) = 0;
    virtual double operator/(const TimeInterface* another) = 0;
    */

    virtual double div(const TimeInterface* another) = 0;
    virtual bool less_than(const TimeInterface* another) = 0;
    virtual boost::any data() const = 0;
    virtual TimeInterface* setData(const boost::any& any) const = 0;
    virtual ~TimeInterface() {}
};

/**
 * Class-wrapper for various time representation.
 */
class Time : public boost::less_than_comparable<Time>
{
public: /* static methods */

    enum UnitType { us, ms, sec, min, hour };
    enum Format { Plain, Advanced };

    static const QStringList & units()  { return units_; }

    static int getUnit()  { return unit; }
    static void setUnit(int new_unit) { unit = new_unit; }

    static Format format() { return format_; }
    static void setFormat(Format format) { format_ = format; }

    static QString unit_name(int new_unit = -1)
    {
        if (new_unit == -1) new_unit = unit;
        return units_[new_unit];
    }

    static long long unit_scale(int new_unit = -1)
    {
        if (new_unit == -1) new_unit = unit;
        return scales_[new_unit];
    }

public: /* static members */

    static QStringList units_;
    static QList<long long> scales_;

    static int unit;
    static Format format_;

public:
    // Created uninitialized time. Using such instance in any way
    // is a programmer's error.
    // NOTE: most usages will assert thanks to shared_ptr.
    Time();

    Time(const boost::shared_ptr<TimeInterface>& imp) : object(imp) {}

    Time(const Time& another) : object(another.object) {}

    Time& operator=(const Time& another)
    {
        object = another.object;
        return *this;
    }

    bool isNull() const
    {
        return object.get() == nullptr;
    }

    Time operator+(const Time& another) const
    {
        assert(typeid(*object.get()) == typeid(*another.object.get()));
        boost::shared_ptr<TimeInterface> n(
            object->add(another.object.get()));
        return Time(n);
    }

    Time operator-(const Time& another) const
    {
        assert(typeid(*object.get()) == typeid(*another.object.get()));
        boost::shared_ptr<TimeInterface> n(
            object->sub(another.object.get()));
        return Time(n);
    }

    Time operator*(double a) const
    {
        boost::shared_ptr<TimeInterface> n(
            object->mul(a));
        return Time(n);
    }

    Time operator/(double a) const
    {
        boost::shared_ptr<TimeInterface> n(object->mul(1 / a));
        return Time(n);
    }

    double operator/(const Time& another) const
    {
        assert(typeid(*object.get()) == typeid(*another.object.get()));
        return object->div(another.object.get());
    }

    bool operator<(const Time& another) const
    {
        assert(object.get());
        assert(another.object.get());
        assert(typeid(*object.get()) == typeid(*another.object.get()));
        return object->less_than(another.object.get());
    }

    bool operator==(const Time& another) const
    {
        if (object.get() == 0) return another.isNull();
        if (another.isNull()) return false;

        //? stupid code
        /* We might add another method to Time_implementation to
           compare this directly, but time equality comparison
           is not done often, so we don't care about performance.  */
        return !(*this < another) && !(another < *this);
    }

    bool operator!=(const Time& another) const
    {
        return !(*this == another);
    }

    bool sameType(const Time& another) const
    {
        return typeid(*object.get()) == typeid(*another.object.get());
    }

    Time fromString(const QString & time) const
    {
        Q_ASSERT(object.get() != 0);
        boost::shared_ptr<TimeInterface> p(object->fromString(time));
        return Time(p);
    }

    QString toString(bool also_unit = false) const
    {
        QString time_str = object->toString();
        if (also_unit && format() == Plain)
            time_str += " " + unit_name(getUnit());

        return time_str;
    }

    boost::any data() const
    {
        return object->data();
    }

    Time setData(const boost::any& any) const
    {
        boost::shared_ptr<TimeInterface> p(object->setData(any));
        return Time(p);
    }

    static Time scale(const Time& point1, const Time& point2, double pos)
    {
        return point1 + (point2 - point1) * pos;
    }

private:
    boost::shared_ptr<TimeInterface> object;
};

/**
 * Time representation with one template typed member.
 */
template<class T>
class ScalarTime : public TimeInterface
{
public:
    ScalarTime(T time) :
        time(time)
    {}

    virtual TimeInterface* add(const TimeInterface *xanother)
    {
        const ScalarTime *another = dynamic_cast<const ScalarTime*>(xanother);
        return construct(time + another->time);
    }

    virtual TimeInterface* sub(const TimeInterface* xanother)
    {
        const ScalarTime *another = dynamic_cast<const ScalarTime*>(xanother);
        return construct(time - another->time);
    }

    virtual TimeInterface* mul(double a)
    {
        return construct(T(floor(time * a + 0.5)));//? ???
    }

    virtual double div(const TimeInterface* xanother)
    {
        const ScalarTime *another = dynamic_cast<const ScalarTime*>(xanother);
        return static_cast<double>(time)/another->time;
    }

    virtual bool less_than(const TimeInterface* xanother)
    {
        const ScalarTime* another = dynamic_cast<const ScalarTime*>(xanother);
        return time < another->time;
    }

    boost::any data() const//? why not T? Time_implementation doesn't depends on T, that's why.
    {
        return time;
    }

    ScalarTime* setData(const boost::any& any) const
    {
        return construct(boost::any_cast<T>(any));
    }

    virtual TimeInterface* fromString(const QString & time) const//? useless
    {
        return new ScalarTime(*this);
    }

    virtual QString toString() const
    {
        return QString::number(time/Time::unit_scale());
    }

    virtual ScalarTime* construct(T time) const
    {
        return new ScalarTime(time);
    }
protected:
    T time;
};

template<class T>
Time scalarTime(T t)
{
    boost::shared_ptr<TimeInterface> p(new ScalarTime<T>(t));
    return Time(p);
}

inline Time distance(const Time& t1, const Time& t2)
{
    return (t1 > t2) ? t1 - t2 : t2 - t1;
}

// возвращает время в микросекундах или -1, если не ясно на основе какого типа построено время
unsigned long long getUs(const Time & t);

} // namespaces

#endif

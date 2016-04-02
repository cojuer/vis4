#include "timeedit.h"
#include <boost/any.hpp>

namespace vis4 {

TimeEdit::TimeEdit(QWidget* parent) :
    QAbstractSpinBox(parent),
    long_long_time(false),
    unsigned_long_long_time(false),
    mNoError(true),
    mEdited(false)
{
    connect(lineEdit(), SIGNAL(textEdited(const QString &)), this,
                        SLOT(valueChanged(const QString &)));

    connect(lineEdit(), SIGNAL(editingFinished()), this,
                        SLOT(onEditingFinished()));

    mValidator = 0;
    mValidatorRegExp.setPattern("(\\d{1,2}:){1,2}(\\d{1,2}\\.)?\\d+");
    onTimeSettingsChanged();

    mErrorPalette.setColor(QPalette::Base, Qt::red);
    mNormalPalette = palette();

    // пока не задано время, редактирование запрещено
    setEnabled(false);
}

void TimeEdit::stepBy(int steps)
{
    Time t = cur_time_.fromString(lineEdit()->text());
    int u = Time::getUnit();

    long long newUs = t.toULL();
    newUs += Time::unit_scale(u)*steps;
    if (newUs < 0)
    {
        // проверим, чтобы не уйти в минус
        return;
    }

    //? TO UPDATE
    if (long_long_time)
    {
        t = Time(newUs);
    }
    else if (unsigned_long_long_time)
    {
        t = Time((unsigned long long)newUs);
    }
    else
    {
        t = Time((int)newUs);
    }

    if (!validate(t))
    {
        return;
    }

    setTime(t);

    emit editingFinished();
    emit timeChanged(t);
}

Time TimeEdit::time() const
{
    return cur_time_;
}

void TimeEdit::setTime(const Time& time)
{
    setEnabled(false);
    cur_time_ = time;
    lineEdit()->setText(time.toString());
    if (!mNoError)
    {
        setPalette(mNormalPalette);
        mNoError = true;
    }
    mEdited = false;

    setEnabled(true);
}

void TimeEdit::valueChanged(const QString & value)
{
    if (!(Time::format() != Time::Advanced || mValidatorRegExp.exactMatch(value)) ||
	!validate(cur_time_.fromString(value)))
    {
        setPalette(mErrorPalette);
        mNoError = false;
    }
    else
    {
        setPalette(mNormalPalette);
        mNoError = true;
        cur_time_ = cur_time_.fromString(value);
    }
    mEdited = true;

    emit textChanged();
}

bool TimeEdit::validate(const Time & t) const
{
    if (min_time_ > t) return false;
    if (max_time_ < t) return false;

    return true;
}

void TimeEdit::onEditingFinished()
{
    if (!mEdited)
    {
        return;
    }
    mEdited = false;

    Time t = cur_time_.fromString(lineEdit()->text());
    t = Time(lineEdit()->text().toInt());
    if (!validate(t) || !mNoError)
    {
	// возвращаемся в корректное состояние
        setTime(cur_time_);
        return;
    }
    cur_time_ = t;
    emit timeChanged(t);
}

void TimeEdit::onTimeSettingsChanged()
{
	if (mValidator) delete mValidator;
	mValidator = 0;
	switch (Time::format())
	{
		case Time::Plain:
		{
			unsigned long long maximum = 24 * Time::unit_scale(Time::hour);
			maximum /= Time::unit_scale();
			mValidator = new QIntValidator(0, maximum, this);
			break;
		}
		case Time::Advanced:
			mValidator = new QRegExpValidator(mValidatorRegExp, this);
			break;
		default:;
	}
	lineEdit()->setValidator(mValidator);
}

QAbstractSpinBox::StepEnabled TimeEdit::stepEnabled () const
{
    if (cur_time_.toULL() < (unsigned long long)Time::unit_scale())
    {
        return StepUpEnabled;
    }
    return StepUpEnabled | StepDownEnabled;
}

} // namespaces

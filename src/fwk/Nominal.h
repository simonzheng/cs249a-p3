/**
 * Nominal and Ordinal template types for encapsulating simple values.
 */

#ifndef FWK_NOMINAL_H
#define FWK_NOMINAL_H

/**
 * Nominal only defines equality, inequality, assignment, and conversion
 * to and from the representation type.
 */
template <class UnitType, class RepType>
class Nominal {
public:

    Nominal(RepType v) :
        value_(v)
    {
        // Nothing else to do.
    }

    Nominal(const Nominal& n) :
        value_(n.value_)
    {
        // Nothing else to do.
    }


    bool operator ==(const Nominal& v) const {
        return value_ == v.value_;
    }
	
    bool operator !=(const Nominal& v) const {
        return value_ != v.value_;
    }


    void operator =(const Nominal& v) {
        value_ = v.value_;
    }


    operator RepType() const {
        return value_;
    }

    RepType value() const {
        return value_;
    }

protected:

    RepType value_;

};

/**
 * Ordinal adds ordering comparisons, addition/subtraction,
 * multiplication/division, and increment/decrement operators.
 */
template <class UnitType, class RepType>
class Ordinal : public Nominal<UnitType, RepType> {
public:

    Ordinal(RepType v) :
        Nominal<UnitType, RepType>(v)
    {
        // Nothing else to do.
    }


    bool operator <(const Ordinal& v) const {
        return Nominal<UnitType, RepType>::value_ < v.value_;
    }

    bool operator <=(const Ordinal& v) const {
        return Nominal<UnitType, RepType>::value_ <= v.value_;
    }

    bool operator >(const Ordinal& v) const {
        return Nominal<UnitType, RepType>::value_ > v.value_;
    }

    bool operator >=(const Ordinal& v) const {
        return this->value_ >= v.value_;
    }


    Ordinal operator +(const Ordinal& other) const {
        return this->value_ + other.value_;
    }

    Ordinal operator -(const Ordinal& other) const {
        return this->value_ - other.value_;
    }

    Ordinal operator /(const Ordinal& other) const {
        return this->value_ / other.value_;
    }

    Ordinal operator *(const Ordinal& other) const {
        return this->value_ * other.value_;
    }


    const Ordinal& operator ++() {
        ++(this->value_);
        return *this;
    }

    const Ordinal& operator --() {
        --(this->value_);
        return *this;
    }

};

#endif

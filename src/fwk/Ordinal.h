/**
 * Ordinal template type for encapsulating simple values.
 */

#ifndef FWK_ORDINAL_H
#define FWK_ORDINAL_H

/**
 * Ordinal adds ordering comparisons, addition/subtraction,
 * multiplication/division, and increment/decrement operators.
 */
template <class UnitType, class RepType>
class Ordinal : public Nominal<UnitType, RepType> {
public:

    Ordinal(const RepType v) :
        Nominal<UnitType, RepType>(v)
    {
        // Nothing else to do.
    }

    Ordinal(const Ordinal& ordinal) :
        Nominal<UnitType, RepType>(ordinal.value_)
    {
        // Nothing else to do.
    }


    void operator =(const Ordinal& ordinal) {
        this->value_ = ordinal.value_;
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

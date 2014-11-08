// Copyright (C) 1993-2002 David R. Cheriton.  All rights reserved.
//
// Edited by Mark Linton for CS 249A Fall 2014.
//

#ifndef FWK_PTR_H
#define FWK_PTR_H


static void checkNull(void* const ptr);

static void checkNull(const void* const ptr);


template <class T>
class Ptr {
public:

    Ptr(T* const ptr = null) :
        ptr_(ptr)
    {
        newRef(ptr);
    }

    Ptr(const Ptr& p) :
        ptr_(p.ptr_)
    {
        newRef(ptr_);
    }

    ~Ptr() {
        deleteRef(ptr_);
    }


    _noinline
    void operator =(const Ptr& p) {
        T* const ptr = p.ptr_;
        newRef(ptr);
        deleteRef(ptr_);
        ptr_ = ptr;
    }

    _noinline
    void operator =(T* const ptr) {
        newRef(ptr);
        deleteRef(ptr_);
        ptr_ = ptr;
    }

    template <class OtherType>
    _noinline
    void operator =(const Ptr<OtherType> p) {
        T* const ptr = p.ptr();
        newRef(ptr);
        deleteRef(ptr_);
        ptr_ = ptr;
    }


    bool operator ==(const Ptr& p) const {
        return ptr_ == p.ptr_;
    }

    bool operator !=(const Ptr& p) const {
        return ptr_ != p.ptr_;
    }

    bool operator ==(T* const ptr) const {
        return ptr_ == ptr;
    }

    bool operator !=(T* const ptr) const {
        return ptr_ != ptr;
    }

    template <class OtherType>
    bool operator ==(const Ptr<OtherType&> p) const {
        return ptr_ == p.ptr();
    }

    template <class OtherType>
    bool operator !=(const Ptr<OtherType&> p) const {
        return ptr_ != p.ptr();
    }


    T* operator ->() const {
        checkNull(ptr_);
        return ptr_;
    }

    T* operator ->() {
        checkNull(ptr_);
        return ptr_;
    }

    T* ptr() const {
        return ptr_;
    }

    T* checkedPtr() const {
        checkNull(ptr_);
        return ptr_;
    }


    operator bool() const {
        return ptr_ != null;
    }

    template <class OtherType>
    operator Ptr<OtherType>() const {
        return Ptr<OtherType>(ptr_);
    }

    template <class OtherType>
    Ptr<OtherType> narrow() const {
        return dynamic_cast<OtherType*>(ptr_);
    }

protected:

    T* ptr_;

private:

    _noinline
    static void newRef(T* const ptr) {
        if (ptr != null) {
            ptr->newRef();
        }
    }

    _noinline
    static void deleteRef(T* const ptr) {
        if (ptr != null) {
            ptr->deleteRef();
        }
    }

};

#endif /* FWK_PTR_H */

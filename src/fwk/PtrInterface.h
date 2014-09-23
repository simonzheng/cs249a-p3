// Copyright (c) 1993-2007 David R. Cheriton, all rights reserved.
// PtrInterface.h
//
// Edited by Mark Linton for CS 249A Fall 2014.
//

#ifndef FWK_PTRINTERFACE_H
#define FWK_PTRINTERFACE_H

class PtrInterface {
public:

    PtrInterface() :
        ref_(0)
    {
        // Nothing else to do.
    }

    unsigned long references() const {
        return ref_;
    }

    enum Attribute {
        nextAttributeNumber__ = 1
    };

    // DRC - support for templates
    void newRef() const {
        PtrInterface* const ptr = const_cast<PtrInterface*>(this);
        ptr->ref_ += 1;
    }

    void deleteRef() const {
        PtrInterface* const ptr = const_cast<PtrInterface*>(this);
        ptr->ref_ -= 1;
        if (ptr->ref_ == 0) {
            ptr->onZeroReferences();
        }
    }

protected:

    virtual ~PtrInterface() {
        // Nothing to do.
    }

    virtual void onZeroReferences() {
        delete this;
    }

private:

    long unsigned ref_;

};

#endif

//
// Library for notifiers.
//
// Edited by Mark Linton for CS 249A Fall 2014.
//

#ifndef FWK_NOTIFIERLIB_H
#define FWK_NOTIFIERLIB_H

namespace NotifierLib {

    template <class T>
    _noinline
    void post(T* const notifier, void (T::Notifiee::*func)()) {
        for (const auto n : notifier->notifiees()) {
            try {
                (n->*func)();
            } catch (...) {
                n->onNotificationException();
            }
        }
    }

    template <class T>
    _noinline
    void post(const Ptr<T>& notifier, void (T::Notifiee::*func)()) {
        for (const auto n : notifier->notifiees()) {
            try {
                (n->*func)();
            } catch (...) {
                n->onNotificationException();
            }
        }
    }


    template <class T, typename P1>
    _noinline
    void post(
        T* const notifier, void (T::Notifiee::*func)(const P1),
        const P1 a
    ) {
        for (const auto n : notifier->notifiees()) {
            try {
                (n->*func)(a);
            } catch (...) {
                n->onNotificationException();
            }
        }
    }

    template <class T, typename P1>
    _noinline
    void post(
        const Ptr<T>& notifier, void (T::Notifiee::*func)(const P1),
        const P1 a1
    ) {
        for (const auto n : notifier->notifiees()) {
            try {
                (n->*func)(a1);
            } catch (...) {
                n->onNotificationException();
            }
        }
    }


    template <class T, typename P1>
    _noinline
    void post(
        T* const notifier, void (T::Notifiee::*func)(const P1& a1),
        const P1& a1
    ) {
        for (const auto n : notifier->notifiees()) {
            try {
                (n->*func)(a1);
            } catch (...) {
                n->onNotificationException();
            }
        }
    }

    template <class T, typename P1>
    _noinline
    void post(
        const Ptr<T>& notifier, void (T::Notifiee::*func)(const P1& a1),
        const P1& a1
    ) {
        for (const auto n : notifier->notifiees()) {
            try {
                (n->*func)(a1);
            } catch (...) {
                n->onNotificationException();
            }
        }
    }

}

#endif

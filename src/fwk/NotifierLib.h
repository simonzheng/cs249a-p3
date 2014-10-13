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
            const auto& a = n->activity();
            if (a == null || a->immediateDeliveryFlag()) {
                try {
                    (n->*func)();
                } catch (...) {
                    n->onNotificationException();
                }
            } else {
                a->postingNew(n, [=]() { (n->*func)(); });
            }
        }
    }

    template <class T, typename P1>
    _noinline
    void post(
        T* const notifier, void (T::Notifiee::*func)(const P1 a1),
        const P1 a1
    ) {
        for (const auto n : notifier->notifiees()) {
            const auto& a = n->activity();
            if (a == null || a->immediateDeliveryFlag()) {
                try {
                    (n->*func)(a1);
                } catch (...) {
                    n->onNotificationException();
                }
            } else {
                a->postingNew(n, [=]() { (n->*func)(a1); });
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
            const auto& a = n->activity();
            if (a == null || a->immediateDeliveryFlag()) {
                try {
                    (n->*func)(a1);
                } catch (...) {
                    n->onNotificationException();
                }
            } else {
                a->postingNew(n, [=]() { (n->*func)(a1); });
            }
        }
    }
}

#endif

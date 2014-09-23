/**
 * Activity interface for coordinating object interactions.
 */

#ifndef FWK_ACTIVITY_H
#define FWK_ACTIVITY_H

class Activity : public NamedInterface {
public:

    class Notifiee : public BaseNotifiee<Activity> {
    public:

        void notifierIs(const Ptr<Activity>& notifier) {
            connect(notifier, this);
        }

        /** Notification that the activity's current time has changed. */
        virtual void onNextTime() { }

        /** Notification that an activity's status has changed. */
	virtual void onStatus() {}
    };


    enum Status {
        free, waiting, ready, executing, nextTimeScheduled, deleted
    };


    /** Current activity status. */
    virtual Status status() = 0;

    /** Modify the current activity status. */
    virtual void statusIs(Status s) = 0;
    

    /** Time when the activity next should run. */
    virtual Time nextTime() = 0;

    /** Motify the time when the activity next should run. */
    virtual void nextTimeIs(const Time& t) = 0;

protected:

    typedef std::list<Notifiee*> NotifieeList;

public:

    NotifieeList& notifiees() {
        return notifiees_;
    }

protected:

    NotifieeList notifiees_;


    Activity(const string& name) :
        NamedInterface(name)
    {
        // Nothing else to do.
    }

};

#endif

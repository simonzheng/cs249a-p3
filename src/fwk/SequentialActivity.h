#ifndef FWK_SEQUENTIALACTIVITY_H
#define FWK_SEQUENTIALACTIVITY_H

using NotifierLib::post;

class SequentialActivity : public Activity {
public:

    static Ptr<Activity> instanceNew(
        const string& name, const Ptr<ActivityManager>& mgr
    ) {
        return new SequentialActivity(name, mgr);
    }

    Status status() {
        return status_;
    }

    void statusIs(const Status s) {
        if (status_ != s) {
            status_ = s;

            post(this, &Notifiee::onStatus);
        }
    }

    Time nextTime() {
        return nextTime_;
    }

    void nextTimeIs(const Time& t) {
        nextTime_ = t;

        post(this, &Notifiee::onNextTime);
    }

protected:

    Status status_;
    Time nextTime_;
    Ptr<ActivityManager> manager_;


    SequentialActivity(const string& name, const Ptr<ActivityManager>& mgr) :
        Activity(name),
        status_(free),
        nextTime_(0.0),
        manager_(mgr)
    {
        // Nothing else to do.
    }

};

#endif

/**
 * ActivityManager interface to control a set of activities.
 */

#ifndef FWK_ACTIVITYMANAGER_H
#define FWK_ACTIVITYMANAGER_H

class ActivityManager : public PtrInterface {
public:

    /**
     * Return the singleton activty manager instance.
     *
     * This value should be created by whatever activity manager chosen
     * at startup time.
     */
    static Ptr<ActivityManager> instance() {
        return instance_;
    }


    /**
     * Return the managed activity with the given name.
     */
    virtual Ptr<Activity> activity(const string& name) = 0;

    /**
     * Create a new activity associated with the given name.
     */
    virtual Ptr<Activity> activityNew(const string& name) = 0;

    /**
     * Remove the association between the given activity and the given name.
     */
    virtual void activityDel(const string& name) = 0;

    /**
     * Add the given activity to the end of the queue of activites to run.
     */
    virtual void lastActivityIs(const Ptr<Activity>& ptr) = 0;


    /**
     * Return the current time.
     */
    virtual Time now() = 0;

    /**
     * Modify the current time to the given time, running any activites
     * scheduled between the current time and t in chronological order.
     */
    virtual void nowIs(const Time& t) = 0;

protected:

    static Ptr<ActivityManager> instance_;

};

Ptr<ActivityManager> ActivityManager::instance_;

#endif

#ifndef TRAVELSIM_TRAVELINSTANCEMANAGER_H
#define TRAVELSIM_TRAVELINSTANCEMANAGER_H

#include "InstanceManager.h"

#include <sstream>

/**
 * InstanceManager implementation for the travel simulation.
 */
class TravelInstanceManager : public virtual InstanceManager {
public:

    /**
     * Return an instance manager for the travel simulation.
     */
    static Ptr<InstanceManager> instanceManager() {
        if (instanceManager_ == null) {
            instanceManager_ = new TravelInstanceManager();
            instanceManager_->initialize();
        }

        return instanceManager_;
    }

protected:

    typedef std::invalid_argument InvalidArgumentException;

    class LocationInstance : public Instance {
    public:

        _noinline
        string attribute(const string& name) {
            const auto i = segmentNumber(name) - 1;

            // TODO: Finish the implementation.

            return "";
        }

        _noinline
        void attributeIs(const string& name, const string& value) {
            // Ignore.
        }

    private:

        friend class TravelInstanceManager;


        static const string segmentStr;

        static const unsigned long segmentStrlen;


        // TODO: Finish the implementation.


        _noinline
        int segmentNumber(const string& name) {
            if (name.substr(0, segmentStrlen) == segmentStr) {
                // TODO: Check that name doesn't have extraneous characters.
                return std::stoi(name.c_str() + segmentStrlen);
            }

            return 0;
        }

    };


    class SegmentInstance : public Instance {
    public:

        _noinline
        string attribute(const string& name) {
            if (name == "source") {
                // TODO: Finish the implementation.
                return;
            }

            // TODO: Finish the implementation.

            return "";
        }

        _noinline
        void attributeIs(const string& name, const string& value) {
            if (name == "source") {
                // TODO: Finish the implementation.
                return;
            }

            // TODO: Finish the implementation.
        }

    private:

        friend class TravelInstanceManager;


        // TODO: Finish the implementation.

    };


    // TODO: Finish the implementation.


    class ConnInstance : public Instance {
    public:

        // TODO: Finish the implementation.

        _noinline
        string attribute(const string& name) {
            using std::stringstream;

            stringstream ss(name);

            string cmd;
            ss >> cmd;
            if (cmd != "explore") {
                return "";
            }

            string locName;
            ss >> locName;

            // TODO: Finish the implementation.

            return "";
        }

        _noinline
        void attributeIs(const string& name, const string& value) {
            // Ignore.
        }

    private:

        friend class TravelInstanceManager;


        // TODO: Finish the implementation.

    };


    static Ptr<TravelInstanceManager> instanceManager_;


    /**
     * Initialize any state in the manager.
     */
    void initialize() {
        // TODO: Finish the implementation.
    }

    /**
     * Returns a newly-created instance with the specified name.
     * The spec parameter indicates additional information about
     * the instance, such as its type.
     */
    Ptr<Instance> createInstance(const string& name, const string& spec) {
        if (spec == "Residence") {
            // TODO: Finish the implementation.
            return null;
        }

        // TODO: Finish the implementation.

        return null;
    }

};

const string TravelInstanceManager::LocationInstance::segmentStr = "segment";
const unsigned long TravelInstanceManager::LocationInstance::segmentStrlen =
    segmentStr.length();

Ptr<TravelInstanceManager> TravelInstanceManager::instanceManager_;

#endif

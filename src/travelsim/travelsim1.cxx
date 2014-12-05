#include "fwk/fwk.h"
#include <random>

#include "TravelNetwork.h"
#include <algorithm>

using namespace fwk;

/********************************************************************************
* Helper Classes and Functions                                                  *
*********************************************************************************/

/**
 * Helper class for random number generation.
 */
class Random : public PtrInterface {
public:

    static Ptr<Random> instanceNew() {
        return new Random();
    }


    double normal(const double mean, const double dev) {
        return dev * distribution_(generator_) + mean;
    }

    double normalRange(
        const double mean, const double dev,
        const double lower, const double upper
    ) {
        const auto r = normal(mean, dev);

        if (r < lower) {
            return lower;
        }

        if (r > upper) {
            return upper;
        }

        return r;
    }

protected:

    std::default_random_engine generator_;
    std::normal_distribution<double> distribution_;


    Random() :
        generator_(U32(SystemTime::now().value() & 0xffffffff)),
        distribution_(0.0, 1.0)
    {
        // Nothing else to do.
    }
};

static void logEntryNew(const Time t, const string& s) {
    std::cout << timeMilliAsString(t) << " " << s << std::endl;
}

void locationNew(
    const Ptr<TravelNetwork>& tn, const string& name, const string& spec
) {
    Ptr<Location> location;
    if (spec == "Residence") {
        location = Residence::instanceNew(name);
    } else if (spec == "Airport") {
        location = Airport::instanceNew(name);
    }
    tn->locationNew(location);
}

void segmentNew(
    const Ptr<TravelNetwork>& tn, const string& name, const string& spec, 
    const string& source, const string& dest, const double length
) {
    Ptr<Segment> seg;
    if (spec == "Flight") {
        seg = Flight::instanceNew(name);
    } else if (spec == "Road") {
        seg = Road::instanceNew(name);
    }
    tn->segmentNew(seg);
    seg->sourceIs(tn->location(source));
    seg->destinationIs(tn->location(dest));
    seg->lengthIs(length);
}

void vehicleNew(
    const Ptr<TravelNetwork>& tn, const string& name, const string& spec, 
    const unsigned int speed, const unsigned int capacity, const double cost, const string locationName
) {
    Ptr<Vehicle> vehicle;
    if (spec == "Airplane") {
        vehicle = Airplane::instanceNew(name);
    } else if (spec == "Car") {
        vehicle = Car::instanceNew(name);
    }
    tn->vehicleNew(vehicle);
    vehicle->speedIs(speed);
    vehicle->capacityIs(capacity);
    vehicle->costIs(cost);
    vehicle->locationIs(tn->location(locationName));
}

void tripNew(
    const Ptr<TravelNetwork>& tn, const string& name,
    const string& source, const string& dest, const size_t numTravelers
) {
    Ptr<Trip> trip = Trip::instanceNew(name);
    tn->tripNew(trip);
    trip->startLocationIs(tn->location(source));
    trip->endLocationIs(tn->location(dest));
    trip->numTravelersIs(numTravelers);
}


static const Ptr<TravelNetwork> setupNetwork(Ptr<TravelNetwork> tn) {
    locationNew(tn, "stanford1", "Residence");
    locationNew(tn, "menlopark1", "Residence");
    locationNew(tn, "sfo1", "Airport");
    locationNew(tn, "lax1", "Airport");

    vehicleNew(tn, "plane1", "Airplane", 500, 200, 40, "sfo1");
    vehicleNew(tn, "car1", "Car", 70, 5, 0.75, "stanford1");
    
    return tn;
}

/********************************************************************************
* Global Variables                                                              *
*********************************************************************************/

static Ptr<Random> rng = Random::instanceNew();


/********************************************************************************
* Simulation Classes                                                            *
*********************************************************************************/

/**
 * Common base class for simulations.
 */
class Sim : public Activity::Notifiee {
public:

    Ptr<Activity> activity() {
        return notifier();
    }
    void activityDel() {
        const auto a = notifier();
        a->manager()->activityDel(a->name());
    }

};

/**
 * TripRequesterSim is the simulator logic for a tripRequester that randomly 
 * requests trips from a particular TravelNetwork.
 */
class TripRequesterSim : public Sim {
public:

    static Ptr<TripRequesterSim> instanceNew(
        const Ptr<ActivityManager>& mgr, const Ptr<TravelNetwork>& travelNetwork
    ) {
        const Ptr<TripRequesterSim> sim = new TripRequesterSim(travelNetwork);
        const auto a = mgr->activityNew("TripRequesterSim");
        a->nextTimeIs(mgr->now());
        a->statusIs(Activity::scheduled);
        mgr->activityAdd(a);
        sim->notifierIs(a);
        return sim;
    }


    void onStatus() {
        const auto a = notifier();
        if (a->status() == Activity::running) {
            string tripName = tripNameFromNum(tripNum);
            logEntryNew(a->manager()->now(), "Adding trip: " + tripName);
            tripNew(travelNetwork_, tripName, "sfo1", "lax1", 350); // TODO: changes these tripNew fields to be random
            tripNum++;
            const auto delta = rng->normalRange(3.0, 1.5, 0.5, 5.0);
            a->nextTimeIsOffset(delta);
        }
    }

protected:

    string tripNameFromNum(unsigned int tripNum) {
        std::ostringstream oss;
        oss << "RequestedTrip" << tripNum;
        return oss.str();
    }

    Ptr<TravelNetwork> travelNetwork_;
    unsigned int tripNum; // counter for our trip numbers


    TripRequesterSim(const Ptr<TravelNetwork> travelNetwork) :
        travelNetwork_(travelNetwork),
        tripNum(1)
    {
        // Nothing else to do.
    }

};

/**
 * ServiceSim is the simulator logic for a serviceSim. 
 */
class ServiceSim : public Sim {
public:

    static Ptr<ServiceSim> instanceNew(
        const Ptr<ActivityManager>& mgr, const Ptr<TravelNetwork>& tn
    ) {
        const Ptr<ServiceSim> sim = new ServiceSim(tn);
        const auto a = mgr->activityNew("ServiceSim");
        sim->notifierIs(a);
        return sim;
    }

    void onTravelNetworkTripNew(const Ptr<Trip>& trip) {
        waitingTrips_.push_back(trip);
        if (availableVehicles_.size() > 0) {
            logEntryNew(notifier()->manager()->now(), trip->name() + ": ServiceSim will schedule the trip since there's at least one available car for: " + trip->name());
            scheduleTrip();
        } else {
            logEntryNew(notifier()->manager()->now(), trip->name() + ": ServiceSim will queue trip since no cars available");
        }
    }

    void onTravelNetworkVehicleNew(const Ptr<Vehicle>& vehicle) {
        availableVehicles_.push_back(vehicle);
        if (waitingTrips_.size() > 0) {
            logEntryNew(notifier()->manager()->now(), vehicle->name() + ": ServiceSim will schedule a trip since this car is available");
            scheduleTrip();
        } else {
            logEntryNew(notifier()->manager()->now(), vehicle->name() + ": ServiceSim will put this car in the list beacuse no trips waiting");
        }
    }

    void onTravelNetworkTripDel(const Ptr<Trip>& trip) {
        logEntryNew(notifier()->manager()->now(), "ServiceSim checking if it should remove: " + trip->name());
        const auto iter = find(waitingTrips_.begin(), waitingTrips_.end(), trip);
        if (iter != waitingTrips_.end()) {
            waitingTrips_.erase(iter);
            logEntryNew(notifier()->manager()->now(), "Erased waiting trip from serviceSim: " + trip->name());
        } else {
            logEntryNew(notifier()->manager()->now(), "Not a waiting trip so no removal in serviceSim: " + trip->name());
        }
    }

    void onTravelNetworkVehicleDel(const Ptr<Vehicle>& vehicle) {
        logEntryNew(notifier()->manager()->now(), "ServiceSim checking if it should remove: " + vehicle->name());
        const auto iter = find(availableVehicles_.begin(), availableVehicles_.end(), vehicle);
        if (iter != availableVehicles_.end()) {
            availableVehicles_.erase(iter);
            logEntryNew(notifier()->manager()->now(), "Erased available vehicle from serviceSim: " + vehicle->name());
        } else {
            logEntryNew(notifier()->manager()->now(), "Not a available vehicle so no removal in serviceSim: " + vehicle->name());
        }
    }



protected:

    void scheduleTrip() {
        logEntryNew(notifier()->manager()->now(), "Scheduling trip... [TODO]");
        // TODO: get nearest vehicle and create new tripSim, add activity to the manager, next time offset
        // const Ptr<TripSim> tripSim = TripSim::instanceNew(notifier()->manager(), trip);
        // logEntryNew(notifier()->manager()->now(), "New TripSim created after TripSim implemented");
        // assignNearestAvailableVehicle(trip, tripSim); // TODO: make this function, also ask if getNearestVehicle should be tn->getNearestVehicle
        // logEntryNew(notifier()->manager()->now(), "nearestAvailableVehicle assigned");

        // TOOD: consider setting trip's wait time above
        // notifier()->nextTimeIsOffset(delta);
        // notifier()->statusIs(Activity::scheduled);
        // notifier()->manager()->activityAdd(a);
    }

    // void assignNearestAvailableVehicle(const Ptr<Trip>& trip, const Ptr<TripSim>& tripSim) { // TODO: implement this
    //     Ptr<Vehicle> nearestAvailableVehicle = availableVehicles_.at(0); // TODO: change this later to get the path, distance, and nearest available vehicle and create the tripSim
    //     availableVehicles_.erase(0);
    //     if (nearestAvailableVehicle != null) {
    //         logEntryNew(notifier()->manager()->now(), "Found nearest nearestAvailableVehicle as: " + nearestAvailableVehicle->name() + ". Would create appropriate tripSim and path from here.");
    //     } else {
    //         logEntryNew(notifier()->manager()->now(), "Couldn't find nearest nearestAvailableVehicle");
    //     }
        
    //     // Ptr<Vehicle> nearestAvailableVehicle = null;
    //     // Miles nearestDistance = std::numeric_limits::max();
    //     // for (Ptr<Vehicle>& v : availableVehicles_) {
    //     //     // TODO: find nearest vehicle
    //     //     // Reomve nearest vehicle from queue
    //     //     // Assign it to the tripSim object
    //     //     // Assign the waitTime for the trip object
    //     // }
    //     return nearestAvailableVehicle;
    // }

    // Embedded TravelNetworkReactor
    class TravelNetworkReactor : public TravelNetwork::Notifiee {
    public:
        void onTripNew(const Ptr<Trip>& trip) {
            serviceSim_->onTravelNetworkTripNew(trip); //trampoline
        }
        void onTripDel(const Ptr<Trip>& trip) {
            serviceSim_->onTravelNetworkTripDel(trip); //trampoline
        }

        void onVehicleNew(const Ptr<Vehicle>& vehicle) {
            serviceSim_->onTravelNetworkVehicleNew(vehicle); //trampoline
        }
        void onVehicleDel(const Ptr<Vehicle>& vehicle) {
            serviceSim_->onTravelNetworkVehicleDel(vehicle); //trampoline
        }
    protected:
        friend class ServiceSim;
        TravelNetworkReactor(ServiceSim* const serviceSim, const Ptr<TravelNetwork>& tn) :
            serviceSim_(serviceSim)
        {
            notifierIs(tn);
        }
        ServiceSim* const serviceSim_; // weak pointer to prevent cycles
    };


    Ptr<TravelNetworkReactor> travelNetworkReactor_;
    vector<Ptr<Vehicle>> availableVehicles_;
    vector<Ptr<Trip>> waitingTrips_;

    ServiceSim(const Ptr<TravelNetwork>& tn) :
        travelNetworkReactor_(new TravelNetworkReactor(this, tn))
    {
        // Nothing else to do.
    }

};
/**
 * Main program creates a travel network, service simulation, and trip request simulation, and
 * then runs the simulation for a fixed period of time.
 */
int main(int argc, char *argv[]) {
    const auto mgr = SequentialManager::instance();
    const auto startTime = time(SystemTime::now());
    mgr->nowIs(startTime);
    
    const Ptr<TravelNetwork> tn1 = TravelNetwork::instanceNew("tn1");
    const Ptr<ServiceSim> serviceSim = ServiceSim::instanceNew(mgr, tn1);
    logEntryNew(startTime, "Setting up network");
    setupNetwork(tn1);
    // const Ptr<TripRequesterSim> tripRequesterSim = TripRequesterSim::instanceNew(mgr, tn1);

    logEntryNew(startTime, "*** [starting] ***");
    constexpr auto minutes = 60;
    mgr->nowIs(startTime + 0.5 * minutes);

    tripRequesterSim->activityDel();
    serviceSim->activityDel();

    std::cout << "Trip Statistics: ";
    cout << "numTrips: " << tn1->stats("stats")->numTrips() << endl;
    cout << "numPickups: " << tn1->stats("stats")->numPickups() << endl;
    cout << "numCompletedTrips: " << tn1->stats("stats")->numCompletedTrips() << endl;
    cout << "averageWaitTime: " << tn1->stats("stats")->averageWaitTime() << endl;

    return 0;
}

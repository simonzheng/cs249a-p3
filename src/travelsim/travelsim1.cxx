#include "fwk/fwk.h"
#include <random>

#include "TravelNetwork.h"
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace fwk;
using std::find;
using std::ostringstream;
using std::ifstream;
using std::cin;
using std::to_string;

/********************************************************************************
* Global Variables                                                              *
*********************************************************************************/
unsigned int minutesPerHour = 60;
unsigned int secondsPerMinute = 60;

// Global vars for setting particular simulations
bool randomTimes;
bool trackingModifiedSegments;
unsigned int desiredNumRequests;
double timeBetweenRequestsInSeconds;
double desiredOverallTimespanInSeconds;
int desiredNumParallelNetworks;
int desiredNumCarsInNetwork;
vector<string> allLocationNames;
vector<string> allVehicleNames;
vector<string> allTripNames;
vector<string> allSegmentNames;

/********************************************************************************
* Helper Classes and Functions                                                  *
*********************************************************************************/

string majorTripMessage(Ptr<Trip> trip, string message) {
    ostringstream oss;
    oss << "\n\t\t[**************************]\n\t\t" << message << ": " << trip->name() << " (" + trip->startLocation()->name() + "->" + trip->endLocation()->name() + ")" << "\n\t\t[**************************]\n";
    return oss.str();
}

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
    allLocationNames.push_back(name);
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
    allSegmentNames.push_back(name);
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
    vehicle->speedIs(speed);
    vehicle->capacityIs(capacity);
    vehicle->costIs(cost);
    vehicle->locationIs(tn->location(locationName));
    tn->vehicleNew(vehicle);
    allVehicleNames.push_back(name);
}

void tripNew(
    const Ptr<TravelNetwork>& tn, const string& name,
    const string& source, const string& dest, const size_t numTravelers
) {
    Ptr<Trip> trip = Trip::instanceNew(name);
    trip->startLocationIs(tn->location(source));
    trip->endLocationIs(tn->location(dest));
    trip->numTravelersIs(numTravelers);
    tn->tripNew(trip);
    allTripNames.push_back(name);
}

/********************************************************************************
* Global Variables (cont'd)                                                     *
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
        const Ptr<ActivityManager>& mgr, const Ptr<TravelNetwork>& travelNetwork, unsigned int simNum
    ) {
        const Ptr<TripRequesterSim> sim = new TripRequesterSim(travelNetwork, simNum);
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
            requestTrip(travelNetwork_, tripNum_, simNum_);
            Ptr<Trip> trip = travelNetwork_->trip(tripNameFromNum(tripNum_));
            logEntryNew(a->manager()->now(), majorTripMessage(trip, "Requested Trip:"));
            tripNum_++;
            if (!randomTimes) {
                a->nextTimeIsOffset(timeBetweenRequestsInSeconds);
            } else {
                a->nextTimeIsOffset(rng->normalRange(timeBetweenRequestsInSeconds, double(timeBetweenRequestsInSeconds)/3, 0, 2*timeBetweenRequestsInSeconds));
            }
        }
    }

protected:

    Ptr<TravelNetwork> travelNetwork_;
    unsigned int tripNum_; // counter for my trip numbers
    unsigned int simNum_;


    string tripNameFromNum(unsigned int tripNum) {
        std::ostringstream oss;
        oss << "RequestedTrip" << tripNum;
        return oss.str();
    }

    void requestTrip(Ptr<TravelNetwork>& tn, unsigned int tripNum, unsigned int simNum) {
        string tripName = tripNameFromNum(tripNum);
        if (simNum == 1) {
            tripNew(tn, tripName, "menlopark1", "stanford1", 30);
        } else if (simNum == 2) {
            tripNew(tn, tripName, "menlopark1", "sfo1", 30);
        } else if (simNum == 3) {
            int randomIndex = rand() % allLocationNames.size();
            int randomIndex2 = rand() % allLocationNames.size();
            while (randomIndex == randomIndex2) {
                randomIndex2 = rand() % allLocationNames.size();
            }
            // cout << randomIndex << ", " << randomIndex2 << "\n\n";
            tripNew(tn, tripName, allLocationNames[randomIndex], allLocationNames[randomIndex2], 30);
        }
    }

    TripRequesterSim(const Ptr<TravelNetwork> travelNetwork, unsigned int simNum) :
        travelNetwork_(travelNetwork),
        tripNum_(1),
        simNum_(simNum)
    {
        // Nothing else to do.
    }

};


/**
 * TripSim is the simulator logic for a trip.
 */
class TripSim : public Sim {
public:

    static Ptr<TripSim> instanceNew(
        const Ptr<ActivityManager>& mgr, const Ptr<Trip>& trip
    ) {
        return new TripSim(mgr->activityNew(trip->name()+"Sim"), trip);
    }


    void onStatus() {
        const auto a = notifier();
        if (a->status() == Activity::running) {
            // Trip::waitingForVehicle
            if (trip_->status() == Trip::waitingForVehicle) {
                logEntryNew(a->manager()->now(), majorTripMessage(trip_, "Started Trip"));
                const auto currLoc = trip_->vehicle()->location();
                Time timeToNextLoc;
                if (currLoc->name() != trip_->startLocation()->name()) {
                    // cout << "Found that a started trip has vehicle at " << currLoc->name() << "and startLocation() " << trip_->startLocation()->name() << endl; //debug
                    const auto currSeg = trip_->path()[0];
                    trip_->statusIs(Trip::goingToPickup);
                    timeToNextLoc = currSeg->length().value() / trip_->vehicle()->speed().value() * minutesPerHour * secondsPerMinute;
                    logEntryNew(a->manager()->now(), "[" + trip_->name() + "]: Trip::waitingForVehicle -> Trip::goingToPickup. (Expected: " + timeMilliAsString(notifier()->manager()->now() + timeToNextLoc) + ").");
                } else {
                    cout << "Found that a started trip has vehicle at " << currLoc->name() << "and startLocation() " << trip_->startLocation()->name() << endl; //debug
                    trip_->statusIs(Trip::goingToDropoff);
                    timeToNextLoc = 0;
                    logEntryNew(a->manager()->now(), "[" + trip_->name() + "]: Trip::waitingForVehicle -> Trip::goingToDropoff. (Expected: " + timeMilliAsString(notifier()->manager()->now() + timeToNextLoc) + ").");
                }
                a->nextTimeIsOffset(timeToNextLoc);

            // Trip::goingToPickup
            } else if (trip_->status() == Trip::goingToPickup) {
                const auto currLoc = trip_->vehicle()->location();
                if (currLoc == trip_->startLocation()) {
                    // calculate the new path from start to end location
                    pair<vector<Ptr<Segment>>, double> pathDistPair = trip_->travelNetwork()->conn("conn")->findShortestPath(currLoc, trip_->endLocation());
                    vector<Ptr<Segment>> shortestPath = pathDistPair.first;
                    trip_->pathIs(shortestPath);
                    trip_->statusIs(Trip::goingToDropoff);
                    cout << trip_->name() << ": " << trip_->startLocation()->name() << ".." << currLoc->name() << "->" << trip_->endLocation()->name() << endl; // debug
                    const auto currSeg = trip_->path()[0];
                    Time timeToNextLoc = currSeg->length().value() / trip_->vehicle()->speed().value() * minutesPerHour * secondsPerMinute;
                    logEntryNew(a->manager()->now(), "[" + trip_->name() + "]: Trip::goingToPickup -> Trip::goingToDropoff. (Expected: " + timeMilliAsString(notifier()->manager()->now() + timeToNextLoc) + ").");
                    a->nextTimeIsOffset(timeToNextLoc);
                } else {
                    for (unsigned int i = 0; i < trip_->path().size(); i++) {
                        const auto currSeg = trip_->path()[i];
                        if (currLoc == currSeg->source()) {
                            trip_->vehicle()->locationIs(currSeg->destination());
                            Time timeToNextLoc = currSeg->length().value() / trip_->vehicle()->speed().value() * minutesPerHour * secondsPerMinute;
                            logEntryNew(a->manager()->now(), "[" + trip_->name() + "]:\t\t " + currLoc->name() + " -> " + currSeg->destination()->name() + ". (Expected: " + timeMilliAsString(notifier()->manager()->now() + timeToNextLoc) + ").");
                            a->nextTimeIsOffset(timeToNextLoc);
                            return;
                        }
                    }
                }
            // Trip::goingToDropoff
            } else if (trip_->status() == Trip::goingToDropoff){
                const auto currLoc = trip_->vehicle()->location();
                if (currLoc == trip_->endLocation()) {
                    logEntryNew(a->manager()->now(), "[" + trip_->name() + "]: Trip::goingToDropoff -> Trip::droppedOff.");
                    trip_->statusIs(Trip::droppedOff);
                    a->statusIs(Activity::stopped);
                    logEntryNew(a->manager()->now(), majorTripMessage(trip_, "Finished Trip"));
                } else {
                    for (unsigned int i = 0; i < trip_->path().size(); i++) {
                        const auto currSeg = trip_->path()[i];
                        if (currLoc == currSeg->source()) {
                            trip_->vehicle()->locationIs(currSeg->destination());
                            Time timeToNextLoc = currSeg->length().value() / trip_->vehicle()->speed().value() * minutesPerHour * secondsPerMinute;
                            logEntryNew(a->manager()->now(), "[" + trip_->name() + "]:\t\t " + currLoc->name() + " -> " + currSeg->destination()->name() + ". (Expected: " + timeMilliAsString(notifier()->manager()->now() + timeToNextLoc) + ").");
                            a->nextTimeIsOffset(timeToNextLoc);
                            return;
                        }
                    }
                }
            }
        }
    }

protected:

    Ptr<Trip> trip_;


    TripSim(const Ptr<Activity>& activity, const Ptr<Trip>& trip) :
        trip_(trip)
    {
        activityIs(activity);
        const auto mgr = activity->manager();
        activity->immediateDeliveryFlagIs(false);
        activity->nextTimeIs(mgr->now());
        activity->statusIs(Activity::scheduled);
        mgr->activityAdd(activity);
        notifierIs(activity);
    }
};




/**
 * ServiceSim is the simulator logic for a service that dispatches vehicles to trips. 
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
        if (availableVehicles_.size() > 0) {
            assignNearestAvailableVehicle(trip);
            if (trip->vehicle() != null) {
                logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + trip->name() + "," + trip->vehicle()->name() + "]: will schedule trip with assigned vehicle");
                return;
            }
        }
        waitingTrips_.push_back(trip); // Push back the trip if I can't find a vehicle that can service it
        logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + trip->name() + "]: will not schedule trip because no available reachable vehicle");
    }

    void onTravelNetworkTripDel(const Ptr<Trip>& trip) {
        logEntryNew(notifier()->manager()->now(), "ServiceSim checking if it should remove: " + trip->name());
        const auto iter = find(waitingTrips_.begin(), waitingTrips_.end(), trip);
        if (iter != waitingTrips_.end()) {
            waitingTrips_.erase(iter);
            logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + trip->name() + "]: Erased waiting trip from serviceSim");
        } else {
            logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + trip->name() + "]: Unable to find waiting trip so no erasure from serviceSim");
        }
    }

    void onTravelNetworkVehicleNew(const Ptr<Vehicle>& vehicle) {
        availableVehicles_.push_back(vehicle);
        if (waitingTrips_.size() > 0) {
            for (auto& trip : waitingTrips_) {
                assignNearestAvailableVehicle(trip);
                if (trip->vehicle() != null) {
                    logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + trip->name() + "," + trip->vehicle()->name() + "]: will schedule trip with assigned vehicle");
                    return;
                }
            }
        }
        logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + vehicle->name() + "]: will not schedule this vehicle because no available reachable trips");
    }

    void onTravelNetworkVehicleDel(const Ptr<Vehicle>& vehicle) {
        logEntryNew(notifier()->manager()->now(), "ServiceSim checking if it should remove: " + vehicle->name());
        const auto iter = find(availableVehicles_.begin(), availableVehicles_.end(), vehicle);
        if (iter != availableVehicles_.end()) {
            availableVehicles_.erase(iter);
            logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + vehicle->name() + "]: Erased available vehicle from serviceSim");
        } else {
            logEntryNew(notifier()->manager()->now(), "Not a available vehicle so no removal in serviceSim: " + vehicle->name());
            logEntryNew(notifier()->manager()->now(), "[ServiceSim: " + vehicle->name() + "]: Unable to find available vehicle so no erasure from serviceSim");
        }
    }



protected:

    void assignNearestAvailableVehicle(Ptr<Trip> trip) {
        // logEntryNew(notifier()->manager()->now(), "assignNearestAvailableVehicle for " + trip->name());
        
        // Setup assignee variables
        Miles shortestDistance = numeric_limits<double>::max();
        Ptr<Vehicle> closestVehicle = null;
        vector<Ptr<Segment>> shortestPath;
        // Find shortest path on each vehicle
        for (Ptr<Vehicle>& vehicle : availableVehicles_) {
            Ptr<Location> vehicleLocation = vehicle->location();
            string pickupLocName = trip->startLocation()->name();
            if (vehicleLocation != null) {
                const auto travelNetwork_ = travelNetworkReactor_->notifier();
                pair<vector<Ptr<Segment>>, double> pathDistPair = travelNetwork_->conn("conn")->findShortestPath(vehicleLocation, trip->startLocation());
                vector<Ptr<Segment>> path = pathDistPair.first;
                cout << trip->startLocation()->name() << "(" << vehicleLocation->name() << " -> " << trip->startLocation()->name() << ": " << path.size() << ")" << trip->endLocation()->name() << endl; // debug
                // if (path.size() == 0) exit(-1);
                Miles distance = pathDistPair.second;
                if (distance.value() < shortestDistance.value()) {
                    closestVehicle = vehicle;
                    shortestDistance = distance;
                    shortestPath = path;
                }
            } else {
                cerr << "Could not find the starting location for the trip (" << trip->name() << "). Skipping the vehicle: " << vehicle->name() << endl;
                continue;
            }
        }

        if (closestVehicle == null) {
            return;
        }
        // logEntryNew(notifier()->manager()->now(), "[" + trip->name() + "]: Trip assigned nearest reachable available vehicle " + closestVehicle->name());
        trip->vehicleIs(closestVehicle);
        removeAssignedTripAndVehicle(trip, closestVehicle);
        trip->pathIs(shortestPath);
        Time waitTimeInSeconds = shortestDistance.value() / closestVehicle->speed().value() * minutesPerHour * secondsPerMinute;
        trip->waitTimeIs(waitTimeInSeconds);
        Ptr<TripSim> tripSim = TripSim::instanceNew(notifier()->manager(), trip);
        tripSimsVector_.push_back(tripSim);
    }

    void removeAssignedTripAndVehicle(Ptr<Trip>& trip, Ptr<Vehicle>& vehicle) {
        // Remove the available trip
        waitingTrips_.erase(std::remove(waitingTrips_.begin(), waitingTrips_.end(), trip), waitingTrips_.end());

        // Remove the available vehicle
        availableVehicles_.erase(std::remove(availableVehicles_.begin(), availableVehicles_.end(), vehicle), availableVehicles_.end());
    }

    // Embedded TravelNetworkReactor
    class TravelNetworkReactor : public TravelNetwork::Notifiee {
    public:
        void onTripNew(const Ptr<Trip>& trip) {
            auto tripTracker = TripTracker::instanceNew(trip);
            serviceSim_->tripTrackerMap_[trip->name()] = tripTracker;
            tripTracker->serviceSim_ = serviceSim_; // from slide 28 in lecture3.pdf

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


    /********************************************************
    * Nest TripTracker in ServiceSim                        *
    ********************************************************/
    class TripTracker : public Trip::Notifiee {
    public:
        static Ptr<TripTracker> instanceNew(const Ptr<Trip>& trip) { 
            const Ptr<TripTracker> tripTracker = new TripTracker();
            tripTracker->notifierIs(trip);
            return tripTracker;
        }

        /** Notification that the trip's status changed. */
        void onStatus() {
            if (notifier()->status() == Trip::droppedOff) {
                serviceSim_->onTravelNetworkVehicleNew(notifier()->vehicle());
                notifier()->vehicleIs(null); // TODO: check if I want to nullify the vehicle
            };
        }

        // We can make this public because it's only available to the stats class.
        Ptr<ServiceSim> serviceSim_;
    };
    
    typedef unordered_map< string, Ptr<TripTracker> > TripTrackerMap;
    TripTrackerMap tripTrackerMap_;
    Ptr<TravelNetworkReactor> travelNetworkReactor_;
    vector<Ptr<Vehicle>> availableVehicles_;
    vector<Ptr<Trip>> waitingTrips_;
    vector<Ptr<TripSim>> tripSimsVector_;

    ServiceSim(const Ptr<TravelNetwork>& tn) :
        travelNetworkReactor_(new TravelNetworkReactor(this, tn))
    {
        // Nothing else to do.
    }
};

// /********************************************************************************
// * Helper Classes and Functions (cont'd)                                          *
// *********************************************************************************/

void printChooser() {
    cout << "\n\n************************************************************************************************************\n";
    cout << "Welcome to travelsim! To try one simulation, please choose from the following:\n";
    cout << "1 - \tA simulation in which 60 requests are made for a TravelNetwork with only one car (car1) starting at stanford1. \n";
    cout << "\t All requests are from menlopark1 to stanford1, so the car rapidly shuttles back and forth and the trips are made \n";
    cout << "\t at regular 10-minute intervals by my TripRequesterSim object. \n";
    cout << "\t This simple simulation demonstrates that one car can service multiple vehicles and shows that this simulation \n";
    cout << "\t updates the activity at every location on the path from the vehicle start location to pickup site to the dropoff site.\n";
    cout << "\t Further, because trips are requested faster than car1 can process them, this shows that trips are inserted into the \n";
    cout << "\t waiting trips queue and pulled off of the waiting trips queue properly.\n";

    cout << "2 - \tA simulation in which 60 requests are made for a TravelNetwork with 8 cars starting at stanford1. \n";
    cout << "\t All requests are made periodically according to a normal distribution with mean=10 min. and stddev=3.33 min by TripRequesterSim.\n";
    cout << "\t All requests are from menlopark1 to sfo1, which only has the path menlopark1->stanford1->sfo1, so the car drives between the 3. \n";
    cout << "\t there are more cars servicing requests asynchronously (with respect to virtual time), I complete almost all trips.\n";

    cout << "3 - \tA simulation in which 3000 requests are made for a TravelNetwork with 20 locations and 40 cars starting at stanford1. \n";
    cout << "\t In this scenario, all trips requested by TripRequesterSimare from any uniformly random location to any other uniformly random location.\n";
    cout << "\t All requests are made periodically according to a normal distribution with mean=10 min and stddev=3.33 min by TripRequesterSim.\n";
    cout << "\t This demonstrates that my network is able to handle a high number of requests, and that although I can only store 20 cache entries,\n";
    cout << "\t and there are 20*19=380 possibly optimal paths, my cache efficiency remains around 88% baseline with no changes to the network. \n";

    cout << "4 - \tSimilar to 3, this simulation has 3000 requests for a TravelNetwork with 20 locations and 40 cars starting at stanford1. \n";
    cout << "\t Trip locations are likewise chosen uniformly randomly and requests are still normally distributted with mean=10 min and stddev=3.33 min.\n";
    cout << "\t The difference is that I will close 2 redundant segments at random times in my network now test a cache policy in which I now ,\n";
    cout << "\t and there are 20*19=380 possibly optimal paths, my cache efficiency remains around 88% baseline with no changes to the network. \n";
    
    cout << "************************************************************************************************************\n\n";
}

static const void setupConnectedParallelNetworks(Ptr<TravelNetwork>& tn, int numParallelNetworks) {
    int oldNum = 1;
    int newNum = 2;
    for (int i = 0; i < numParallelNetworks; i++) {
        cout << "making parallel" << endl; // debug
        string oldstanford = "stanford" + to_string(oldNum);
        string newstanford = "stanford" + to_string(newNum);
        string oldmenlopark = "menlopark" + to_string(oldNum);
        string newmenlopark = "menlopark" + to_string(newNum);
        string oldsfo = "sfo" + to_string(oldNum);
        string newsfo = "sfo" + to_string(newNum);
        string oldlax = "lax" + to_string(oldNum);
        string newlax = "lax" + to_string(newNum);

        locationNew(tn, newstanford, "Residence");
        locationNew(tn, newmenlopark, "Residence");
        locationNew(tn, newsfo, "Airport");
        locationNew(tn, newlax, "Airport");


        segmentNew(tn, "parallelDimensionSeg1-" + to_string(newNum), "Road", oldstanford, newstanford, 50);
        segmentNew(tn, "parallelDimensionSeg2-" + to_string(newNum), "Road", oldmenlopark, newmenlopark, 50);
        segmentNew(tn, "parallelDimensionSeg3-" + to_string(newNum), "Road", oldsfo, newsfo, 50);
        segmentNew(tn, "parallelDimensionSeg4-" + to_string(newNum), "Road", oldlax, newlax, 50);

        segmentNew(tn, "parallelDimensionSeg1Reverse-" + to_string(newNum), "Road", newstanford, oldstanford, 50);
        segmentNew(tn, "parallelDimensionSeg2Reverse-" + to_string(newNum), "Road", newmenlopark, oldmenlopark, 50);
        segmentNew(tn, "parallelDimensionSeg3Reverse-" + to_string(newNum), "Road", newsfo, oldsfo, 50);
        segmentNew(tn, "parallelDimensionSeg4Reverse-" + to_string(newNum), "Road", newlax, oldlax, 50);

        segmentNew(tn, "carSeg1-" + to_string(newNum), "Road", newstanford, newsfo, 20);
        segmentNew(tn, "carSeg2-" + to_string(newNum), "Road", newsfo, newstanford, 20);
        segmentNew(tn, "carSeg3-" + to_string(newNum), "Road", newmenlopark, newstanford, 20);
        segmentNew(tn, "carSeg4-" + to_string(newNum), "Road", newsfo, newmenlopark, 20);
        segmentNew(tn, "carSeg5-" + to_string(newNum), "Road", newstanford, newmenlopark, 5);
        segmentNew(tn, "carSeg6-" + to_string(newNum), "Road", newmenlopark, newstanford, 5);
        segmentNew(tn, "roadTripSeg1-" + to_string(newNum), "Road", newsfo, newlax, 350);
        segmentNew(tn, "roadTripSeg2-" + to_string(newNum), "Road", newlax, newsfo, 350);
        oldNum++;
        newNum++;
    }

}

static const Ptr<TravelNetwork> setupNetwork(Ptr<TravelNetwork> tn, unsigned int simNum) {
    // Base network
    locationNew(tn, "stanford1", "Residence");
    locationNew(tn, "menlopark1", "Residence");
    locationNew(tn, "sfo1", "Airport");
    locationNew(tn, "lax1", "Airport");

    segmentNew(tn, "carSeg1", "Road", "stanford1", "sfo1", 20);
    segmentNew(tn, "carSeg2", "Road", "sfo1", "stanford1", 20);
    segmentNew(tn, "carSeg3", "Road", "menlopark1", "stanford1", 20);
    segmentNew(tn, "carSeg4", "Road", "sfo1", "menlopark1", 20);
    segmentNew(tn, "carSeg5", "Road", "stanford1", "menlopark1", 5);
    segmentNew(tn, "carSeg6", "Road", "menlopark1", "stanford1", 5);
    segmentNew(tn, "roadTripSeg1", "Road", "sfo1", "lax1", 350);
    segmentNew(tn, "roadTripSeg2", "Road", "lax1", "sfo1", 350);

    // Configurations for different simulations
    cout << desiredNumCarsInNetwork << endl;
    for (int i = 0; i < desiredNumCarsInNetwork; ++i) {
        cout << "making vechiel" << endl; // debug
        vehicleNew(tn, "car" + to_string(i), "Car", 70, 5, 0.75, "stanford1");
    }
    setupConnectedParallelNetworks(tn, desiredNumParallelNetworks);
    return tn;
}

unsigned int maxSimNum = 3;
void setSimulationVars(unsigned int simNum) {
    desiredNumRequests = 60;
    desiredNumParallelNetworks = 0;
    desiredNumCarsInNetwork = 1;
    desiredOverallTimespanInSeconds =  desiredNumRequests * 10 * secondsPerMinute;
    randomTimes = false;
    if (simNum == 2) {
        desiredNumCarsInNetwork = 8;
        randomTimes = true;
    } else if (simNum == 3) {
        desiredNumCarsInNetwork = 40;
        desiredNumParallelNetworks = 4; // adds desiredNumParallelNetworks*4 locations to the network (which by default starts with 4 locations)
        randomTimes = true;
        desiredNumRequests = 3000;
    }
    timeBetweenRequestsInSeconds = desiredOverallTimespanInSeconds / desiredNumRequests - 1;
}

void printTripStatistics(Ptr<TravelNetwork> tn) {
    cout << "Stats:" << endl;
    Ptr<Stats> stats = tn->stats("stats1");
    cout << "# Residences:\t" << stats->numResidences() << endl;
    cout << "# Airports:\t" << stats->numAirports() << endl;
    cout << "# Roads:\t" << stats->numRoads() << endl;
    cout << "# Car Vehicles:\t" << stats->numCars() << endl;

    cout << endl;
    cout << "Trip Statistics:\t" << endl;
    cout << "numTrips:\t" << tn->stats("stats")->numTrips() << endl;
    cout << "numCompleted:\t" << tn->stats("stats")->numCompletedTrips() << endl;
    cout << "averageWait:\t" << (tn->stats("stats")->averageWaitTime().value() / secondsPerMinute) << " minutes."<< endl;

    cout << endl;
    cout << "Cache Statistics:\t" << endl;
    cout << "numCacheHits:\t" << tn->conn("conn")->numCacheHits() << endl;
    cout << "numCacheChecks:\t" << tn->conn("conn")->numCacheChecks() << endl;
    cout << "Efficiency:\t" << (tn->conn("conn")->cacheEfficiency()*100) << "\%" << endl;
    cout << endl;
}

/**
 * Main program creates a travel network, service simulation, and trip request simulation, and
 * then runs the simulation for a fixed period of time.
 */
int main(int argc, char *argv[]) {
    printChooser();
    unsigned int simNum;
    while (true) {
        cin >> simNum;
        if (simNum > 0 && simNum <= maxSimNum) break;
        cout << "You must choose an integer between 1 and " << maxSimNum << endl;
    }
    setSimulationVars(simNum);


    // Set up activity manager
    const auto mgr = SequentialManager::instance();
    const auto startTime = time(SystemTime::now());
    mgr->nowIs(startTime);

    // Setup TravelNetwork and TripRequester
    const Ptr<TravelNetwork> tn = TravelNetwork::instanceNew("tn");
    const Ptr<ServiceSim> serviceSim = ServiceSim::instanceNew(mgr, tn);
    setupNetwork(tn, simNum);
    const Ptr<TripRequesterSim> tripRequesterSim = TripRequesterSim::instanceNew(mgr, tn, simNum);

    // Start Running Simulation
    logEntryNew(startTime, "\n****************************************\n"
                            "*********[Starting Simulation]**********\n"
                            "****************************************\n");
    mgr->nowIs(startTime + desiredOverallTimespanInSeconds);
    tripRequesterSim->activityDel();
    serviceSim->activityDel();
    logEntryNew(mgr->now(), "\n****************************************\n"
                            "*********[Finished Simulation]**********\n"
                            "****************************************\n");

    // Print statistics
    printTripStatistics(tn);
    cout << "Feel free to run another simulation!" << endl;
    
    return 0;
}
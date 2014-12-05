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

/********************************************************************************
* Global Variables                                                              *
*********************************************************************************/
unsigned int minutesPerHour = 60;
unsigned int secondsPerMinute = 60;

bool randomTimes = false;
unsigned int desiredNumRequests = 15;
double timeBetweenRequestsInSeconds = 5 * secondsPerMinute;
double desiredOverallTimespanInSeconds =  desiredNumRequests * timeBetweenRequestsInSeconds;

/********************************************************************************
* Helper Classes and Functions                                                  *
*********************************************************************************/

string startedTripMessage(Ptr<Trip> trip) {
    ostringstream oss;
    oss << "\n\t\t[**************************]\n\t\t" << "Started Trip: " << trip->name() << "\n\t\t[**************************]\n";
    return oss.str();
}

string finishedTripMessage(Ptr<Trip> trip) {
    ostringstream oss;
    oss << "\n\t\t[**************************]\n\t\t" << "Finished Trip: " << trip->name() << "\n\t\t[**************************]\n";
    return oss.str();
}

void printTripStatistics(Ptr<TravelNetwork> tn) {
    std::cout << "Trip Statistics: ";
    cout << "numTrips: " << tn->stats("stats")->numTrips() << endl;
    cout << "numCompletedTrips: " << tn->stats("stats")->numCompletedTrips() << endl;
    cout << "averageWaitTime: " << (tn->stats("stats")->averageWaitTime().value() / secondsPerMinute) << " minutes."<< endl;
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
    vehicle->speedIs(speed);
    vehicle->capacityIs(capacity);
    vehicle->costIs(cost);
    vehicle->locationIs(tn->location(locationName));
    tn->vehicleNew(vehicle);
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
}


static const Ptr<TravelNetwork> setupNetwork(Ptr<TravelNetwork> tn) {
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
    segmentNew(tn, "flightSeg1", "Flight", "sfo1", "lax1", 350);

    vehicleNew(tn, "car1", "Car", 70, 5, 0.75, "stanford1");
    return tn;
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
            tripNew(travelNetwork_, tripName, "menlopark1", "stanford1", 30); // TODO: changes these tripNew fields to be random
            tripNum++;
            a->nextTimeIsOffset(timeBetweenRequestsInSeconds);
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
                cout << startedTripMessage(trip_) << endl;
                trip_->statusIs(Trip::goingToPickup);
                const auto currLoc = trip_->vehicle()->location();
                const auto currSeg = trip_->path()[0];
                Time timeToNextLoc = currSeg->length().value() / trip_->vehicle()->speed().value() * minutesPerHour * secondsPerMinute;
                logEntryNew(a->manager()->now(), "[" + trip_->name() + "]: Trip::waitingForVehicle -> Trip::goingToPickup. (Expected: " + timeMilliAsString(notifier()->manager()->now() + timeToNextLoc) + ").");
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
                    logEntryNew(a->manager()->now(), finishedTripMessage(trip_));
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
                logEntryNew(notifier()->manager()->now(), trip->name() + ": ServiceSim will schedule the trip since there's at least one available reachable car for: " + trip->name());
                return;
            } else {
                logEntryNew(notifier()->manager()->now(), trip->name() + ": ServiceSim will not schedule the trip since the car isn't reachable to " + trip->name());
            }
        }
        waitingTrips_.push_back(trip); // Push back the trip if we can't find a vehicle that can service it
        logEntryNew(notifier()->manager()->now(), trip->name() + ": ServiceSim will queue trip since no reachable cars available");
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

    void onTravelNetworkVehicleNew(const Ptr<Vehicle>& vehicle) {
        availableVehicles_.push_back(vehicle);
        if (waitingTrips_.size() > 0) {
            for (auto& trip : waitingTrips_) {
                assignNearestAvailableVehicle(trip);
                if (trip->vehicle() != null) {
                    logEntryNew(notifier()->manager()->now(), vehicle->name() + ": ServiceSim will schedule a trip since this car is available and can reach the trip");
                    return;
                }
            }
        } else {
            logEntryNew(notifier()->manager()->now(), vehicle->name() + ": ServiceSim will put this car in the list because there are no reachable waiting trips");
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
        logEntryNew(notifier()->manager()->now(), "assignNearestAvailableVehicle successfully assigned " + trip->name() + " the vehicle " + closestVehicle->name());
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
                notifier()->vehicleIs(null); // TODO: check if we want to nullify the vehicle
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

// void setupSimulation(const Ptr<TravelNetwork>& tn, const Ptr<TripRequesterSim>& trs, string simNumAsString) {
//     // Setup TravelNetwork
//     string line;
//     string setupFileName = "sim" + simNumAsString + "-setup.txt";
//     ifstream setupFile (setupFileName);
//     if (setupFile.is_open()) {
//         while (getline(setupFile,line)) {
//             cout << line << '\n';
//         }
//         setupFile.close();
//     } else {
//         cout << "Could not open file: " << setupFileName << endl;
//     }

//     // Setup TripRequester
//     // const Ptr<TripRequesterSim> tripRequesterSim = TripRequesterSim::instanceNew(mgr, tn);
// }
/**
 * Main program creates a travel network, service simulation, and trip request simulation, and
 * then runs the simulation for a fixed period of time.
 */
int main(int argc, char *argv[]) {
    // Set up activity manager
    const auto mgr = SequentialManager::instance();
    const auto startTime = time(SystemTime::now());
    mgr->nowIs(startTime);

    // Setup TravelNetwork and TripRequester
    const Ptr<TravelNetwork> tn = TravelNetwork::instanceNew("tn");
    const Ptr<ServiceSim> serviceSim = ServiceSim::instanceNew(mgr, tn);
    setupNetwork(tn);
    const Ptr<TripRequesterSim> tripRequesterSim = TripRequesterSim::instanceNew(mgr, tn);

    // Start Running Simulation
    logEntryNew(startTime, "\n****************************************\n*********[Starting Simulation]**********\n****************************************\n");
    mgr->nowIs(startTime + desiredOverallTimespanInSeconds);
    tripRequesterSim->activityDel();
    serviceSim->activityDel();
    logEntryNew(mgr->now(), "\n****************************************\n*********[Finished Simulation]**********\n****************************************\n");

    // Print statistics
    printTripStatistics(tn);
    return 0;
}

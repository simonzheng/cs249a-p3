// TravelNetwork.h
// By Simon Zheng for CS 249A Fall 2014.
//

#include <sstream>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <ostream>
#include <iostream>
#include "fwk/fwk.h"
#include "Cache.h"

using std::cout;
using std::cerr;
using std::endl;
using fwk::BaseNotifiee;
using fwk::NamedInterface;
using fwk::NotifierLib::post;
using fwk::Ptr;
using fwk::Ordinal;
using fwk::Time;
using std::pair;
using std::make_pair;
using std::numeric_limits;


using std::unordered_map;
using std::set;
using std::queue;
using std::vector;

// Declaring Ordinal variable types that may be useful
class PassengersType{};
class Passengers : public Ordinal<PassengersType, unsigned int> {
public:
    Passengers() : Ordinal<PassengersType, unsigned int>(0) {}
    // Modified constructor to prevent negative cost value
    Passengers (const double v) : Ordinal<PassengersType, unsigned int>(v) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v));
        }
    } 
    Passengers (Ordinal<PassengersType, unsigned int> v) : Ordinal<PassengersType, unsigned int>(v.value()) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v.value()));
        }
    }
};

class DollarsPerMileType{};
class DollarsPerMile : public Ordinal<DollarsPerMileType, double> {
public:
    DollarsPerMile() : Ordinal<DollarsPerMileType, double>(0) {}
    // Modified constructor to prevent negative cost value
    DollarsPerMile (const double v) : Ordinal<DollarsPerMileType, double>(v) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v));
        }
    } 
    DollarsPerMile (Ordinal<DollarsPerMileType, double> v) : Ordinal<DollarsPerMileType, double>(v.value()) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v.value()));
        }
    } 
};

class MilesType{};
class Miles : public Ordinal<MilesType, double> {
public:
    Miles() : Ordinal<MilesType, double>(0) {}
    // Modified constructor to prevent negative cost value
    Miles (const double v) : Ordinal<MilesType, double>(v) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v));
        }
    }
    Miles (Ordinal<MilesType, double> v) : Ordinal<MilesType, double>(v.value()) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v.value()));
        }
    }
};
class MilesPerHourType{};
class MilesPerHour : public Ordinal<MilesPerHourType, double> {
public:
    MilesPerHour() : Ordinal<MilesPerHourType, double>(0) {}
    // Modified constructor to prevent negative cost value
    MilesPerHour (const double v) : Ordinal<MilesPerHourType, double>(v) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v));
        }
    }
    MilesPerHour (Ordinal<MilesPerHourType, double> v) : Ordinal<MilesPerHourType, double>(v.value()) {
        if (v < 0) {
            throw fwk::RangeException(std::to_string(v.value()));
        }
    }
};


/******************************************************************************
*******************************************************************************
******************************************************************************/
class Segment; // forward declared so Location can refer to it.
class TravelNetwork;

// A location is a place where passenger travel starts or ends, or an 
// intermediary point along the way. Some intermediate locations allow 
// passengers to switch to a different mode of transportation for the next 
// portion of their trip. For example, an airport is a location where 
// passengers might switch from a car to a plane, or switch from one plane 
// to another.

class Location : public NamedInterface {
public:
    class Notifiee : public BaseNotifiee<Location> {
    public:
        void notifierIs(const Ptr<Location>& location) {
            connect(location, this);
        }
    };

protected:
    typedef vector< Ptr<Segment> > SegmentVector;
    typedef std::list<Notifiee*> NotifieeList;

public:
    typedef SegmentVector::iterator iterator;
    typedef SegmentVector::const_iterator const_iterator;
    typedef SegmentVector::size_type size_type;

    // Remove the copy and assignment constructors
    Location(const Location&) = delete;
    void operator =(const Location&) = delete;

    size_type segmentCount() const {
        return segmentVector_.size();
    }

    const_iterator segmentIter() {
        return segmentVector_.cbegin();
    }

    const_iterator segmentIterEnd() {
        return segmentVector_.cend();
    }

    Ptr<Segment> segment(const size_type i) {
        if (i > segmentCount()) {
            cerr << "Error in segment(): Cannot access the segment at index " << i << endl;
            return null;
        }
        return segmentVector_[i];
    }

    void segmentNew(const Ptr<Segment>& segment) {
        if (segment == null) {
            cerr << "Error in segmentNew(): The segment is null" << endl;
            return;
        }
        segmentVector_.push_back(segment);
    }

    Ptr<Segment> segmentDel(const size_type i) {
        if (i > segmentCount()) {
            cerr << "Error in segmentDel(): Cannot access the segment at index " << i << endl;
            return null;
        }
        Ptr<Segment> segment = segmentVector_[i];
        segmentVector_.erase(segmentVector_.begin() + i);
        return segment;
    }

    // travelNetwork
    Ptr<TravelNetwork> travelNetwork() {
        return travelNetwork_;
    }
    void travelNetworkIs(const Ptr<TravelNetwork>& travelNetwork) {
        travelNetwork_ = travelNetwork;
    }

    NotifieeList& notifiees() {
        return notifiees_;
    }

protected:
    SegmentVector segmentVector_;
    NotifieeList notifiees_;
    Ptr<TravelNetwork> travelNetwork_ = null;

    explicit Location(const string& name) : NamedInterface(name)
    {
        // Nothing else to do.
    }
    ~Location() { }
};

/********************************************************
* Residence (subtype of Location)                       *
********************************************************/
class Residence : public Location {
public:
    /**
     * Return a new instance with the given name.
     */
    static Ptr<Residence> instanceNew(const string& name) {
        return new Residence(name);
    }

protected:
    Residence(const string& name) :
        Location(name)
    {
        // Nothing else to do.
    }
};

/********************************************************
* Airport (subtype of Location)                                    *
********************************************************/
class Airport : public Location {
public:
    /**
     * Return a new instance with the given name.
     */
    static Ptr<Airport> instanceNew(const string& name) {
        return new Airport(name);
    }

protected:
    Airport(const string& name) :
        Location(name)
    {
        // Nothing else to do.
    }
};

/******************************************************************************
*******************************************************************************
******************************************************************************/

// A segment connects one location to another. In addition to the starting 
// and ending locations, segments have a length attribute, which is the 
// mileage from start to end.
class Segment : public NamedInterface {
public:
    class Notifiee : public BaseNotifiee<Segment> {
    public:
        void notifierIs(const Ptr<Segment>& segment) {
            connect(segment, this);
        }
    };

protected:
    typedef std::list<Notifiee*> NotifieeList;

public:

    // Remove the copy and assignment constructors
    Segment(const Segment&) = delete;
    void operator =(const Segment&) = delete;


    // Source
    Ptr<Location> source() {
        return source_;
    }

    void sourceIs(const Ptr<Location>& source) {
        if (source != null && source->travelNetwork() != travelNetwork_) {
            string errorMessage = "Error in sourceIs(): Segment's source and destination should be in same travelNetwork as segment!";
            cerr << errorMessage << endl;
            throw fwk::DifferentNetworkException(errorMessage);
        }
        if (source_ != null) {
            // Removing this source from existing source Location's segment list
            int segmentIndexInSourceList = 0; // 
            for (auto it = source_->segmentIter(); it != source_->segmentIterEnd(); ++it) {
                if ((*it)->name() == this->name()) {
                    source_->segmentDel(segmentIndexInSourceList);
                }
            }
        }
        if (source != null) {
            // Connect new source Location's Segment
            source->segmentNew(this);
        }
        source_ = source;
    }

    // Destination
    Ptr<Location> destination() {
        return destination_;
    }

    void destinationIs(const Ptr<Location>& destination) {
        if (destination != null && destination->travelNetwork() != travelNetwork_) {
            string errorMessage = "Error in destinationIs(): Segment's source and destination should be in same travelNetwork as segment!";
            cerr << errorMessage << endl;
            throw fwk::DifferentNetworkException(errorMessage);
        }
        destination_ = destination;
    }

    // Length
    Miles length() {
        return length_;
    }

    void lengthIs(const Miles length) {
        length_ = length;
    }

    // travelNetwork
    Ptr<TravelNetwork> travelNetwork() {
        return travelNetwork_;
    }
    void travelNetworkIs(const Ptr<TravelNetwork>& travelNetwork) {
        travelNetwork_ = travelNetwork;
    }

    // Notifiees
    NotifieeList& notifiees() {
        return notifiees_;
    }


protected:
    NotifieeList notifiees_;
    Ptr<TravelNetwork> travelNetwork_ = null;
    Ptr<Location> source_ = null;
    Ptr<Location> destination_ = null;
    Miles length_ = 0.0;

    explicit Segment(const string& name) : NamedInterface(name)
    {
        // Nothing else to do.
    }
    ~Segment() { }
};

/********************************************************
* Flight : Segment                                      *
********************************************************/
class Flight : public Segment {
public:
    /**
     * Return a new instance with the given name.
     */
    static Ptr<Flight> instanceNew(const string& name) {
        return new Flight(name);
    }

    // Source
    void sourceIs(const Ptr<Location>& source) {
        if (source != null && source->travelNetwork() != travelNetwork_) {
            string errorMessage = "Error in sourceIs(): Flight's source and destination should be in same travelNetwork as flight!";
            cerr << errorMessage << endl;
            throw fwk::DifferentNetworkException(errorMessage);
        }
        if (dynamic_cast<Airport*>(source.ptr()) != null) {
            if (source_ != null) {
                // Removing this source from existing source Location's segment list
                int segmentIndexInSourceList = 0; // 
                for (auto it = source_->segmentIter(); it != source_->segmentIterEnd(); ++it) {
                    if ((*it)->name() == this->name()) {
                        source_->segmentDel(segmentIndexInSourceList);
                    }
                }
            }
            if (source != null) {
                // Connect new source Location's Segment
                source->segmentNew(this);
            }
            source_ = source;
        } else {
            string errorMessage = "Error in sourceIs(): Flight's source and destination can only be of type Airport!";
            cerr << errorMessage << endl;
            throw fwk::InvalidArgumentExeption(errorMessage);
        }
    }
    
    // Destination
    void destinationIs(const Ptr<Location>& destination) {
        if (destination != null && destination->travelNetwork() != travelNetwork_) {
            string errorMessage = "Error in destinationIs(): Flight's source and destination should be in same travelNetwork as flight!";
            cerr << errorMessage << endl;
            throw fwk::DifferentNetworkException(errorMessage);
        }
        if (dynamic_cast<Airport*>(destination.ptr()) != null) {
            destination_ = destination;
        } else {
            string errorMessage = "Error in destinationIs(): Flight's source and destination can only be of type Airport!";
            cerr << errorMessage << endl;
            throw fwk::InvalidArgumentExeption(errorMessage);
        }
    }

protected:
    Flight(const string& name) :
        Segment(name)
    {
        // Nothing else to do.
    }
};


/********************************************************
* Road : Segment                                        *
********************************************************/

class Road : public Segment {
public:
    /**
     * Return a new instance with the given name.
     */
    static Ptr<Road> instanceNew(const string& name) {
        return new Road(name);
    }

protected:
    Road(const string& name) :
        Segment(name)
    {
        // Nothing else to do.
    }
};


/******************************************************************************
*******************************************************************************
******************************************************************************/

// A vehicle is a mode of transportation such as a plane or car. A vehicle has 
// attributes specifying the mean speed traveling along a segment, in miles per 
// hour, the capacity, in maximum number of passengers, and the cost of using the 
// vehicle to transport the maximum number of passengers, in dollars per mile.
class Vehicle : public NamedInterface {
public:
    class Notifiee : public BaseNotifiee<Vehicle> {
    public:
        void notifierIs(const Ptr<Vehicle>& vehicle) {
            connect(vehicle, this);
        }
    };

protected:
    typedef std::list<Notifiee*> NotifieeList;

public:

    // Remove the copy and assignment constructors
    Vehicle(const Vehicle&) = delete;
    void operator =(const Vehicle&) = delete;


    // Passenger Capacity
    Passengers capacity() {
        return capacity_;
    }
    void capacityIs(const Passengers capacity) {
        capacity_ = capacity;
    }

    // Speed
    MilesPerHour speed() {
        return speed_;
    }
    void speedIs(const MilesPerHour speed) {
        speed_ = speed;
    }

    // DollarsPerMile
    DollarsPerMile cost() {
        return cost_;
    }
    void costIs(const DollarsPerMile cost) {
        cost_ = cost;
    }

    // currLocation
    Ptr<Location> location() {
        return location_;
    }
    void locationIs(const Ptr<Location>& location) {
        location_ = location;
    }

    // travelNetwork
    Ptr<TravelNetwork> travelNetwork() {
        return travelNetwork_;
    }
    void travelNetworkIs(const Ptr<TravelNetwork>& travelNetwork) {
        travelNetwork_ = travelNetwork;
    }

    // Notifiees
    NotifieeList& notifiees() {
        return notifiees_;
    }


protected:
    NotifieeList notifiees_;
    Ptr<TravelNetwork> travelNetwork_ = null;

    Passengers capacity_ = 0;
    MilesPerHour speed_ = 0.0;
    DollarsPerMile cost_ = 0.0;
    Ptr<Location> location_ = null;

    explicit Vehicle(const string& name) : NamedInterface(name)
    {
        // Nothing else to do.
    }
    ~Vehicle() { }
};

/********************************************************
* Airplane : Vehicle                                      *
********************************************************/
class Airplane : public Vehicle {
public:
    /**
     * Return a new instance with the given name.
     */
    static Ptr<Airplane> instanceNew(const string& name) {
        return new Airplane(name);
    }

protected:
    Airplane(const string& name) :
        Vehicle(name)
    {
        // Nothing else to do.
    }
};


/********************************************************
* Car : Vehicle                                        *
********************************************************/

class Car : public Vehicle {
public:
    /**
     * Return a new instance with the given name.
     */
    static Ptr<Car> instanceNew(const string& name) {
        return new Car(name);
    }

protected:
    Car(const string& name) :
        Vehicle(name)
    {
        // Nothing else to do.
    }
};


/******************************************************************************
*******************************************************************************
******************************************************************************/
class Trip : public NamedInterface {
public:
    enum Status { waitingForVehicle, goingToPickup, goingToDropoff, droppedOff};

    class Notifiee : public BaseNotifiee<Trip> {
    public:
        void notifierIs(const Ptr<Trip>& trip) {
            connect(trip, this);
        }

        virtual void onStatus() { }
    };

protected:
    typedef std::list<Notifiee*> NotifieeList;

public:

    static Ptr<Trip> instanceNew(const string& name) {
        return new Trip(name);
    }

    // Remove the copy and assignment constructors
    Trip(const Trip&) = delete;
    void operator =(const Trip&) = delete;

    // Source
    Ptr<Location> startLocation() {
        return startLocation_;
    }

    void startLocationIs(const Ptr<Location>& startLocation) {
        startLocation_ = startLocation;
    }

    // Destination
    Ptr<Location> endLocation() {
        return endLocation_;
    }

    void endLocationIs(const Ptr<Location>& endLocation) {
        endLocation_ = endLocation;
    }

    Ptr<Vehicle> vehicle() {
        return vehicle_;
    }

    void vehicleIs(const Ptr<Vehicle>& vehicle) {
        if (vehicle != null && vehicle->travelNetwork() != travelNetwork_) {
            string errorMessage = "Error in vehicleIs(): Trip's vehicle and endVehicle should be in same travelNetwork as trip!";
            cerr << errorMessage << endl;
            throw fwk::DifferentNetworkException(errorMessage);
        }
        vehicle_ = vehicle;
    }

    // Length
    Passengers numTravelers() {
        return numTravelers_;
    }

    void numTravelersIs(const Passengers numTravelers) {
        numTravelers_ = numTravelers;
    }

    // travelNetwork
    Ptr<TravelNetwork> travelNetwork() {
        return travelNetwork_;
    }
    void travelNetworkIs(const Ptr<TravelNetwork>& travelNetwork) {
        travelNetwork_ = travelNetwork;
    }

    // status
    Status status() {
        return status_;
    }
    void statusIs(const Status status) {
        if (status > status_) {
            status_ = status;
            post(this, &Notifiee::onStatus);
        } else {
            string errorMessage = "Error in statusIs(): Trip's current status is already farther along than the new status to be assigned!";
            cerr << errorMessage << endl;
            throw fwk::InvalidArgumentExeption(errorMessage);
        }
    }

    // waitTime
    Time waitTime() {
        return waitTime_;
    }
    void waitTimeIs(const Time waitTime) {
        waitTime_ = waitTime;
    }

    // path
    vector<Ptr<Segment>> path() {
        return path_;
    }
    void pathIs(vector<Ptr<Segment>> path) {
        path_ = path;
    }

    // Notifiees
    NotifieeList& notifiees() {
        return notifiees_;
    }


protected:
    NotifieeList notifiees_;
    Ptr<TravelNetwork> travelNetwork_ = null;
    Ptr<Location> startLocation_ = null;
    Ptr<Location> endLocation_ = null;
    Ptr<Vehicle> vehicle_ = null;
    vector<Ptr<Segment>> path_;
    Passengers numTravelers_ = 0;
    Status status_;
    Time waitTime_ = 0; // Confirmed with Prof. Linton on 12/3/2014 that we could set this to 0 and return it in the accessor

    explicit Trip(const string& name) : NamedInterface(name), status_(waitingForVehicle)
    {
        // Nothing else to do.
    }
    ~Trip() { }
};

/******************************************************************************
*******************************************************************************
******************************************************************************/
class Stats; // forward-declared in order to include it in network
class Conn; // forward-declared in order to include it in network

// Our TravelNetwork class contains collections of all of our travel entities.
class TravelNetwork : public NamedInterface {
public:
    class Notifiee : public BaseNotifiee<TravelNetwork> {
    public:
        void notifierIs(const Ptr<TravelNetwork>& tn) {
            connect(tn, this);
        }

        virtual void onLocationNew(const Ptr<Location>& location) { }
        virtual void onLocationDel(const Ptr<Location>& location) { }
        virtual void onSegmentNew(const Ptr<Segment>& segment) { }
        virtual void onSegmentDel(const Ptr<Segment>& segment) { }
        virtual void onTripNew(const Ptr<Trip>& trip) { }
        virtual void onTripDel(const Ptr<Trip>& trip) { }
        virtual void onVehicleNew(const Ptr<Vehicle>& vehicle) { }
        virtual void onVehicleDel(const Ptr<Vehicle>& vehicle) { }
    };

protected:
    typedef std::unordered_map< string, Ptr<Location> > LocationMap;
    typedef std::unordered_map< string, Ptr<Segment> > SegmentMap;
    typedef std::unordered_map< string, Ptr<Vehicle> > VehicleMap;
    typedef std::unordered_map< string, Ptr<Trip> > TripMap;
    typedef std::list<Notifiee*> NotifieeList;

    LocationMap locationMap_;
    SegmentMap segmentMap_;
    VehicleMap vehicleMap_;
    TripMap tripMap_;
    Ptr<Stats> stats_;
    Ptr<Conn> conn_;

    NotifieeList notifiees_;

    explicit TravelNetwork(const string& name) : NamedInterface(name)
    {
        // Nothing left to do.
    }
    ~TravelNetwork() { }

public:

    // Remove the copy and assignment constructors
    TravelNetwork(const TravelNetwork&) = delete;
    void operator =(const TravelNetwork&) = delete;


    /********************************************************
    * Location Operations                                   *
    ********************************************************/
    Ptr<Location> location(const string& name) {
        const auto i = locationMap_.find(name);
        if (i != locationMap_.end()) {
            return i->second;
        }
        cerr << "Error in location: Could not find a location by the name (" << name << "). Returning null." << endl;
        return null;
    }

    void locationNew(const Ptr<Location>& location) {
        const string name = location->name();
        if (! (locationMap_.insert(LocationMap::value_type(name, location)).second) ) {
            cerr << "Error in locationNew: The location name (" << name << ") is already in use. Throwing exception." << endl;
            throw fwk::NameInUseException(name);
        }
        location->travelNetworkIs(this);
        post(this, &Notifiee::onLocationNew, location);
    }

    Ptr<Location> locationDel(const string& name) {
        const auto iter = locationMap_.find(name);
        if (iter == locationMap_.end()) {
            cerr << "Error in locationDel: Could not find a location by the name (" << name << "). Returning null." << endl;
            return null;
        }
        auto location = iter->second;
        locationDel(iter);
        return location;
    }

    LocationMap::iterator locationDel(LocationMap::const_iterator iter) {
        const auto location = iter->second;
        for (auto it = location->segmentIter(); it != location->segmentIterEnd(); ++it) {
            (*it)->sourceIs(null);
        }
        const auto next = locationMap_.erase(iter);
        location->travelNetworkIs(null);
        post(this, &Notifiee::onLocationDel, location);
        return next;
    }

    /********************************************************
    * Segment Operations                                   *
    ********************************************************/
    Ptr<Segment> segment(const string& name) {
        const auto i = segmentMap_.find(name);
        if (i != segmentMap_.end()) {
            return i->second;
        }
        cerr << "Error in segment(): Could not find a segment by the name (" << name << "). Returning null." << endl;
        return null;
    }

    void segmentNew(const Ptr<Segment>& segment) {
        const string name = segment->name();
        if (! (segmentMap_.insert(SegmentMap::value_type(name, segment)).second) ) {
            cerr << "Error in segmentNew: The segment name (" << name << ") is already in use. Throwing exception." << endl;
            throw fwk::NameInUseException(name);
        }
        segment->travelNetworkIs(this);
        post(this, &Notifiee::onSegmentNew, segment);
    }

    Ptr<Segment> segmentDel(const string& name) {
        const auto iter = segmentMap_.find(name);
        if (iter == segmentMap_.end()) {
            cerr << "Error in segmentDel: Could not find a segment by the name (" << name << "). Returning null." << endl;
            return null;
        }
        auto segment = iter->second;
        segmentDel(iter);
        return segment;
    }

    SegmentMap::iterator segmentDel(SegmentMap::const_iterator iter) {
        const auto segment = iter->second;
        const auto next = segmentMap_.erase(iter);
        segment->sourceIs(null);
        segment->destinationIs(null);
        segment->travelNetworkIs(null);
        post(this, &Notifiee::onSegmentDel, segment);
        return next;
    }

    /********************************************************
    * Trip Operations                                       *
    ********************************************************/
    Ptr<Trip> trip(const string& name) {
        const auto i = tripMap_.find(name);
        if (i != tripMap_.end()) {
            return i->second;
        }
        cerr << "Error in trip(): Could not find a trip by the name (" << name << "). Returning null." << endl;
        return null;
    }

    void tripNew(const Ptr<Trip>& trip) {
        const string name = trip->name();
        if (! (tripMap_.insert(TripMap::value_type(name, trip)).second) ) {
            cerr << "Error in tripNew: The trip name (" << name << ") is already in use. Throwing exception." << endl;
            throw fwk::NameInUseException(name);
        }
        trip->travelNetworkIs(this);
        post(this, &Notifiee::onTripNew, trip);
    }

    Ptr<Trip> tripDel(const string& name) {
        const auto iter = tripMap_.find(name);
        if (iter == tripMap_.end()) {
            cerr << "Error in tripDel: Could not find a trip by the name (" << name << "). Returning null." << endl;
            return null;
        }
        auto trip = iter->second;
        tripDel(iter);
        return trip;
    }

    TripMap::iterator tripDel(TripMap::const_iterator iter) {
        const auto trip = iter->second;
        const auto next = tripMap_.erase(iter);
        trip->startLocationIs(null);
        trip->endLocationIs(null);
        trip->travelNetworkIs(null);
        post(this, &Notifiee::onTripDel, trip);
        return next;
    }

    /********************************************************
    * Vehicle Operations                                    *
    ********************************************************/
    Ptr<Vehicle> vehicle(const string& name) {
        const auto i = vehicleMap_.find(name);
        if (i != vehicleMap_.end()) {
            return i->second;
        }
        cerr << "Error in vehicle(): Could not find a vehicle by the name (" << name << "). Returning null." << endl;
        return null;
    }

    void vehicleNew(const Ptr<Vehicle>& vehicle) {
        const string name = vehicle->name();
        if (! (vehicleMap_.insert(VehicleMap::value_type(name, vehicle)).second) ) {
            cerr << "Error in vehicleNew(): The vehicle name (" << name << ") is already in use. Throwing exception." << endl;
            throw fwk::NameInUseException(name);
        }
        vehicle->travelNetworkIs(this);
        post(this, &Notifiee::onVehicleNew, vehicle);
    }

    Ptr<Vehicle> vehicleDel(const string& name) {
        const auto iter = vehicleMap_.find(name);
        if (iter == vehicleMap_.end()) {
            cerr << "Error in vehicleDel(): Could not find a vehicle by the name (" << name << "). Returning null." << endl;
            return null;
        }
        auto vehicle = iter->second;
        vehicleDel(iter);
        return vehicle;
    }

    VehicleMap::iterator vehicleDel(VehicleMap::const_iterator iter) {
        const auto vehicle = iter->second;
        const auto next = vehicleMap_.erase(iter);
        vehicle->travelNetworkIs(null);
        vehicle->locationIs(null);
        post(this, &Notifiee::onVehicleDel, vehicle);
        return next;
    }

    /********************************************************
    * Stats Operations                                   *
    ********************************************************/
    Ptr<Stats> stats(const string& name);

    /********************************************************
    * Conn Operations                                       *
    ********************************************************/
    Ptr<Conn> conn(const string& name);

    /********************************************************
    * Notifiee Operations                                   *
    ********************************************************/
    NotifieeList& notifiees() {
        return notifiees_;
    }

    static Ptr<TravelNetwork> instanceNew(string name);
};

/******************************************************************************
*******************************************************************************
******************************************************************************/
// Statistics: Given a transporation entity type, return how many entities of 
// that type are present in the network. You are required to collect this 
// information using the notification-reactor model.
class Stats : public NamedInterface {
public:
    class Notifiee : public BaseNotifiee<Stats> {
    public:
        // void notifierIs(const Ptr<Stats>& stats) {
        //     connect(stats, this);
        // }
    };

protected:
    typedef std::list<Notifiee*> NotifieeList;
    NotifieeList notifiees_;

    
    /********************************************************
    * Nest TravelNetworkTracker in Stats                    *
    ********************************************************/
    class TravelNetworkTracker : public TravelNetwork::Notifiee {
    public:
        static Ptr<TravelNetworkTracker> instanceNew(const Ptr<TravelNetwork>& tn) { 
            const Ptr<TravelNetworkTracker> tnt = new TravelNetworkTracker(); 
            tnt->notifierIs(tn);
            return tnt;
        }

        /********************************************************
        * Reactor Functions                                     *
        ********************************************************/
        /** Notification that a location is added to the network. */
        void onLocationNew(const Ptr<Location>& location) {
            if (location == null) {
                cerr << "Unable to new null location!" << endl;
                return;
            }

            // Check if location is a Residence
            if (dynamic_cast<Residence*>(location.ptr()) != null) {
                stats_->numResidences_++;
            }

            // Check if location is a Airport
            if (dynamic_cast<Airport*>(location.ptr()) != null) {
                stats_->numAirports_++;
            }
        }

        /** Notification that a location is removed from the network. */
        void onLocationDel(const Ptr<Location>& location) {
            // Check if location is a Residence
            if (dynamic_cast<Residence*>(location.ptr()) != null) {
                stats_->numResidences_--;
            }

            // Check if location is a Airport
            if (dynamic_cast<Airport*>(location.ptr()) != null) {
                stats_->numAirports_--;
            }
        }

        /** Notification that a segment is added to the network. */
        void onSegmentNew(const Ptr<Segment>& segment) {
            // Check if segment is a Road
            if (dynamic_cast<Road*>(segment.ptr()) != null) {
                stats_->numRoads_++;
            }

            // Check if segment is a Flight
            if (dynamic_cast<Flight*>(segment.ptr()) != null) {
                stats_->numFlights_++;
            }
        }

        /** Notification that a segment is removed from the network. */
        void onSegmentDel(const Ptr<Segment>& segment) {
            // Check if segment is a Road
            if (dynamic_cast<Road*>(segment.ptr()) != null) {
                stats_->numRoads_--;
            }

            // Check if segment is a Flight
            if (dynamic_cast<Flight*>(segment.ptr()) != null) {
                stats_->numFlights_--;
            }
        }

        /** Notification that a vehicle is added to the network. */
        void onVehicleNew(const Ptr<Vehicle>& vehicle) {
            // Check if vehicle is a Car
            if (dynamic_cast<Car*>(vehicle.ptr()) != null) {
                stats_->numCars_++;
            }

            // Check if vehicle is a Airplane
            if (dynamic_cast<Airplane*>(vehicle.ptr()) != null) {
                stats_->numAirplanes_++;
            }
        }

        /** Notification that a vehicle is removed from the network. */
        void onVehicleDel(const Ptr<Vehicle>& vehicle) {
            // Check if vehicle is a Car
            if (dynamic_cast<Car*>(vehicle.ptr()) != null) {
                stats_->numCars_--;
            }

            // Check if vehicle is a Airplane
            if (dynamic_cast<Airplane*>(vehicle.ptr()) != null) {
                stats_->numAirplanes_--;
            }
        }

        /** Notification that a trip is added to the network. */
        void onTripNew(const Ptr<Trip>& trip) {
            stats_->numTrips_++;

            // Create a new trip tracker for each new trip
            auto tripTracker = TripTracker::instanceNew(trip);
            stats_->tripTrackerMap_[trip->name()] = tripTracker;
            tripTracker->stats_ = stats_; // from slide 28 in lecture3.pdf
        }

        /** Notification that a trip is removed from the network. */
        void onTripDel(const Ptr<Trip>& trip) {
            stats_->numTrips_--;
            stats_->tripTrackerMap_.erase(trip->name());
        }
        
        // We can make this public because it's only available to the stats class.
        Ptr<Stats> stats_;
    };

    /********************************************************
    * Nest TripTracker in Stats                             *
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
                stats_->numCompletedTrips_++;
            };
            if (notifier()->status() == Trip::goingToDropoff) {
                stats_->numPickups_++;
                stats_->cumWaitTime_ += notifier()->waitTime();
            }
        }

        // We can make this public because it's only available to the stats class.
        Ptr<Stats> stats_;
    };

    unsigned int numResidences_ = 0;
    unsigned int numAirports_ = 0;
    unsigned int numFlights_ = 0;
    unsigned int numRoads_ = 0;
    unsigned int numTrips_ = 0;
    unsigned int numCompletedTrips_ = 0;
    unsigned int numPickups_ = 0;
    unsigned int numCars_ = 0;
    unsigned int numAirplanes_ = 0;
    Time cumWaitTime_ = 0;

    Ptr<TravelNetworkTracker> travelNetworkTracker_;
    typedef unordered_map< string, Ptr<TripTracker> > TripTrackerMap;
    TripTrackerMap tripTrackerMap_;

    explicit Stats(const string& name) : NamedInterface(name)
    {
        // Nothing else to do.
    }
    ~Stats() {

    }


public:
    static Ptr<Stats> instanceNew(string name, Ptr<TravelNetwork> tn) {
        Ptr<Stats> s = new Stats(name);
        s->travelNetworkTracker_ = TravelNetworkTracker::instanceNew(tn);
        s->travelNetworkTracker_->stats_ = s;
        return s;
    }

    // Remove the copy and assignment constructors
    Stats(const Stats&) = delete;
    void operator =(const Stats&) = delete;

    // Notifiees
    NotifieeList& notifiees() {
        return notifiees_;
    }

    /********************************************************
    * Accessor Functions                                    *
    ********************************************************/
    unsigned int numResidences() {
        return numResidences_;
    }

    unsigned int numAirports() {
        return numAirports_;
    }

    unsigned int numFlights() {
        return numFlights_;
    }

    unsigned int numRoads() {
        return numRoads_;
    }

    unsigned int numTrips() {
        return numTrips_;
    }

    unsigned int numPickups() {
        return numPickups_;
    }

    unsigned int numAirplanes() {
        return numAirplanes_;
    }

    unsigned int numCars() {
        return numCars_;
    }

    unsigned int numCompletedTrips() {
        return numCompletedTrips_;
    }

    Time averageWaitTime() {
        if (numPickups_ == 0) return 0;
        return cumWaitTime_.value() / numPickups_;
    }
};


/******************************************************************************
*******************************************************************************
******************************************************************************/
// Connectivity: Given a starting location and a distance, return all paths of that 
// distance or less. For example, a connectivity query on starting location "sfo" 
// with distance "100" should return all paths whose end locations are 100 miles 
// or less away from the sfo location.
class Conn : public NamedInterface {
public:
    class Notifiee : public BaseNotifiee<Conn> {
    public:
        // Defined just in case.
    };

protected:
    /********************************************************
    * Nest TravelNetworkTracker in Conn                    *
    ********************************************************/
    class TravelNetworkTracker : public TravelNetwork::Notifiee {
    public:
        static Ptr<TravelNetworkTracker> instanceNew(const Ptr<TravelNetwork>& tn) { 
            const Ptr<TravelNetworkTracker> tnt = new TravelNetworkTracker(); 
            tnt->notifierIs(tn);
            return tnt;
        }

        /********************************************************
        * Reactor Functions                                     *
        ********************************************************/

        /** Notification that a segment is added to the network. */
        void onSegmentNew(const Ptr<Segment>& segment) {
            // Check if segment is a Road
            if (dynamic_cast<Road*>(segment.ptr()) != null) {
                cout << "conn_->numRoads_++;" << endl; // TODO
            }

            // Check if segment is a Flight
            if (dynamic_cast<Flight*>(segment.ptr()) != null) {
                cout << "conn_->numFlights_++;" << endl; // TODO
            }
        }

        /** Notification that a segment is removed from the network. */
        void onSegmentDel(const Ptr<Segment>& segment) {
            // Check if segment is a Road
            if (dynamic_cast<Road*>(segment.ptr()) != null) {
                cout << "conn_->numRoads_--;" << endl; // TODO
            }

            // Check if segment is a Flight
            if (dynamic_cast<Flight*>(segment.ptr()) != null) {
                cout << "conn_->numFlights_--;" << endl; // TODO
            }
        }
        // We can make this public because it's only available to the conn_ class.
        Ptr<Conn> conn_;
    };

    typedef std::list<Notifiee*> NotifieeList;
    NotifieeList notifiees_;
    Ptr<TravelNetwork> travelNetwork_;
    size_t cacheSize = 20;
    Cache<string, pair<vector<Ptr<Segment>>,double>> cache_ = Cache<string, pair<vector<Ptr<Segment>>,double>>(cacheSize);
    unsigned int numCacheHits_ = 0;
    unsigned int numCacheChecks_ = 0;
    bool startedAtLeastOneTrip = false; // Tracks whether we've ran our first trip so we can keep our candidate list of new segments and deleted segments

    Ptr<TravelNetworkTracker> travelNetworkTracker_;
    vector<Ptr<Segment>> newSegments_;
    vector<Ptr<Segment>> deletedSegments_;

    explicit Conn(const string& name) : NamedInterface(name)
    {
        // Nothing else to do.
    }
    ~Conn() {
    }

    void recDFSWithinMaxDistance(Ptr<Location> & currLocation, Miles distanceSoFar, Miles maxDistance, string pathSoFar, set < string > visitedLocations, vector<string>& pathsWithinMaxDistance) {
        visitedLocations.insert(currLocation->name());
        for (auto it = currLocation->segmentIter() ; it != currLocation->segmentIterEnd(); ++it) {
            Ptr<Segment> currSegment = *it;
            if (currSegment->source() == null || currSegment->destination() == null) {
                // Skip segment if we have an invalid destination (perhaps from failed initialization)
                continue;
            }

            Ptr<Location> nextLocation = currSegment->destination();
            if (distanceSoFar + currSegment->length() <= maxDistance) {
                if (visitedLocations.find(nextLocation->name()) != visitedLocations.end()) {
                    // Skip this destination if you've already visited this node in this path
                    continue;
                }
                Miles newDistance = (distanceSoFar + currSegment->length());
                string newPathSoFar = (pathSoFar + "(" + currSegment->name() + ":" + std::to_string(currSegment->length().value()) + ") " + nextLocation->name());
                pathsWithinMaxDistance.push_back(newPathSoFar);
                recDFSWithinMaxDistance(nextLocation, newDistance, maxDistance, newPathSoFar, visitedLocations, pathsWithinMaxDistance);
            }
        }
    }

    void recDFSForShortestPath(const Ptr<Location> & currLocation, const Ptr<Location> & destination,
                            Miles distanceSoFar, vector<Ptr<Segment>>& pathSoFar, 
                            unordered_map<string, double>& visitedLocationDistances,
                            unordered_map<string, vector<Ptr<Segment>>>& visitedLocationShortestPaths) {
        visitedLocationDistances[currLocation->name()] = distanceSoFar.value();
        visitedLocationShortestPaths[currLocation->name()] = pathSoFar;

        for (auto it = currLocation->segmentIter(); it != currLocation->segmentIterEnd(); ++it) {
            Ptr<Segment> currSegment = *it;
            if (currSegment->source() == null || currSegment->destination() == null) {
                // Skip segment if we have an invalid destination (perhaps from failed initialization)
                continue;
            }

            Ptr<Location> nextLocation = currSegment->destination();
            if (visitedLocationDistances.find(nextLocation->name()) == visitedLocationDistances.end()) {
                visitedLocationDistances[nextLocation->name()] = numeric_limits<double>::max();
            }
            // cout << currLocation->name() << "->" << nextLocation->name() << " via " << currSegment->name();
            Miles newDistance = (distanceSoFar + currSegment->length());
            if (newDistance.value() < visitedLocationDistances[nextLocation->name()]) {
                if (visitedLocationDistances.find(destination->name()) != visitedLocationDistances.end() &&
                    newDistance >= visitedLocationDistances[destination->name()]) {
                    // Skip this destination if you've already visited this node in this path
                    // cout << " [Discarded this segment we're past the limit for the goal because distanceSoFar " << newDistance.value() << " >= " << visitedLocationDistances[destination->name()] << " ] " << endl;
                    continue;
                }
                visitedLocationDistances[nextLocation->name()] = newDistance.value();
                // cout << " [Set the distance to " << newDistance.value() << "]" << endl;
                vector<Ptr<Segment>> newPathSoFar = pathSoFar;
                newPathSoFar.push_back(currSegment);
                visitedLocationShortestPaths[nextLocation->name()] = newPathSoFar;

                recDFSForShortestPath(nextLocation, destination, newDistance, newPathSoFar, visitedLocationDistances, visitedLocationShortestPaths);
            } 
            // else {
            //     cout << " [Discarded this segment because newDistance " << newDistance.value() << " >= old value of this nextLocation " << visitedLocationDistances[nextLocation->name()] << " ] " << endl;
            // }
        }
    }

public:
    static Ptr<Conn> instanceNew(string name, Ptr<TravelNetwork> tn) {
        Ptr<Conn> c = new Conn(name);
        c->travelNetwork_ = tn;
        c->travelNetworkTracker_ = TravelNetworkTracker::instanceNew(tn);
        return c;
    }

    // Remove the copy and assignment constructors
    Conn(const Conn&) = delete;
    void operator =(const Conn&) = delete;

    string processQuery(const string& startLocName, const Miles& maxDistance) {
        vector<string> resultingValidPaths;
        Ptr<Location> startLocation = travelNetwork_->location(startLocName);

        if (startLocation != null) {
            set < string > visitedLocations;
            string pathSoFar = startLocation->name();
            Miles startingDistance = 0;
            recDFSWithinMaxDistance(startLocation, startingDistance, maxDistance, pathSoFar, visitedLocations, resultingValidPaths);
        } else {
            cerr << "Could not find the starting location (" << startLocName << "). Returning empty string as results." << endl;
            return "";
        }

        string results = "";
        for (string validPath: resultingValidPaths) {
            results += (validPath + "\n");
        }
        return results;
    }

    pair<vector<Ptr<Segment>>, double> findShortestPath(const Ptr<Location>& source, const Ptr<Location>& destination) {
        if (source->name() == destination->name()) {
            cout << "returned 0 path" << endl;
            return make_pair(vector<Ptr<Segment>>(), 0);
        }
        string key = source->name() + "->" + destination->name();
        pair<vector<Ptr<Segment>>, double> pathDistPair;
        startedAtLeastOneTrip = true;
        numCacheChecks_++;
        if (cache_.containsCacheEntry(key)) {
            numCacheHits_++;
            pathDistPair = cache_.cacheEntry(key);
            auto shortestPath = pathDistPair.first;
            auto shortestPathDistance = pathDistPair.second;
            cout << "cacheHit!!! (<" << source->name() << destination->name() << ">, < " << ", " << shortestPathDistance << ">)" << endl;
            cout << pathDistPair.first.size() << endl;
            return pathDistPair;
        }
        vector<Ptr<Segment>> currPath;
        unordered_map<string, double> visitedLocationDistances;
        unordered_map<string, vector<Ptr<Segment>>> visitedLocationShortestPaths;
        recDFSForShortestPath( source, destination, 0, currPath, 
                                visitedLocationDistances, visitedLocationShortestPaths);
        vector<Ptr<Segment>> shortestPath = visitedLocationShortestPaths[destination->name()];
        cout << "shortestPath.size() =" << shortestPath.size() << endl;
        double shortestPathDistance = visitedLocationDistances[destination->name()];
        
        // cout << "Distance from " << source->name() << " to " << destination->name() << " is " << shortestPathDistance << ". Path is: " << endl;;
        // for (unsigned int i = 0; i < shortestPath.size(); i++) {
        //     cout << "\t" << shortestPath[i]->source()->name() << " -> " << shortestPath[i]->destination()->name() << " : " << shortestPath[i]->length().value() << "\n";
        // }
        pathDistPair = make_pair(shortestPath, shortestPathDistance);
        cache_.cacheEntryIs(key, pathDistPair);
        // cout << "Inserting (<" << source->name() << destination->name() << ">, < shortestPath starting at " << shortestPath[0]->source()->name() << ", " << shortestPathDistance << ">)" << endl;
        return pathDistPair;
    }

    unsigned int numCacheHits() {
        return numCacheHits_;
    }

    unsigned int numCacheChecks() {
        return numCacheChecks_;
    }

    double cacheEfficiency() {
        return double(numCacheHits_) / numCacheChecks_;
    }

    // Notifiees
    NotifieeList& notifiees() {
        return notifiees_;
    }
};

/******************************************************************************
*******************************************************************************
******************************************************************************/


Ptr<Stats> TravelNetwork::stats(const string& name) { 
    // Return the same one if it exists, regardless of the name
    if (stats_ == null) {
        stats_ = Stats::instanceNew(name, this);
    }
    return stats_;
}


Ptr<Conn> TravelNetwork::conn(const string& name) { 
    // Return the same one if it exists, regardless of the name
    if (conn_ == null) {
        conn_ = Conn::instanceNew(name, this);
    }
    return conn_;
}


Ptr<TravelNetwork> TravelNetwork::instanceNew(string name) {
    Ptr<TravelNetwork> tn = new TravelNetwork(name);
    tn->stats("This is an arbitrarily chosen stats object name so that we initiate our stats instance as soon as we create our TravelNetwork.");
    tn->conn("This is an arbitrarily chosen stats object name so that we initiate our conn instance as soon as we create our TravelNetwork.");
    return tn;
}

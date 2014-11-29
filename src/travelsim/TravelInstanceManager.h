#ifndef TRAVELSIM_TRAVELINSTANCEMANAGER_H
#define TRAVELSIM_TRAVELINSTANCEMANAGER_H

#include "InstanceManager.h"
#include "TravelNetwork.h"

#include <sstream>
#include <vector>
#include <ostream>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

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

/******************************************************************************
*******************************************************************************
******************************************************************************/

    // A location is a place where passenger travel starts or ends, or an 
    // intermediary point along the way. Some intermediate locations allow 
    // passengers to switch to a different mode of transportation for the next 
    // portion of their trip. For example, an airport is a location where 
    // passengers might switch from a car to a plane, or switch from one plane 
    // to another.
    class LocationInstance : public Instance {
        // Public variables and functions of LocationInstance
        public:
            static Ptr<LocationInstance> instanceNew(const string& name, Ptr<Location> lp) {
                Ptr<LocationInstance> lip = new LocationInstance(name);
                lip->location_ = lp;
                return lip;
            }

            _noinline
            string attribute(const string& name) {
                int segmentNum = segmentNumber(name);
                if (segmentNum == 0) {
                    cerr << "Error: Since we couldn't convert into a segment num, we are returning empty string in LocationInstance::attribute" << endl;
                    return "";
                }
                const int i = segmentNum - 1;
                string segment = location_->segment(i)->name();
                return segment;
            }

            _noinline
            void attributeIs(const string& name, const string& value) {
                cerr << "Error! You cannot set the attribute of a Location object. " << endl;
                // Ignore.
            }

            _noinline
            Ptr<Location> location() {
                return location_;
            }

        // Private variables and functions of LocationInstance
        private: 
            friend class TravelInstanceManager;
            static const string segmentStr;
            static const unsigned long segmentStrlen;
            Ptr<Location> location_;
        
        // Protected variables and functions of LocationInstance
        protected:
            LocationInstance(const string& name) : Instance(name) {
                // Nothing else to do.
            }

            _noinline
            int segmentNumber(const string& name) {
                if (name.substr(0, segmentStrlen) == segmentStr) {
                    int digitsStringLen = name.length() - segmentStrlen;
                    if (digitsStringLen < 1) {
                        cerr << "Error: Need to have at least one character after the \"segment\" portion of the segmentNum. Your input was: " << name << endl;
                        return 0;
                    }
                    const char *digitsPortionOfSegmentStr = name.c_str() + segmentStrlen;
                    for (int i = 0; i < digitsStringLen; i++) {
                        if (!isdigit(digitsPortionOfSegmentStr[i])) {
                            cerr << "Error: Need to have only digits after the \"segment\" portion so that segmentNumber() returns a positive integer. Your input was: " << name << endl;
                            return 0;
                        }
                    }

                    int segmentNum;
                    try
                    {
                        segmentNum = std::stoi(name.c_str() + segmentStrlen);
                    }
                    catch(std::invalid_argument& e) //or catch(...) to catch all exceptions
                    {
                        cerr << "Exception caught! Unable to convert (" << name << ") to int!" << endl;
                        return 0;
                    }
                    return segmentNum;
                }
                return 0;
            }
    };

/******************************************************************************
*******************************************************************************
******************************************************************************/

    // A segment connects one location to another. In addition to the starting 
    // and ending locations, segments have a length attribute, which is the 
    // mileage from start to end.
    class SegmentInstance : public Instance {
        public:
            static Ptr<SegmentInstance> instanceNew(const string& name, Ptr<Segment> sp) {
                Ptr<SegmentInstance> sip = new SegmentInstance(name);
                sip->segment_ = sp;
                return sip;
            }

            _noinline
            string attribute(const string& name) {
                if (name == "source") {
                    return segment_->source()->name();
                } else if (name == "destination") {
                    return segment_->destination()->name();
                } else if (name == "length") {
                    return std::to_string(segment_->length().value());
                }
                cerr << "Error in SegmentInstance:attribute(): Must specify attribute name of SegmentInstance as source, destination or cost. Currently, the value is: " << name << endl;
                return "";
            }

            _noinline
            void attributeIs(const string& name, const string& value) {
                if (name == "source") {
                    Ptr<LocationInstance> locationInstancePtr = dynamic_cast<LocationInstance*>(instanceManager_->instance(value).ptr());
                    if (locationInstancePtr == null) {
                        cerr << "There is no instance corresponding to " << value << " or it was unable to be dynamic casted to locationInstance" << endl;
                        return;
                    }
                    Ptr<Location> locationPtr = locationInstancePtr->location();
                    segment_->sourceIs(locationPtr);
                } else if (name == "destination") {
                    Ptr<LocationInstance> locationInstancePtr = dynamic_cast<LocationInstance*>(instanceManager_->instance(value).ptr());
                    if (locationInstancePtr == null) {
                        cerr << "There is no instance corresponding to " << value << " or it was unable to be dynamic casted to locationInstance" << endl;
                        return;
                    }
                    Ptr<Location> locationPtr = locationInstancePtr->location();
                    segment_->destinationIs(locationPtr);
                } else if (name == "length") {
                    try
                    {
                        segment_->lengthIs(std::stoi(value));
                    }
                    catch(std::invalid_argument& e)
                    
                    {
                        cerr << "Exception caught! Unable to convert (" << value << ") to int!" << endl;
                    }
                    
                } else {
                    cerr << "Error in SegmentInstance:attributeIs(): Must specify attribute name of SegmentInstance as source, destination or length. Currently, the value is: " << name << endl;
                }
            }

            _noinline
            Ptr<Segment> segment() {
                return segment_;
            }

        // Private variables and functions of SegmentInstance
        private:
            friend class TravelInstanceManager;
            Ptr<Segment> segment_;

        // Protected variables and functions of SegmentInstance
        protected:
            SegmentInstance(const string& name) : Instance(name) {
                // Nothing else to do.
            }
    };

/******************************************************************************
*******************************************************************************
******************************************************************************/

    // A vehicle is a mode of transportation such as a plane or car. A vehicle 
    // has attributes specifying the mean speed traveling along a segment, in 
    // miles per hour, the capacity, in maximum number of passengers, and the 
    // cost of using the vehicle to transport the maximum number of passengers, 
    // in dollars per mile.
    class VehicleInstance : public Instance {
        public:
            static Ptr<VehicleInstance> instanceNew(const string& name, Ptr<Vehicle> vp) {
                Ptr<VehicleInstance> vip = new VehicleInstance(name);
                vip->vehicle_ = vp;
                return vip;
            }

            _noinline
            string attribute(const string& name) {
                if (name == "capacity") {
                    return std::to_string(vehicle_->capacity().value());
                } else if (name == "speed") {
                    return std::to_string(vehicle_->speed().value());
                } else if (name == "cost") {
                    return std::to_string(vehicle_->cost().value());
                }
                cerr << "Error in VehicleInstance:attribute(): Must specify attribute name of VehicleInstance as capacity, speed or cost. Currently, the value is: " << name << endl;
                return "";
            }

            _noinline
            void attributeIs(const string& name, const string& value) {
                if (name == "capacity") {
                    try
                    {
                        vehicle_->capacityIs(std::stoi(value));
                    }
                    catch(std::invalid_argument& e) //or catch(...) to catch all exceptions
                    
                    {
                        cerr << "Exception caught! Unable to convert (" << value << ") to int!" << endl;
                    }
                } else if (name == "speed") {
                    try
                    {
                        vehicle_->speedIs(std::stod(value));
                    }
                    catch(std::invalid_argument& e) //or catch(...) to catch all exceptions
                    
                    {
                        cerr << "Exception caught! Unable to convert (" << value << ") to int!" << endl;
                    }
                } else if (name == "cost") {
                    try
                    {
                        vehicle_->costIs(std::stod(value));
                    }
                    catch(std::invalid_argument& e) //or catch(...) to catch all exceptions
                    {
                        cerr << "Exception caught! Unable to convert (" << value << ") to int!" << endl;
                    }
                } else {
                    cerr << "Error in SegmentInstance:attributeIs(): Must specify attribute name of SegmentInstance as source, destination or length. Currently, the value is: " << name << endl;
                }
            }

            _noinline
            Ptr<Vehicle> vehicle() {
                return vehicle_;
            }

        // Private variables and functions of SegmentInstance
        private:
            friend class TravelInstanceManager;
            Ptr<Vehicle> vehicle_;

        // Protected variables and functions of SegmentInstance
        protected:
            VehicleInstance(const string& name) : Instance(name) {
                // Nothing else to do.
            }
    };

/******************************************************************************
*******************************************************************************
******************************************************************************/

    // Statistics: Given a transporation entity type, return how many entities  
    // of that type are present in the network. You are required to collect this 
    // information using the notification-reactor model.
    class StatsInstance : public Instance {
        public:
            static Ptr<StatsInstance> instanceNew(const string& name, Ptr<Stats> sp) {
                Ptr<StatsInstance> sip = new StatsInstance(name);
                sip->stats_ = sp;
                return sip;
            }

            _noinline
            string attribute(const string& name) {
                if (name == "Residence") {
                    return std::to_string(stats_->numResidences());
                } else if (name == "Airport") {
                    return std::to_string(stats_->numAirports());
                } else if (name == "Road") {
                    return std::to_string(stats_->numRoads());
                } else if (name == "Flight") {
                    return std::to_string(stats_->numFlights());
                }
                cerr << "Error in StatsInstance:attribute(): Must specify one of the following attribute names: Residence, Airport, Road, Flight. The erroneous specified value was: " << name << endl;
                return "";
            }

            _noinline
            void attributeIs(const string& name, const string& value) {
                cerr << "Error! You cannot set the attribute of a Stats object. We track the statistics via notification-reactor model." << endl;
            }

        // Private variables and functions of SegmentInstance
        private:
            friend class TravelInstanceManager;
            StatsInstance(const string& name) : Instance(name) {
                // Nothing else to do.
            }
            Ptr<Stats> stats_;
    };

/******************************************************************************
*******************************************************************************
******************************************************************************/

    // Connectivity: Given a starting location and a distance, return all paths of 
    // that distance or less. For example, a connectivity query on starting location 
    // "sfo" with distance "100" should return all paths whose end locations are 
    // 100 miles or less away from the sfo location.
    class ConnInstance : public Instance {
    public:
        static Ptr<ConnInstance> instanceNew(const string& name, Ptr<Conn> cp) {
            Ptr<ConnInstance> cip = new ConnInstance(name);
            cip->conn_ = cp;
            return cip;
        }


        _noinline
        string attribute(const string& name) {
            using std::stringstream;

            stringstream ss(name);

            // Get first token of our query, which should be "explore"
            string cmd;
            ss >> cmd;
            if (cmd != "explore") {
                cerr << "Error: Queries must be of the form \"explore (loc) distance (value)\". You entered: " << name << endl;
                return "";
            }

            // Get second token of our query, which should be some location that exists in our network
            string locName;
            ss >> locName;

            // Get first token of our query, which should be "distance"
            string distanceString;
            ss >> distanceString;
            if (distanceString != "distance") {
                cerr << "Error: Queries must be of the form \"explore (loc) distance (value)\". You entered: " << name << endl;
                return "";
            }

            // Get fourth token of our query, which should be some unsigned int corresponding to the maximum distance we want to search.
            string distanceValueString;
            ss >> distanceValueString;
            unsigned int distanceValue;

            try
            {
                distanceValue = std::stoi(distanceValueString);
            }
            catch(std::invalid_argument& e) //or catch(...) to catch all exceptions
            
            {
                cerr << "Exception caught! Unable to convert (" << distanceValueString << ") to int!" << endl;
            }
            return conn_->processQuery(locName, distanceValue);
        }

        _noinline
        void attributeIs(const string& name, const string& value) {
            // Ignore.
            cerr << "Error! You cannot set the attribute of a Conn object. We track the connectivity by calculating a breadth-first search across our internal graph." << endl;
        }

    private:
        friend class TravelInstanceManager;
            ConnInstance(const string& name) : Instance(name) {
                // Nothing else to do.
            }
            Ptr<Conn> conn_;

    };

/******************************************************************************
*******************************************************************************
******************************************************************************/

    static Ptr<TravelInstanceManager> instanceManager_;

    /**
     * Initialize any state in the manager.
     */
    void initialize() {
        travelNetwork_ = TravelNetwork::instanceNew("myNetwork");
    }

    /**
     * Returns a newly-created instance with the specified name.
     * The spec parameter indicates additional information about
     * the instance, such as its type.
     */
    Ptr<Instance> createInstance(const string& name, const string& spec) {
        if (spec == "Residence") {
            return addLocation(Residence::instanceNew(name));
        } else if (spec == "Airport") {
            return addLocation(Airport::instanceNew(name));
        } else if (spec == "Flight") {
            return addSegment(Flight::instanceNew(name));
        } else if (spec == "Road") {
            return addSegment(Road::instanceNew(name));
        } else if (spec == "Airplane") {
            return addVehicle(Airplane::instanceNew(name));
        } else if (spec == "Car") {
            return addVehicle(Car::instanceNew(name));
        } else if (spec == "Stats") {
            return addStats(name);
        } else if (spec == "Conn") {
            return addConn(name);
        } 
        
        cerr << "Error: Invalid spec type (" << spec << ") passed in. Returning null." << endl;
        return null;
    }

    Ptr<Instance> addLocation(Ptr<Location> lp) {
        travelNetwork_->locationNew(lp);
        return LocationInstance::instanceNew(lp->name(), lp);
    }

    Ptr<Instance> addSegment(Ptr<Segment> sp) {
        travelNetwork_->segmentNew(sp);
        return SegmentInstance::instanceNew(sp->name(), sp);
    }

    Ptr<Instance> addVehicle(Ptr<Vehicle> vp) {
        travelNetwork_->vehicleNew(vp);
        return VehicleInstance::instanceNew(vp->name(), vp);
    }

    Ptr<Instance> addStats(const string& name) {
        if (statsInstance_ == null) {
            Ptr<Stats> sp = travelNetwork_->stats(name);
            statsInstance_ = StatsInstance::instanceNew(sp->name(), sp);
        }
        return statsInstance_;
    }

    Ptr<Instance> addConn(const string& name) {
        if (connInstance_ == null) {
            Ptr<Conn> cp = travelNetwork_->conn(name);
            connInstance_ = ConnInstance::instanceNew(cp->name(), cp);
        }
        return connInstance_;
    }

    Ptr<StatsInstance> statsInstance_;
    Ptr<ConnInstance> connInstance_;
    Ptr<TravelNetwork> travelNetwork_;
};

/******************************************************************************
*******************************************************************************
******************************************************************************/

const string TravelInstanceManager::LocationInstance::segmentStr = "segment";
const unsigned long TravelInstanceManager::LocationInstance::segmentStrlen =
    segmentStr.length();

Ptr<TravelInstanceManager> TravelInstanceManager::instanceManager_;

#endif
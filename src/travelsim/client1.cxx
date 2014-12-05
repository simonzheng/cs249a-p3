#include "TravelInstanceManager.h"

#include <ostream>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

// Ptr<Instance> segmentNew(
//     const Ptr<InstanceManager>& manager, const string& name, const string& spec, 
//     const string& source, const string& dest, const double length
// ) {
//     const auto seg = manager->instanceNew(name, spec);
//     seg->attributeIs("source", source);
//     seg->attributeIs("destination", dest);
//     seg->attributeIs("length", std::to_string(length));
//     return seg;
// }

// int main(const int argc, const char* const argv[]) {
//     const auto manager = TravelInstanceManager::instanceManager();
    
//     const auto stats = manager->instanceNew("myStats", "Stats");


//     const auto plane = manager->instanceNew("plane", "Airplane");
//     plane->attributeIs("speed", "500");
//     plane->attributeIs("capacity", "200");
//     plane->attributeIs("cost", "40");

//     const auto car = manager->instanceNew("car", "Car");
//     car->attributeIs("speed", "70");
//     car->attributeIs("capacity", "5");
//     car->attributeIs("cost", "0.75");

//     const auto residence = manager->instanceNew("stanford", "Residence");
//     manager->instanceNew("menlopark", "Residence");  

//     manager->instanceNew("sfo", "Airport");  
//     manager->instanceNew("lax", "Airport");  

//     segmentNew(manager, "carSeg1", "Road", "stanford", "sfo", 20);
//     segmentNew(manager, "carSeg2", "Road", "sfo", "stanford", 20);
//     segmentNew(manager, "carSeg3", "Road", "menlopark", "stanford", 20);
//     segmentNew(manager, "carSeg4", "Road", "sfo", "menlopark", 20);
//     segmentNew(manager, "carSeg5", "Road", "stanford", "menlopark", 5);
//     segmentNew(manager, "carSeg6", "Road", "menlopark", "stanford", 5);
//     segmentNew(manager, "flightSeg1", "Flight", "sfo", "lax", 350);

//     cout << "carSeg1.source: ";
//     cout << manager->instance("carSeg1")->attribute("source") << endl;


//     const auto conn = manager->instanceNew("myConn", "Conn");

//     const auto query1 = "explore sfo distance 500";
//     const auto query2 = "explore sfo distance 20";
//     cout << "**** " << query1 << " ****" << endl;
//     cout << conn->attribute(query1) << endl;
//     cout << "**** " << query2 << " ****" << endl;
//     cout << conn->attribute(query2) << endl;

//     cout << "Stats:" << endl;
//     cout << "# Residences: " << stats->attribute("Residence") << endl;
//     cout << "# Airports: " << stats->attribute("Airport") << endl;
//     cout << "# Road segments: " << stats->attribute("Road") << endl;

//     // Error checking code
//     // car->attributeIs("cost", "-0.75"); // should throw RangeException
//     // car->attributeIs("capacity", "-5"); // should throw RangeException but doesn't work right now
//     // residence->attribute("segmentNoNumber"); // should print error message that segment_ should have an integer
    

//     return 0;
// }


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


int main(const int argc, const char* const argv[]) {
    const auto tn1 = TravelNetwork::instanceNew("tn1");
    locationNew(tn1, "stanford1", "Residence");
    locationNew(tn1, "menlopark1", "Residence");

    locationNew(tn1, "sfo1", "Airport");
    locationNew(tn1, "lax1", "Airport");

    vehicleNew(tn1, "plane1", "Airplane", 500, 200, 40, "stanford1");
    vehicleNew(tn1, "car1", "Car", 70, 5, 0.75, "stanford1");
    // tn1->locationDel("lax1");
    
    segmentNew(tn1, "carSeg1", "Road", "stanford1", "sfo1", 20);
    segmentNew(tn1, "carSeg2", "Road", "sfo1", "stanford1", 20);
    segmentNew(tn1, "carSeg3", "Road", "menlopark1", "stanford1", 20);
    segmentNew(tn1, "carSeg4", "Road", "sfo1", "menlopark1", 20);
    segmentNew(tn1, "carSeg5", "Road", "stanford1", "menlopark1", 5);
    segmentNew(tn1, "carSeg6", "Road", "menlopark1", "stanford1", 5);
    segmentNew(tn1, "flightSeg1", "Flight", "sfo1", "lax1", 350);

    cout << "carSeg1.source: ";
    cout << tn1->segment("carSeg1")->source()->name() << endl;

    const auto conn = tn1->conn("conn1");

    string query1Loc = "sfo1";
    double query1Dist = 500;
    const auto query1 = "explore " + query1Loc + " distance " + std::to_string(query1Dist);

    string query2Loc = "sfo1";
    double query2Dist = 20;
    const auto query2 = "explore " + query2Loc + " distance " + std::to_string(query2Dist);
    cout << "**** " << query1 << " ****" << endl;
    cout << tn1->conn("conn1")->processQuery(query1Loc, query1Dist) << endl;
    cout << "**** " << query2 << " ****" << endl;
    cout << tn1->conn("conn1")->processQuery(query2Loc, query2Dist) << endl;

    cout << "Stats:" << endl;
    Ptr<Stats> stats = tn1->stats("stats1");
    cout << "# Residences: " << stats->numResidences() << endl;
    cout << "# Airports: " << stats->numAirports() << endl;
    cout << "# Road segments: " << stats->numRoads() << endl;
    
    string tripName1 = "trip1";
    string tripName2 = "trip2";
    
    cout << "**Adding Trips to Network **" << endl;
    tripNew(tn1, tripName1, "sfo1", "lax1", 350); // NTS: added
    tripNew(tn1, tripName2, "sfo1", "lax1", 350); // NTS: added
    const auto trip1 = tn1->trip(tripName1);
    const auto trip2 = tn1->trip(tripName2);
    cout << "# Trips: " << stats->numTrips() << endl;
    cout << "# Pickups: " << stats->numPickups() << endl;
    cout << "# droppedOff Trips: " << stats->numCompletedTrips() << endl;
    cout << "average wait time of Pickups: " << stats->averageWaitTime() << endl;
    
    trip1->statusIs(Trip::goingToPickup);
    cout << "**Trip1 set to goingToPickup**" << endl;
    cout << "# Trips: " << stats->numTrips() << endl;
    cout << "# Pickups: " << stats->numPickups() << endl;
    cout << "# droppedOff Trips: " << stats->numCompletedTrips() << endl;
    cout << "average wait time of Pickups: " << stats->averageWaitTime() << endl;

    trip2->statusIs(Trip::goingToPickup);
    cout << "**Trip2 set to goingToPickup**" << endl;
    cout << "# Trips: " << stats->numTrips() << endl;
    cout << "# Pickups: " << stats->numPickups() << endl;
    cout << "# droppedOff Trips: " << stats->numCompletedTrips() << endl;
    cout << "average wait time of Pickups: " << stats->averageWaitTime() << endl;

    trip1->statusIs(Trip::droppedOff);
    cout << "**Trip1 set to droppedOff**" << endl;
    cout << "# Trips: " << stats->numTrips() << endl;
    cout << "# Pickups: " << stats->numPickups() << endl;
    cout << "# droppedOff Trips: " << stats->numCompletedTrips() << endl;
    cout << "average wait time of Pickups: " << stats->averageWaitTime() << endl;

    tn1->tripDel(tripName1);
    cout << "**Trip1 deleted**" << endl;
    cout << "# Trips: " << stats->numTrips() << endl;
    cout << "# Pickups: " << stats->numPickups() << endl;
    cout << "# droppedOff Trips: " << stats->numCompletedTrips() << endl;
    cout << "average wait time (in minutes) of Pickups: " << stats->averageWaitTime() << endl;

    // Error checking code

    // car1->costIs(-0.75); // should throw RangeException
    // car1->capacityIs(-5); // should throw RangeException but doesn't work right now
    
    // trip1->statusIs(Trip::goingToPickup); // shouldn't let you change status back

    // Try to connect two networks
    // const auto tn2 = TravelNetwork::instanceNew("tn2");
    // Ptr<Airport> sfo2 = Airport::instanceNew("sfo2");
    // tn2->locationNew(sfo2);
    // Ptr<Road> testRoad = Road::instanceNew("testRoad");
    // testRoad->sourceIs(stanford1);
    // testRoad->destinationIs(sfo2);
    // testRoad->lengthIs(20);
    // tn1->segmentNew(testRoad);



    return 0;
}

#include "TravelInstanceManager.h"

#include <ostream>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

Ptr<Instance> segmentNew(
    const Ptr<InstanceManager>& manager, const string& name, const string& spec, 
    const string& source, const string& dest, const double length
) {
    const auto seg = manager->instanceNew(name, spec);
    seg->attributeIs("source", source);
    seg->attributeIs("destination", dest);
    seg->attributeIs("length", std::to_string(length));
    return seg;
}

int main(const int argc, const char* const argv[]) {
    const auto manager = TravelInstanceManager::instanceManager();

    const auto stats = manager->instanceNew("myStats", "Stats");


    const auto plane = manager->instanceNew("plane", "Airplane");
    plane->attributeIs("speed", "500");
    plane->attributeIs("capacity", "200");
    plane->attributeIs("cost", "40");

    const auto car = manager->instanceNew("car", "Car");
    car->attributeIs("speed", "70");
    car->attributeIs("capacity", "5");
    car->attributeIs("cost", "0.75");


    manager->instanceNew("stanford", "Residence");  
    manager->instanceNew("menlopark", "Residence");  

    manager->instanceNew("sfo", "Airport");  
    manager->instanceNew("lax", "Airport");  

    segmentNew(manager, "carSeg1", "Road", "stanford", "sfo", 20);
    segmentNew(manager, "carSeg2", "Road", "sfo", "stanford", 20);
    segmentNew(manager, "carSeg3", "Road", "menlopark", "stanford", 20);
    segmentNew(manager, "carSeg4", "Road", "sfo", "menlopark", 20);
    segmentNew(manager, "carSeg5", "Road", "stanford", "menlopark", 5);
    segmentNew(manager, "carSeg6", "Road", "menlopark", "stanford", 5);
    segmentNew(manager, "flightSeg1", "Flight", "sfo", "lax", 350);

    cout << "carSeg1.source: ";
    cout << manager->instance("carSeg1")->attribute("source") << endl;


    const auto conn = manager->instanceNew("myConn", "Conn");

    const auto query1 = "explore sfo distance 500";
    const auto query2 = "explore sfo distance 20";
    cout << "**** " << query1 << " ****" << endl;
    cout << conn->attribute(query1) << endl;
    cout << "**** " << query2 << " ****" << endl;
    cout << conn->attribute(query2) << endl;

    cout << "Stats:" << endl;
    cout << "# Residences: " << stats->attribute("Residence") << endl;
    cout << "# Airports: " << stats->attribute("Airport") << endl;
    cout << "# Road segments: " << stats->attribute("Road") << endl;

    return 0;
}

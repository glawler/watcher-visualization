#include <string>
#include <boost/foreach.hpp>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/connectivityMessage.h>
#include "GFC.h"
#include "configuration.h"
#include "fileparser.h"
#include "sqliteDatabase.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
namespace po=boost::program_options;

struct Node {
    GpsFileParser *parser;
    double x, y, z;
    unsigned int nid;
    Timestamp ts;
    Node() { }
    Node(const Node &n) { nid=n.nid; x=n.x; y=n.y; z=n.z; parser=n.parser; ts=n.ts; }
};

void buildNodeVector(const string &scenFile, vector<Node> &nodes)
{
    FILE *theFile;
    if (NULL==(theFile=fopen(scenFile.c_str(), "r"))) {
        fprintf(stderr, "Unable to open %s for reading.\n", scenFile.c_str()); 
        exit(EXIT_FAILURE);
    }

    char buf[265];
    while (fgets(buf, sizeof(buf), theFile)) {
        unsigned int nid;
        char file[sizeof(buf)-sizeof(nid)-sizeof(char)]; // give it all remaining space from the line.
        if (2!=sscanf(buf, "%d,%s\n", &nid, &file)) {
            fprintf(stderr, "Error reading scenario file\n"); 
            exit(EXIT_FAILURE);
        }
        Node node;
        node.nid=(192<<24|168<<16)+nid;
        node.parser=new GpsFileParser;
        if (!node.parser->openFile(file)) {
            fprintf(stderr, "Unable to open file %s for reading.\n", &file);
            exit(EXIT_FAILURE);
        }
        // fprintf(stdout, "Opened data file for node %d (%u)\n", nid, node.nid); 
        nodes.push_back(node);
    }
}
void freeNodeVector(vector<Node> &nodes) 
{
    BOOST_FOREACH(Node &n, nodes) {
        // fprintf(stdout, "Closing data file for node %u\n", n.nid); 
        n.parser->closeFile();
        delete n.parser;
    }
}
bool moveForwardOneStep(vector<Node> &nodes) 
{
    BOOST_FOREACH(Node &n, nodes) 
        if (!n.parser->yield(n.ts, n.x, n.y, n.z))
            return false;
    return true;
}

int main(int argc, char *argv[])
{
    // Load config and bail on error. 
    if (!Gps2EventDb::loadConfiguration(argc, argv))
        return EXIT_FAILURE;

    // Show loaded config on stderr
    Gps2EventDb::dumpConfiguration(std::cerr);

    po::variables_map &config=Gps2EventDb::getConfig();
    string scenFile=config["scenario"].as<string>();
    double rad=config["radius"].as<double>();

    string dbName("event.db"); 
    Database *db=Database::connect(dbName); 
    if (!db) {
        fprintf(stderr, "Unable to create database.\n"); 
        exit(EXIT_FAILURE);
    }
    
    vector<Node> nodes;
    buildNodeVector(scenFile, nodes);

    CEarth earth;
    GPSMessagePtr gpsMess=(GPSMessagePtr)new GPSMessage;
    ConnectivityMessagePtr conMess=(ConnectivityMessagePtr)new ConnectivityMessage;
    conMess->layer="One_Hop_Routing";
    int totalEvents=0, gpsEvents=0, nbrEvents=0;
    while (moveForwardOneStep(nodes)) {
        BOOST_FOREACH(const Node &n, nodes) {
            // cout << "node " << n.nid << " @ " << n.ts << ": " << n.x << ", " << n.y << ", " << n.z << endl;
            boost::asio::ip::address_v4 naddr(n.nid);
            gpsMess->timestamp=(Timestamp)n.ts;
            gpsMess->x=n.x;
            gpsMess->y=n.y;
            gpsMess->z=n.z;
            gpsMess->fromNodeID=naddr;
            db->storeEvent(gpsMess);
            totalEvents++;
            gpsEvents++;

            CPolarCoordinate node1Point;
            node1Point.SetUpDownAngleInDegrees(n.y);
            node1Point.SetLeftRightAngleInDegrees(n.x);
            node1Point.SetDistanceFromSurfaceInMeters(n.z); 

            ConnectivityMessage::NeighborList nbrs;
            BOOST_FOREACH(const Node &n2, nodes) {
                if (n.nid==n2.nid)
                    continue;

                CPolarCoordinate node2Point;
                node2Point.SetUpDownAngleInDegrees(n2.y);
                node2Point.SetLeftRightAngleInDegrees(n2.x);
                node2Point.SetDistanceFromSurfaceInMeters(n2.z); 
                double dist=earth.GetSurfaceDistance(node1Point, node2Point);
                // cout << "dist between nodes: " << dist << endl;
                if (dist<rad) {
                    boost::asio::ip::address_v4 addr(n2.nid); 
                    nbrs.push_back(addr);
                    // cout << "adding connection: " << gpsMess->fromNodeID << " <--> " << addr << endl;
                }
            }
            conMess->neighbors=nbrs;
            conMess->timestamp=(Timestamp)n.ts;
            conMess->fromNodeID=naddr;
            db->storeEvent(conMess);
            totalEvents++;
            nbrEvents++;
        }
    }

    freeNodeVector(nodes); 

    printf("Added %d events (%d gps events and %d connectivity events)\n", totalEvents, gpsEvents, nbrEvents); 

    return(EXIT_SUCCESS);
}

#include <iostream>

#include <libwatcher/client.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/colorMessage.h>
#include <libwatcher/colors.h>
#include <libwatcher/watcherColors.h>
#include <libwatcher/connectivityMessage.h>
#include <libwatcher/sendMessageHandler.h>

#include <logger.h>

#include "configuration.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
namespace po=boost::program_options;
namespace rs=randomScenario;

int main(int argc, char **argv) 
{
    // Load config and bail on error. 
    if (!rs::loadConfiguration(argc, argv))
        return EXIT_FAILURE;

    rs::printConfiguration(cout); 

    srand(time(NULL));

    const unsigned int MAX_LAT=1000;
    const unsigned int MAX_LNG=1000;
    const unsigned int MAX_ALT=100;

    po::variables_map &config=rs::getConfig();
    unsigned int nodeNum=config["nodeNum"].as<unsigned int>(); 
    float nodeDegreePercentage=config["nodeDegreePercentage"].as<float>();
    float nodeLabelPercentage=config["nodeLabelPercentage"].as<float>();
    unsigned int layerNum=config["layerNum"].as<unsigned int>();
    int duration=config["duration"].as<int>(); 
    string server=config["server"].as<string>(); 

    LOAD_LOG_PROPS(config["logproperties"].as<string>());

    cout << "Connecting to watcherd on " << server << endl;
    watcher::Client client(server); 
    client.addMessageHandler(MultipleMessageHandler::create());

    while (duration) { 
        GPSMessagePtr gpsMess(GPSMessagePtr(new GPSMessage)); 
        ConnectivityMessagePtr connMess(ConnectivityMessagePtr(new ConnectivityMessage));

        for (int i=1; i<=nodeNum; i++) {
                NodeIdentifier nid=boost::asio::ip::address_v4::address_v4(i);
                gpsMess->x=(rand()%MAX_LAT/30000.0);
                gpsMess->y=(rand()%MAX_LNG/30000.0);
                gpsMess->z=rand()%MAX_ALT;
                gpsMess->fromNodeID=nid;
                if (!client.sendMessage(gpsMess)) 
                    cerr << "Error sending gps message #" << i << endl;

                if (rand()%100 <= (unsigned int)nodeLabelPercentage) { 
                    LabelMessagePtr lm(new LabelMessage("Label", nid)); 
                    if (!client.sendMessage(lm)) 
                        cerr << "Error sending node label message"; 
                }

                for (unsigned int l=0; l<layerNum; l++) {
                    connMess->fromNodeID=nid;
                    connMess->layer="ConnectivityMessages_" + boost::lexical_cast<string>(l);
                    for (int n=0; n<nodeNum*(nodeDegreePercentage/100.0); n++) 
                        connMess->neighbors.push_back(boost::asio::ip::address_v4::address_v4((rand()%nodeNum)+1));
                    if (!client.sendMessage(connMess)) 
                        cerr << "Error sending connectivity message #" << i << endl;
                    connMess->neighbors.clear();
                }
        }

        if (duration>0)
            duration--;

        sleep(1);
    }

    client.wait();
    client.close();

    return EXIT_SUCCESS;
}

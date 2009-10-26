/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * Use Google's libkml to out a KML file based upon the current network topology.
 * @author michael.elkins@cobham.com
 */

#define CUSTOM_ICON // use non-default icon for nodes

// libkml
#include "kml/convenience/convenience.h"
#include "kml/engine.h"
#include "kml/dom.h"

#include "libwatcher/watcherGraph.h"

#include <boost/lexical_cast.hpp>

using kmldom::ChangePtr;
using kmldom::CreatePtr;
using kmldom::CoordinatesPtr;
using kmldom::DocumentPtr;
using kmldom::FeaturePtr;
using kmldom::FolderPtr;
using kmldom::IconStyleIconPtr;
using kmldom::IconStylePtr;
using kmldom::KmlFactory;
using kmldom::KmlPtr;
using kmldom::LineStringPtr;
using kmldom::PlacemarkPtr;
using kmldom::PointPtr;
using kmldom::StylePtr;
using kmldom::UpdatePtr;
using kmlengine::KmlFile;
using kmlengine::KmlFilePtr;

using namespace watcher;

namespace {

#ifdef CUSTOM_ICON
/*
 * Keep a mapping to the random Icon associated with a particular node so that the
 * same icon is used between invocations of write_kml().
 */
typedef std::map<NodeIdentifier, std::string> NodeIconMap ;
typedef NodeIconMap::iterator NodeIconMapIterator ;
NodeIconMap nodeIconMap;

const std::string BASE_ICON_URL = "http://maps.google.com/mapfiles/kml/shapes/";

//interesting icons for placemarks
const char *ICONS[] = {
    "cabs.png",
    "bus.png",
    "rail.png",
    "truck.png",
    "airports.png",
    "ferry.png",
    "heliport.png",
    "tram.png",
    "sailing.png"
};

// template to get the number of items in an array
template <typename T, int N> size_t sizeof_array( T (&)[N] ) { return N; }

void icon_setup(KmlFactory* kmlFac, DocumentPtr document)
{
    // set up styles for each icon we use, using the icon name as the id
    for (size_t i = 0; i < sizeof_array(ICONS); ++i) {
        IconStyleIconPtr icon(kmlFac->CreateIconStyleIcon());
        std::string url(BASE_ICON_URL + ICONS[i]);
        icon->set_href(url.c_str());

        IconStylePtr iconStyle(kmlFac->CreateIconStyle());
        iconStyle->set_icon(icon);

        StylePtr style(kmlFac->CreateStyle());
        style->set_iconstyle(iconStyle);
        style->set_id(ICONS[i]);

        document->add_styleselector(style);
    }
}
#endif // CUSTOM_ICON

void output_nodes(KmlFactory* kmlFac, const WatcherGraph& graph, FolderPtr folder)
{
    // iterate over all nodes in the graph
    WatcherGraph::vertexIterator vi, vend;
    for (tie(vi, vend) = vertices(graph.theGraph); vi != vend; ++vi) {
        const WatcherGraphNode &node = graph.theGraph[*vi]; 

        PlacemarkPtr ptr = kmlFac->CreatePlacemark();
        std::string ip(node.nodeId.to_string());
        ptr->set_name(ip); // textual label, can be html
        ptr->set_id(ip); // internal label for locating object in the DOM tree

        // set the location
        ptr->set_geometry(kmlconvenience::CreatePointLatLon(node.gpsData->y, node.gpsData->x));

        /*
        //id and target id are both set here
        //target id is required when changing some attribute of a feature already in the dom
        ptr->set_id("node0");
        ptr->set_targetid("node0");
        */

#ifdef CUSTOM_ICON
        // for testing purposes, randomly select an interesting icon to replace the default yellow pushpin
        std::string styleUrl;
        NodeIconMapIterator it = nodeIconMap.find(node.nodeId);
        if (it == nodeIconMap.end()) {
            styleUrl = std::string("#") + ICONS[ random() % sizeof_array(ICONS) ];
            nodeIconMap[node.nodeId] = styleUrl;
        } else
            styleUrl = it->second;
        ptr->set_styleurl(styleUrl);
#endif

        folder->add_feature(ptr);//add to DOM
    }
}

void output_edges(KmlFactory* kmlFac, const WatcherGraph& graph, FolderPtr folder)
{
    WatcherGraph::edgeIterator ei, eend;
    unsigned int count = 0;
    for (tie(ei, eend) = edges(graph.theGraph); ei != eend; ++ei, ++count) {
        const WatcherGraphEdge &edge = graph.theGraph[*ei]; 
        const WatcherGraphNode &node1 = graph.theGraph[source(*ei, graph.theGraph)]; 
        const WatcherGraphNode &node2 = graph.theGraph[target(*ei, graph.theGraph)]; 

        CoordinatesPtr coords = kmlFac->CreateCoordinates();
        coords->add_latlng(node1.gpsData->y, node1.gpsData->x);
        coords->add_latlng(node2.gpsData->y, node2.gpsData->x);

        LineStringPtr lineString = kmlFac->CreateLineString();
        lineString->set_coordinates(coords);

        PlacemarkPtr ptr = kmlFac->CreatePlacemark();
        ptr->set_geometry(lineString);

        std::string label("edge-" + boost::lexical_cast<std::string>(count));
        ptr->set_id(label);
        ptr->set_name(label);

        folder->add_feature(ptr);//add to DOM
    }
}

} // end namespace


void write_kml(const WatcherGraph& graph, const std::string& outputFile)
{
    // libkml boilerplate
    KmlFactory* kmlFac(kmldom::KmlFactory::GetFactory());
    KmlPtr kml(kmlFac->CreateKml());

    /*
     * create initial DOM tree.  As nodes appear, they get added to the tree. 
     * As nodes move, we update the position attribute
     * in the DOM, and regenerate the KML file.
     */
    DocumentPtr document(kmlFac->CreateDocument());
    kml->set_feature(document);

    FolderPtr folder(kmlFac->CreateFolder());
    folder->set_name("Watcher");
    document->add_feature(folder);

#ifdef CUSTOM_ICON
    icon_setup(kmlFac, document);
#endif // CUSTOM_ICON
    output_nodes(kmlFac, graph, folder);
    output_edges(kmlFac, graph, folder);

    kmlbase::File::WriteStringToFile(kmldom::SerializePretty(kml), outputFile);
}

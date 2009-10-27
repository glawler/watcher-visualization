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
#include <boost/foreach.hpp>

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
using kmldom::LabelStylePtr;
using kmldom::LineStringPtr;
using kmldom::LineStylePtr;
using kmldom::MultiGeometryPtr;
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

// interesting icons for placemarks
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

/*
 * For testing purposes, randomly select an interesting icon to replace the default yellow pushpin.
 * In order to use the same icon between invocations of write_kml(), a map is used to store the icon
 * selected.
 */
void set_node_icon(const WatcherGraphNode& node, PlacemarkPtr ptr)
{
    std::string styleUrl;
    NodeIconMapIterator it = nodeIconMap.find(node.nodeId);
    if (it == nodeIconMap.end()) {
        styleUrl = std::string("#") + ICONS[ random() % sizeof_array(ICONS) ];
        nodeIconMap[node.nodeId] = styleUrl;
    } else
        styleUrl = it->second;
    ptr->set_styleurl(styleUrl);
}
#endif // CUSTOM_ICON

typedef std::map<GUILayer, FolderPtr> LayerMap;
typedef LayerMap::iterator LayerMapIterator;

struct DefinedLabelStyle {
    std::string id;
    watcher::Color color;
    DefinedLabelStyle(const watcher::Color& col, const std::string& name) : id(name), color(col) {}
};

typedef boost::shared_ptr<DefinedLabelStyle> DefinedLabelStylePtr;
typedef std::list<DefinedLabelStylePtr> DefinedLabelStyleList;

class Args {
    public:
        Args(const WatcherGraph&);
        KmlPtr kml;
        void start();
    private:
        KmlFactory *kmlFac;
        const WatcherGraph& graph;
        DocumentPtr doc;
        FolderPtr topFolder;
        LayerMap layerMap;
        int label_count;
        DefinedLabelStyleList definedLabelStyles;

        void icon_setup();
        FolderPtr get_layer(const GUILayer& name);
        void output_nodes();
        void output_edges();
        void add_labels(const std::list<LabelDisplayInfoPtr>&, double lat, double lng);
        void add_to_layer(const GUILayer& name, PlacemarkPtr);
        PointPtr create_point(double lat, double lng);
        std::string get_label_style(LabelDisplayInfoPtr dispInfo);
};

/*
 * Convert a watcher color to the KML format.
 */
std::string watcher_color_to_kml(const watcher::Color& color)
{
    char buf[sizeof("aabbggrr")];
    sprintf(buf, "%02x%02x%02x%02x", color.a, color.b, color.g, color.r);
    return std::string(buf);
}

Args::Args(const WatcherGraph& g) :
    // libkml boilerplate
    kmlFac(kmldom::KmlFactory::GetFactory()),
    graph(g),
    kml(kmlFac->CreateKml()),
    doc(kmlFac->CreateDocument()),
    topFolder(kmlFac->CreateFolder()),
    label_count(0)
{

    /*
     * create initial DOM tree.  As nodes appear, they get added to the tree. 
     * As nodes move, we update the position attribute
     * in the DOM, and regenerate the KML file.
     */
    kml->set_feature(doc);
    topFolder->set_name("Watcher");
    doc->add_feature(topFolder);
}

void Args::start()
{
    icon_setup();
    output_nodes();
    output_edges();
}

void Args::icon_setup()
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

        doc->add_styleselector(style);
    }
}

FolderPtr Args::get_layer(const GUILayer& name)
{
    LayerMapIterator it = layerMap.find(name);
    if (it == layerMap.end()) {
        FolderPtr layer = kmlFac->CreateFolder();
        layer->set_name(name);
        topFolder->add_feature(layer);
        layerMap[name] = layer;
        return layer;
    } else
        return it->second;
}

void Args::add_to_layer(const GUILayer& name, PlacemarkPtr place)
{
    FolderPtr layer = get_layer(name);
    layer->add_feature(place);
}

std::string Args::get_label_style(LabelDisplayInfoPtr dispInfo)
{
    // many labels will probably share the same attributes, so we cache
    // them to make the KML file smaller
    BOOST_FOREACH(DefinedLabelStylePtr lsp, definedLabelStyles) {
        if (dispInfo->foregroundColor == lsp->color)
            return lsp->id;
    }
    // not found, create a new style

    // no icon for labels
    IconStylePtr iconStyle(kmlFac->CreateIconStyle());
    iconStyle->set_scale(0); // scale to 0 should mean hide it?

    kmldom::LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
    labelStyle->set_color(watcher_color_to_kml(dispInfo->foregroundColor));
    //TODO: KML doesn't support background colors

    std::string url("label-style-" + boost::lexical_cast<std::string>(label_count));
    ++label_count;

    StylePtr style(kmlFac->CreateStyle());
    style->set_id(url);
    style->set_iconstyle(iconStyle);
    style->set_labelstyle(labelStyle);

    doc->add_styleselector(style);

    definedLabelStyles.push_back( DefinedLabelStylePtr(new DefinedLabelStyle(dispInfo->foregroundColor, url)));

    return url;
}

// create a placemark for each label, putting into the appropriate layer
void Args::add_labels(const std::list<LabelDisplayInfoPtr>& labels,
                      double lat, double lng)
{
    BOOST_FOREACH(LabelDisplayInfoPtr dispInfo, labels) {
        PlacemarkPtr place(kmlFac->CreatePlacemark());
        place->set_name(dispInfo->labelText);
        place->set_geometry(create_point(lat, lng));
        place->set_styleurl("#" + get_label_style(dispInfo));

        add_to_layer(dispInfo->layer, place);

        ++label_count;
    }
}

/*
 * Point's can not be shared because even though they are marked const, libKML does modify them.
 * Therefore, we must duplicate the points for each Placemark
 */
PointPtr Args::create_point(double lat, double lng)
{
    CoordinatesPtr coords(kmlFac->CreateCoordinates());
    coords->add_latlng(lat, lng);

    PointPtr point(kmlFac->CreatePoint());
    point->set_coordinates(coords);

    return point;
}

void Args::output_nodes()
{
    // iterate over all nodes in the graph
    WatcherGraph::vertexIterator vi, vend;
    for (tie(vi, vend) = vertices(graph.theGraph); vi != vend; ++vi) {
        const WatcherGraphNode &node = graph.theGraph[*vi]; 

        PlacemarkPtr ptr = kmlFac->CreatePlacemark();
        ptr->set_name(node.displayInfo->get_label()); // textual label, can be html
        ptr->set_geometry(create_point(node.gpsData->y, node.gpsData->x));

        //target id is required when changing some attribute of a feature already in the dom
        //ptr->set_targetid("node0");

#ifdef CUSTOM_ICON
        set_node_icon(node, ptr);
#endif
        add_to_layer(node.displayInfo->layer, ptr);

        // create a placemark for each label, putting into the appropriate layer
        add_labels(node.labels, node.gpsData->y, node.gpsData->x);
    }
}

void Args::output_edges()
{
    WatcherGraph::edgeIterator ei, eend;
    unsigned int count = 0;
    bool has_style = false;

    for (tie(ei, eend) = edges(graph.theGraph); ei != eend; ++ei, ++count) {
        const WatcherGraphEdge &edge = graph.theGraph[*ei]; 
        const WatcherGraphNode &node1 = graph.theGraph[source(*ei, graph.theGraph)]; 
        const WatcherGraphNode &node2 = graph.theGraph[target(*ei, graph.theGraph)]; 

        CoordinatesPtr coords = kmlFac->CreateCoordinates();
        coords->add_latlng(node1.gpsData->y, node1.gpsData->x);
        coords->add_latlng(node2.gpsData->y, node2.gpsData->x);

        LineStringPtr lineString = kmlFac->CreateLineString();
        lineString->set_coordinates(coords);

        //place label at the midpoint on the line between the two nodes
        PointPtr point(create_point((node1.gpsData->y + node2.gpsData->y)/2,(node1.gpsData->x + node2.gpsData->x)/2));

        /*
         * Google Earth doesn't allow a label to be attached to something without a Point, so
         * we need to create a container with the LineString and the Point at which to attach
         * the label/icon.
         */
        MultiGeometryPtr multiGeo(kmlFac->CreateMultiGeometry());
        multiGeo->add_geometry(lineString);
        multiGeo->add_geometry(point);

        PlacemarkPtr ptr = kmlFac->CreatePlacemark();
        ptr->set_geometry(multiGeo);
        ptr->set_name(edge.displayInfo->label);

        // WatcherGraph only has a single style per layer, so we stash the value somewhere instead of creating multiple styles
        if (!has_style) {
            LineStylePtr lineStyle(kmlFac->CreateLineStyle());
            lineStyle->set_color(watcher_color_to_kml(edge.displayInfo->color));
            lineStyle->set_width(edge.displayInfo->width);

            IconStylePtr iconStyle(kmlFac->CreateIconStyle());
            iconStyle->set_scale(0); // scale to 0 should mean hide it?

            StylePtr style(kmlFac->CreateStyle());
            style->set_id("edge-style");
            style->set_linestyle(lineStyle);
            style->set_iconstyle(iconStyle);

            kmldom::LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
            labelStyle->set_color(watcher_color_to_kml(edge.displayInfo->labelColor));
            style->set_labelstyle(labelStyle);

            doc->add_styleselector(style);
            has_style = true;
        }

        ptr->set_styleurl("#edge-style");

        add_to_layer(edge.displayInfo->layer, ptr);

        // add additional placemarks for each label on this edge
        add_labels(edge.labels,
                   (node1.gpsData->y + node2.gpsData->y)/2,
                   (node1.gpsData->x + node2.gpsData->x)/2);
    }
}

} // end namespace


void write_kml(const WatcherGraph& graph, const std::string& outputFile)
{
    Args args(graph);
    args.start();

    kmlbase::File::WriteStringToFile(kmldom::SerializePretty(args.kml), outputFile);
}

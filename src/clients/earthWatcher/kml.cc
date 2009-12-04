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

// libkml
#include "kml/convenience/convenience.h"
#include "kml/engine.h"
#include "kml/dom.h"
#include "kml/base/file.h"

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

namespace watcher {
    // set from config file
    extern float LayerPadding;
}

namespace {

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

struct LayerInfo {
    float zpad; // alt padding value to order layers vertically
    FolderPtr folder; // folder for this layer
    LayerInfo(float zp, FolderPtr p) : zpad(zp), folder(p) {}
    LayerInfo() : zpad(0.0) {}  // required for use in std::map
};

typedef std::map<GUILayer, LayerInfo> LayerMap;
typedef LayerMap::iterator LayerMapIterator;

struct DefinedLabelStyle {
    std::string id;
    watcher::Color color;
    DefinedLabelStyle(const watcher::Color& col, const std::string& name) : id(name), color(col) {}
};

typedef boost::shared_ptr<DefinedLabelStyle> DefinedLabelStylePtr;
typedef std::list<DefinedLabelStylePtr> DefinedLabelStyleList;

class Render {
    public:
        Render(const WatcherGraph&);
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
        float zpad;

        void icon_setup();
        const LayerInfo& get_layer(const GUILayer& name);
        void output_nodes();
        void output_edges();
        void output_floating_labels();
        void add_label(const GUILayer& layer, const std::string& label, const watcher::Color& color, double lat, double lng, double alt);
        void add_label(LabelDisplayInfoPtr dispInfo, double lat, double lng, double alt);
        void add_labels(const std::list<LabelDisplayInfoPtr>&, double lat, double lng, double alt);
        PointPtr create_point(double lat, double lng, double alt);
        std::string get_label_style(const watcher::Color&);
        std::string get_edge_style(const WatcherGraphEdge& edge, unsigned int edgenum);
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

Render::Render(const WatcherGraph& g) :
    // libkml boilerplate
    kmlFac(kmldom::KmlFactory::GetFactory()),
    graph(g),
    kml(kmlFac->CreateKml()),
    doc(kmlFac->CreateDocument()),
    topFolder(kmlFac->CreateFolder()),
    label_count(0),
    zpad(0.0)
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

void Render::start()
{
    icon_setup();
    output_nodes();
    output_edges();
    output_floating_labels();
}

// generate styles for each supported icon so they can be shared.
// note that we disable the label on the icon and expect that another placemark with the label will be located at the same position
void Render::icon_setup()
{
    // set up styles for each icon we use, using the icon name as the id
    for (size_t i = 0; i < sizeof_array(ICONS); ++i) {
        IconStyleIconPtr icon(kmlFac->CreateIconStyleIcon());
        std::string url(BASE_ICON_URL + ICONS[i]);
        icon->set_href(url.c_str());

        IconStylePtr iconStyle(kmlFac->CreateIconStyle());
        iconStyle->set_icon(icon);

        LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
        labelStyle->set_scale(0); // hide the label, we use separate placemarks for the labels so they can have their own style

        StylePtr style(kmlFac->CreateStyle());
        style->set_iconstyle(iconStyle);
        style->set_labelstyle(labelStyle);
        style->set_id(ICONS[i]);

        doc->add_styleselector(style);
    }
}

/*
 * Return the KML folder associated with the given layer name.  Creates the layer if it does not already exist.
 */
const LayerInfo& Render::get_layer(const GUILayer& name)
{
    LayerMapIterator it = layerMap.find(name);
    if (it == layerMap.end()) {
        FolderPtr layer = kmlFac->CreateFolder();
        layer->set_name(name);
        topFolder->add_feature(layer);
        if (!layerMap.empty())
            zpad += LayerPadding;
        layerMap[name] = LayerInfo(zpad, layer);
        return layerMap[name];
    } else
        return it->second;
}

/*
 * many labels will probably share the same attributes, so we cache
 * them to make the KML file smaller
  */
std::string Render::get_label_style(const watcher::Color& color)
{
    BOOST_FOREACH(DefinedLabelStylePtr lsp, definedLabelStyles) {
        if (color == lsp->color)
            return lsp->id;
    }
    // not found, create a new style

    // no icon for labels
    IconStylePtr iconStyle(kmlFac->CreateIconStyle());
    iconStyle->set_scale(0); // scale to 0 means hide it

    kmldom::LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
    labelStyle->set_color(watcher_color_to_kml(color));
    //TODO: KML doesn't support background colors for placemark labels

    std::string url("label-style-" + boost::lexical_cast<std::string>(label_count));
    ++label_count;

    StylePtr style(kmlFac->CreateStyle());
    style->set_id(url);
    style->set_iconstyle(iconStyle);
    style->set_labelstyle(labelStyle);

    doc->add_styleselector(style);

    definedLabelStyles.push_back( DefinedLabelStylePtr(new DefinedLabelStyle(color, url)));

    return url;
}

void Render::add_label(const GUILayer& layer, const std::string& label, const watcher::Color& color, double lat, double lng, double alt)
{
    const LayerInfo& li (get_layer(layer));

    PlacemarkPtr place(kmlFac->CreatePlacemark());
    place->set_name(label);
    place->set_geometry(create_point(lat, lng, alt + li.zpad));
    place->set_styleurl("#" + get_label_style(color));

    li.folder->add_feature(place);
}

void Render::add_label(LabelDisplayInfoPtr dispInfo, double lat, double lng, double alt)
{
    add_label(dispInfo->layer, dispInfo->labelText, dispInfo->foregroundColor, lat, lng, alt);
}

// create a placemark for each label, putting into the appropriate layer
void Render::add_labels(const std::list<LabelDisplayInfoPtr>& labels, double lat, double lng, double alt)
{
    BOOST_FOREACH(LabelDisplayInfoPtr dispInfo, labels) {
        add_label(dispInfo, lat, lng, alt);
    }
}

/*
 * Point's can not be shared because even though they are marked const, libKML does modify them.
 * Therefore, we must duplicate the points for each Placemark
 */
PointPtr Render::create_point(double lat, double lng, double alt)
{
    CoordinatesPtr coords(kmlFac->CreateCoordinates());
    coords->add_latlngalt(lat, lng, alt);

    PointPtr point(kmlFac->CreatePoint());
    point->set_coordinates(coords);
    point->set_altitudemode(kmldom::ALTITUDEMODE_ABSOLUTE);     // avoid clamping to ground

    return point;
}

void Render::output_floating_labels()
{
    WatcherGraph::FloatingLabelList::const_iterator b = graph.floatingLabels.begin(); 
    WatcherGraph::FloatingLabelList::const_iterator e = graph.floatingLabels.end(); 
    for ( ; b != e; ++b) {
        add_label((*b)->layer, (*b)->labelText, (*b)->foregroundColor, (*b)->lat, (*b)->lng, (*b)->alt);
    }
}

void Render::output_nodes()
{
    // iterate over all nodes in the graph
    WatcherGraph::vertexIterator vi, vend;
    for (tie(vi, vend) = vertices(graph.theGraph); vi != vend; ++vi) {
        const WatcherGraphNode &node = graph.theGraph[*vi]; 

        const LayerInfo& layer(get_layer(node.displayInfo->layer));

        PlacemarkPtr ptr = kmlFac->CreatePlacemark();
        //ptr->set_name(node.displayInfo->get_label()); // textual label, can be html
        ptr->set_geometry(create_point(node.gpsData->y, node.gpsData->x, node.gpsData->z + layer.zpad));
        ptr->set_id(node.nodeId.to_string());
        // target id is required when changing some attribute of a feature already in the dom
        //ptr->set_targetid("node0");
        set_node_icon(node, ptr);

        layer.folder->add_feature(ptr); // add placemark to DOM

        // add a label for the node separate from its placemark icon
        add_label(node.displayInfo->layer, node.displayInfo->get_label(), node.displayInfo->labelColor, node.gpsData->y, node.gpsData->x, node.gpsData->z);

        // create a placemark for each label, putting into the appropriate layer
        add_labels(node.labels, node.gpsData->y, node.gpsData->x, node.gpsData->z);
    }
}

std::string Render::get_edge_style(const WatcherGraphEdge& edge, unsigned int edgenum)
{
        std::string edgeid = "edge-style-" + boost::lexical_cast<std::string>(edgenum);

        LineStylePtr lineStyle(kmlFac->CreateLineStyle());
        lineStyle->set_color(watcher_color_to_kml(edge.displayInfo->color));
        lineStyle->set_width(edge.displayInfo->width);

        IconStylePtr iconStyle(kmlFac->CreateIconStyle());
        iconStyle->set_scale(0); // scale to 0 means hide it

        StylePtr style ( kmlFac->CreateStyle() );
        style->set_id(edgeid);
        style->set_linestyle(lineStyle);
        style->set_iconstyle(iconStyle);

        kmldom::LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
        if (edge.displayInfo->label == "none") {
            labelStyle->set_scale(0); // hide the label
        }
        labelStyle->set_color(watcher_color_to_kml(edge.displayInfo->labelColor));
        style->set_labelstyle(labelStyle);

        doc->add_styleselector(style);

        return "#" + edgeid;
}

void Render::output_edges()
{
    WatcherGraph::edgeIterator ei, eend;
    int edgenum = 0;

    for (tie(ei, eend) = edges(graph.theGraph); ei != eend; ++ei, ++edgenum) {
        const WatcherGraphEdge &edge = graph.theGraph[*ei]; 
        const WatcherGraphNode &node1 = graph.theGraph[source(*ei, graph.theGraph)]; 
        const WatcherGraphNode &node2 = graph.theGraph[target(*ei, graph.theGraph)]; 

        const LayerInfo& layer(get_layer(edge.displayInfo->layer));

        CoordinatesPtr coords = kmlFac->CreateCoordinates();
        coords->add_latlngalt(node1.gpsData->y, node1.gpsData->x, node1.gpsData->z + layer.zpad);
        coords->add_latlngalt(node2.gpsData->y, node2.gpsData->x, node2.gpsData->z + layer.zpad);

        LineStringPtr lineString = kmlFac->CreateLineString();
        lineString->set_coordinates(coords);
        lineString->set_altitudemode(kmldom::ALTITUDEMODE_ABSOLUTE);    // avoid clamping points to the ground

        // place label at the midpoint on the line between the two nodes
        PointPtr point(create_point((node1.gpsData->y + node2.gpsData->y)/2,(node1.gpsData->x + node2.gpsData->x)/2, (node1.gpsData->z + node2.gpsData->z)/2 + layer.zpad ));

        /*
         * Google Earth doesn't allow a label to be attached to something without a Point, so
         * we need to create a container with the LineString and the Point at which to attach
         * the label/icon.
         * TODO: this could be optimized in the case where the label is "none", since the Point can
         * be omitted.
         */
        MultiGeometryPtr multiGeo(kmlFac->CreateMultiGeometry());
        multiGeo->add_geometry(lineString);
        multiGeo->add_geometry(point);

        PlacemarkPtr ptr = kmlFac->CreatePlacemark();
        ptr->set_geometry(multiGeo);
        ptr->set_name(edge.displayInfo->label);
        ptr->set_styleurl(get_edge_style(edge, edgenum));

        layer.folder->add_feature(ptr);

        // add additional placemarks for each label on this edge
        add_labels(edge.labels, (node1.gpsData->y + node2.gpsData->y)/2, (node1.gpsData->x + node2.gpsData->x)/2, (node1.gpsData->z + node2.gpsData->z)/2);
    }
}

} // end namespace


void write_kml(const WatcherGraph& graph, const std::string& outputFile)
{
    Render args(graph);
    args.start();

    kmlbase::File::WriteStringToFile(kmldom::SerializePretty(args.kml), outputFile);
}

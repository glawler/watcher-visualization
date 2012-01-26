#include <string>
#include <boost/cast.hpp>
#include <boost/foreach.hpp>

#include "marshalYAML.h"
#include "messageFactory.h"

using namespace watcher;
using namespace watcher::event;

// Color
YAML::Emitter &operator<<(YAML::Emitter &out, const watcher::Color &c) {
	out << YAML::Flow;
	out << YAML::BeginSeq << c.r << c.g << c.b << c.a << YAML::EndSeq;
}
void operator>>(YAML::Parser& p, watcher::Color &c) { 
	YAML::Node node;
	p.GetNextDocument(node); 
	node[0] >> c.r;
	node[1] >> c.g;
	node[2] >> c.b;
	node[3] >> c.a;
}

// NodeIdentifier
YAML::Emitter &operator<<(YAML::Emitter &out, const NodeIdentifier &nid) {
	// might want to use different encoding for V4 vs. v6? 
	out << YAML::BeginMap; 
	out << YAML::Key << "NodeID" << YAML::Value; 
	out << nid.to_string(); 
	out << YAML::EndMap; 
	return out; 
}
void operator>>(YAML::Parser& p, watcher::NodeIdentifier &nid) { 
	YAML::Node node;
	p.GetNextDocument(node); 
	std::string str; 
	node["NodeID"] >> str; 
	nid=watcher::NodeIdentifier::from_string(str); 
}


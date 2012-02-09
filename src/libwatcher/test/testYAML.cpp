/* Copyright 2012 SPARTA, a Parsons Company
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
/** 
 * @file testYAML.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2012-18-01
 */
#define BOOST_TEST_MODULE testYAML
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <yaml-cpp/yaml.h>

using namespace std;

namespace YAMLTest {
	struct base {
		base(int b_=0) : b(b_) { }
		virtual ~base() { }

		int b;

		virtual void operator<<(YAML::Emitter &e) {
			e << YAML::Key << "b" << YAML::Value << b;
		}
		virtual void operator>>(YAML::Node &n) {
			n["b"] >> b;
		}
		
		virtual YAML::Emitter &serialize(YAML::Emitter &e) const {
			// e << YAML::Comment("base"); 
			// e << YAML::Flow;
			// e << YAML::BeginMap;
			e << YAML::Key << "b" << YAML::Value << b;
			// e << YAML::EndMap; 
			return e; 
		}

		virtual YAML::Node &serialize(YAML::Node &n) {
			n["b"] >> b;
			return n;
		}
	};
	struct derived : public base {
		derived(int d_=0) : base(d_), d(d_+10), x(d_+20) { }
		virtual ~derived() { }

		int d;
		int x; 

		virtual void operator<<(YAML::Emitter &e) {
			e << YAML::BeginMap; 
			// this does not work:
			// n >> (dynamic_cast<base*>(this); 
			base::operator<<(e); 
			e << YAML::Key << "d" << YAML::Value << d;
			e << YAML::EndMap; 
		}

		virtual void operator>>(YAML::Node &n) {
			// this does not work:
			// n >> (dynamic_cast<base*>(this))
			base::operator>>(n); 
			n["d"] >> d;
		}

		virtual YAML::Emitter &serialize(YAML::Emitter &e) const {
			e << YAML::BeginDoc;
			// e << YAML::Comment("base"); 
			// e << YAML::Flow;
			e << YAML::BeginMap;
			base::serialize(e); 
			// e << YAML::Key << "b" << YAML::Value << b;
			// e << YAML::EndMap; 
			// e << YAML::Flow;
			// e << YAML::BeginMap;
			e << YAML::Key << "d" << YAML::Value << d;
			e << YAML::Key << "x" << YAML::Value << x;
			e << YAML::EndMap; 
			e << YAML::EndDoc; 
			return e; 
		}

		virtual YAML::Node &serialize(YAML::Node &n) {
			base::serialize(n); 
			n["d"] >> d;
			n["x"] >> x;
			return n;
		}
	}; 

	// the global functions are somewhat useless as
	// they are not virtual and the pointers
	// must be cast. 
	void operator<<(YAML::Emitter &e, base *b) {
		e << YAML::Key << "b" << YAML::Value << b->b;
	}
	void operator<<(YAML::Emitter &e, derived *d) {
		e << YAML::BeginMap; 
		e << dynamic_cast<base*>(d); 
		e << YAML::Key << "d" << YAML::Value << d->d;
		e << YAML::EndMap; 
	}
	void operator>>(YAML::Node &n, base *b) {
		n["b"] >> b->b;
	}
	void operator>>(YAML::Node &n, derived *d) {
		n >> dynamic_cast<base *>(d); 
		n["d"] >> d->d;
	}
}

using namespace YAMLTest;

BOOST_AUTO_TEST_CASE( global_operator_test )
{
	base *b=new derived(100); 
	derived *d=new derived(); 

	YAML::Emitter e;
	e << dynamic_cast<derived *>(b); 

	stringstream ss(e.c_str()); 
	YAML::Parser p(ss); 
	YAML::Node n;
	p.GetNextDocument(n);
	n >> d;

    BOOST_CHECK_EQUAL(b->b+10, d->d); 

	delete b;
	delete d; 
}

BOOST_AUTO_TEST_CASE( virtual_operator_test )
{
	base *b=new derived(100); 
	derived *d=new derived(); 

	YAML::Emitter e;
	// e << b; 		// should be d::operator<<()
	b->operator<<(e); 		// should be d::operator<<()

	stringstream ss(e.c_str()); 
	YAML::Parser p(ss); 
	YAML::Node n;
	p.GetNextDocument(n);
	n >> d;

    BOOST_CHECK_EQUAL(b->b+10, d->d); 

	delete b;
	delete d; 
}

BOOST_AUTO_TEST_CASE( shared_ptr_test )
{
	boost::shared_ptr<base> b(new derived(100)); 
	boost::shared_ptr<derived>d(new derived()); 

	YAML::Emitter e;
	// e << b; 		// should be d::operator<<()
	b->operator<<(e); 		// should be d::operator<<()

	stringstream ss(e.c_str()); 
	YAML::Parser p(ss); 
	YAML::Node n;
	p.GetNextDocument(n);
	n >> d.get(); // API leak

    BOOST_CHECK_EQUAL(b->b+10, d->d); 
}

BOOST_AUTO_TEST_CASE( serialize_test )
{
	base *b(new derived(100)); 
	derived *d(new derived()); 

	YAML::Emitter e;
	b->serialize(e); 

	stringstream ss(e.c_str()); 
	YAML::Parser p(ss); 
	YAML::Node node;
	p.GetNextDocument(node);
	d->serialize(node); 

    BOOST_CHECK_EQUAL(b->b+10, d->d); 
}

BOOST_AUTO_TEST_CASE( multiread )
{
	// Can we parse the same stringstream multiple times? 
	base *b(new derived(100)); 
	derived *d(new derived()); 
	derived *d2(new derived()); 

	YAML::Emitter e;
	b->serialize(e); 

	stringstream ss(e.c_str()); 
	streampos pos=ss.tellg(); 
	YAML::Parser p(ss); 
	YAML::Node node;
	p.GetNextDocument(node);
	d->serialize(node); 

    BOOST_CHECK_EQUAL(b->b+10, d->d); 

	ss.seekg(pos); 
	YAML::Parser p2(ss); 
	p2.GetNextDocument(node);
	d2->serialize(node); 

    BOOST_CHECK_EQUAL(b->b+10, d2->d); 
}

BOOST_AUTO_TEST_CASE( multiobject )
{
	// Can we parse the same stringstream multiple times? 
	base *b[] = { new derived(100), new derived(200), new derived(300) }; 
	derived *d[] = { new derived(), new derived(), new derived() }; 
	int array_size=sizeof(b)/sizeof(b[0]); 

	YAML::Emitter e;
	for (int i=0; i<array_size; i++) 
		b[i]->serialize(e); 

	stringstream ss(e.c_str()); 
	BOOST_TEST_MESSAGE("serialized data:"); 
	BOOST_TEST_MESSAGE(ss.str()); 

	// Only on parser per stream, see issue #148 for yaml-cpp. 
	YAML::Parser p(ss); 
	for (int i=0; i<sizeof(b)/sizeof(b[0]); i++) {
		cout << "Pre: " << ss.tellg() << endl; 
		YAML::Node node; 
		d[i]=new derived(); 
		p.GetNextDocument(node); 
		d[i]->serialize(node); 
		cout << "Post: " << ss.tellg() << endl; 
		cout << "val -  d:" << d[i]->d << endl;
	    BOOST_CHECK_EQUAL(b[i]->b+10, d[i]->d); 
	}
}

BOOST_AUTO_TEST_CASE( empty_sequence ) {
	YAML::Emitter e; 
	e << YAML::BeginDoc; 
	e << YAML::Flow; 
	e << YAML::BeginMap; 
	e << YAML::Key << "before" << YAML::Value << "before value"; 
	e << YAML::Key << "seq" << YAML::Value;
	e << YAML::Flow << YAML::BeginSeq;
	// nothing here. 
	e << YAML::EndSeq;
	e << YAML::Key << "after" << YAML::Value << "after value"; 
	e << YAML::EndMap; 
	e << YAML::EndDoc; 

	stringstream ss(e.c_str()); 
	BOOST_TEST_MESSAGE("serialized data:"); 
	BOOST_TEST_MESSAGE(ss.str()); 

	YAML::Parser p(ss); 
	YAML::Node n; 
	p.GetNextDocument(n); 
	string str; 
	n["before"] >> str; 
	BOOST_REQUIRE_EQUAL(str, "before value"); 
	const YAML::Node &seq=n["seq"]; 
	BOOST_REQUIRE(seq.size()==0); 
	for (unsigned i=0;i<seq.size();i++) {
		BOOST_REQUIRE(0); // boom.
	}
	n["after"] >> str; 
	BOOST_REQUIRE_EQUAL(str, "after value"); 
}

BOOST_AUTO_TEST_CASE( stl_containers ) {
	// woo c++0x (add -std=c++0x to CFLAGS for g++)
	vector<string> strs = {"hello", "world", "how", "are", "you?"}; // woo c++0x

	YAML::Emitter e; 
	e << YAML::Flow << YAML::BeginMap;
	e << YAML::Key << "strs" << YAML::Value << strs; 
	e << YAML::EndMap; 

	stringstream ss(e.c_str()); 
	BOOST_TEST_MESSAGE("serialized data:"); 
	BOOST_TEST_MESSAGE(ss.str()); 

	vector<string> new_strs; 
	YAML::Parser p(ss); 
	YAML::Node node;
	p.GetNextDocument(node); 
	node["strs"] >> new_strs; 

	BOOST_REQUIRE(new_strs == strs); 
}




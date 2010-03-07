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

#include "watcherSerialize.h"

#include <boost/lexical_cast.hpp>

#include "logger.h"
#include "watcherColors.h"
#include "colors.h"
#include <cstdio>
#include <string>

using namespace boost; 
using namespace std;

BOOST_CLASS_EXPORT(watcher::Color);

namespace watcher {

    using namespace colors;

    INIT_LOGGER(Color, "Color");

    Color::Color() : r(0), g(0), b(0), a(0)
    {
        // GTL - cannot log in here as there are static Colors and logging is not initialized before statics.
        // TRACE_ENTER();
        // TRACE_EXIT();
    }

    Color::Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) :
        r(R), g(G), b(B), a(A)
    {
        // GTL - cannot log in here as there are static Colors and logging is not initialized before statics.
        // TRACE_ENTER();
        // TRACE_EXIT();
    }

    Color::Color(const uint32_t &color) : r(0), g(0), b(0), a(0)
    {
        // GTL - cannot log in here as there are static Colors and logging is not initialized before statics.
        // TRACE_ENTER();
        r=color>>24;
        g=color>>16;
        b=color>>8;
        a=color;
        // TRACE_EXIT();
    }

    Color::Color(const Color &other) : r(other.r), g(other.g), b(other.b), a(other.a)
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }

    Color::~Color()
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }

    bool Color::fromString(const std::string &color)
    {
        // GTL - this may not be a great, read efficient, idea....
        // Should put them all into a BST, and query that.
        if(color=="white") *this=white;
        else if(color=="black") *this=black;
        else if(color=="blue") *this=blue;
        else if(color=="red") *this=red;
        else if(color=="green") *this=green;
        else if(color=="yellow") *this=yellow;
        else if(color=="orange") *this=orange;
        else if(color=="purple") *this=purple;
        else if(color=="snow") *this=snow;
        else if(color=="snow2") *this=snow2;
        else if(color=="snow3") *this=snow3;
        else if(color=="snow4") *this=snow4;
        else if(color=="ghostwhite") *this=ghostwhite;
        else if(color=="whitesmoke") *this=whitesmoke;
        else if(color=="gainsboro") *this=gainsboro;
        else if(color=="floralwhite") *this=floralwhite;
        else if(color=="oldlace") *this=oldlace;
        else if(color=="linen") *this=linen;
        else if(color=="antiquewhite") *this=antiquewhite;
        else if(color=="antiquewhite2") *this=antiquewhite2;
        else if(color=="antiquewhite3") *this=antiquewhite3;
        else if(color=="antiquewhite4") *this=antiquewhite4;
        else if(color=="papayawhip") *this=papayawhip;
        else if(color=="blanchedalmond") *this=blanchedalmond;
        else if(color=="bisque") *this=bisque;
        else if(color=="bisque2") *this=bisque2;
        else if(color=="bisque3") *this=bisque3;
        else if(color=="bisque4") *this=bisque4;
        else if(color=="peachpuff") *this=peachpuff;
        else if(color=="peachpuff2") *this=peachpuff2;
        else if(color=="peachpuff3") *this=peachpuff3;
        else if(color=="peachpuff4") *this=peachpuff4;
        else if(color=="navajowhite") *this=navajowhite;
        else if(color=="moccasin") *this=moccasin;
        else if(color=="cornsilk") *this=cornsilk;
        else if(color=="cornsilk2") *this=cornsilk2;
        else if(color=="cornsilk3") *this=cornsilk3;
        else if(color=="cornsilk4") *this=cornsilk4;
        else if(color=="ivory") *this=ivory;
        else if(color=="ivory2") *this=ivory2;
        else if(color=="ivory3") *this=ivory3;
        else if(color=="ivory4") *this=ivory4;
        else if(color=="lemonchiffon") *this=lemonchiffon;
        else if(color=="seashell") *this=seashell;
        else if(color=="seashell2") *this=seashell2;
        else if(color=="seashell3") *this=seashell3;
        else if(color=="seashell4") *this=seashell4;
        else if(color=="honeydew") *this=honeydew;
        else if(color=="honeydew2") *this=honeydew2;
        else if(color=="honeydew3") *this=honeydew3;
        else if(color=="honeydew4") *this=honeydew4;
        else if(color=="mintcream") *this=mintcream;
        else if(color=="azure") *this=azure;
        else if(color=="aliceblue") *this=aliceblue;
        else if(color=="lavender") *this=lavender;
        else if(color=="lavenderblush") *this=lavenderblush;
        else if(color=="mistyrose") *this=mistyrose;
        else if(color=="darkslategray") *this=darkslategray;
        else if(color=="dimgray") *this=dimgray;
        else if(color=="slategray") *this=slategray;
        else if(color=="lightslategray") *this=lightslategray;
        else if(color=="gray") *this=gray;
        else if(color=="lightgray") *this=lightgray;
        else if(color=="midnightblue") *this=midnightblue;
        else if(color=="navy") *this=navy;
        else if(color=="cornflowerblue") *this=cornflowerblue;
        else if(color=="darkslateblue") *this=darkslateblue;
        else if(color=="slateblue") *this=slateblue;
        else if(color=="mediumslateblue") *this=mediumslateblue;
        else if(color=="lightslateblue") *this=lightslateblue;
        else if(color=="mediumblue") *this=mediumblue;
        else if(color=="royalblue") *this=royalblue;
        else if(color=="dodgerblue") *this=dodgerblue;
        else if(color=="deepskyblue") *this=deepskyblue;
        else if(color=="skyblue") *this=skyblue;
        else if(color=="lightskyblue") *this=lightskyblue;
        else if(color=="steelblue") *this=steelblue;
        else if(color=="lightsteelblue") *this=lightsteelblue;
        else if(color=="lightblue") *this=lightblue;
        else if(color=="powderblue") *this=powderblue;
        else if(color=="paleturquoise") *this=paleturquoise;
        else if(color=="darkturquoise") *this=darkturquoise;
        else if(color=="mediumturquoise") *this=mediumturquoise;
        else if(color=="turquoise") *this=turquoise;
        else if(color=="cyan") *this=cyan;
        else if(color=="lightcyan") *this=lightcyan;
        else if(color=="cadetblue") *this=cadetblue;
        else if(color=="mediumaquamarine") *this=mediumaquamarine;
        else if(color=="aquamarine") *this=aquamarine;
        else if(color=="darkgreen") *this=darkgreen;
        else if(color=="darkolivegreen") *this=darkolivegreen;
        else if(color=="darkseagreen") *this=darkseagreen;
        else if(color=="seagreen") *this=seagreen;
        else if(color=="mediumseagreen") *this=mediumseagreen;
        else if(color=="lightseagreen") *this=lightseagreen;
        else if(color=="palegreen") *this=palegreen;
        else if(color=="springgreen") *this=springgreen;
        else if(color=="lawngreen") *this=lawngreen;
        else if(color=="chartreuse") *this=chartreuse;
        else if(color=="mediumspringgreen") *this=mediumspringgreen;
        else if(color=="greenyellow") *this=greenyellow;
        else if(color=="limegreen") *this=limegreen;
        else if(color=="yellowgreen") *this=yellowgreen;
        else if(color=="forestgreen") *this=forestgreen;
        else if(color=="olivedrab") *this=olivedrab;
        else if(color=="darkkhaki") *this=darkkhaki;
        else if(color=="khaki") *this=khaki;
        else if(color=="palegoldenrod") *this=palegoldenrod;
        else if(color=="lightgoldenrodyellow") *this=lightgoldenrodyellow;
        else if(color=="lightyellow") *this=lightyellow;
        else if(color=="gold") *this=gold;
        else if(color=="lightgoldenrod") *this=lightgoldenrod;
        else if(color=="goldenrod") *this=goldenrod;
        else if(color=="darkgoldenrod") *this=darkgoldenrod;
        else if(color=="rosybrown") *this=rosybrown;
        else if(color=="indianred") *this=indianred;
        else if(color=="saddlebrown") *this=saddlebrown;
        else if(color=="sienna") *this=sienna;
        else if(color=="peru") *this=peru;
        else if(color=="burlywood") *this=burlywood;
        else if(color=="beige") *this=beige;
        else if(color=="wheat") *this=wheat;
        else if(color=="sandybrown") *this=sandybrown;
        else if(color=="tan") *this=tan;
        else if(color=="chocolate") *this=chocolate;
        else if(color=="firebrick") *this=firebrick;
        else if(color=="brown") *this=brown;
        else if(color=="darksalmon") *this=darksalmon;
        else if(color=="salmon") *this=salmon;
        else if(color=="lightsalmon") *this=lightsalmon;
        else if(color=="darkorange") *this=darkorange;
        else if(color=="coral") *this=coral;
        else if(color=="lightcoral") *this=lightcoral;
        else if(color=="tomato") *this=tomato;
        else if(color=="orangered") *this=orangered;
        else if(color=="hotpink") *this=hotpink;
        else if(color=="deeppink") *this=deeppink;
        else if(color=="pink") *this=pink;
        else if(color=="lightpink") *this=lightpink;
        else if(color=="palevioletred") *this=palevioletred;
        else if(color=="maroon") *this=maroon;
        else if(color=="mediumvioletred") *this=mediumvioletred;
        else if(color=="violetred") *this=violetred;
        else if(color=="violet") *this=violet;
        else if(color=="plum") *this=plum;
        else if(color=="orchid") *this=orchid;
        else if(color=="mediumorchid") *this=mediumorchid;
        else if(color=="darkorchid") *this=darkorchid;
        else if(color=="darkviolet") *this=darkviolet;
        else if(color=="blueviolet") *this=blueviolet;
        else if(color=="mediumpurple") *this=mediumpurple;
        else if(color=="thistle") *this=thistle;
        else
        {
            // basic sanity checking, may be a better way to do this.
            if (color[0]!='0' && color[1]!='x' && color.length()!=10)
            {
                TRACE_EXIT_RET("false"); 
                return false;
            }
            istringstream is(color); 
            uint32_t vals;
            is >> hex >> vals;
            r=vals>>24;
            g=vals>>16;
            b=vals>>8;
            a=vals;
        }
        TRACE_EXIT_RET("true");
        return true;
    }

    bool Color::operator==(const Color &other) const
    {
        TRACE_ENTER();
        bool retVal=
            r==other.r && 
            g==other.g && 
            b==other.b;
        TRACE_EXIT_RET((retVal ? "true" : "false"));
        return retVal;
    }

    Color &Color::operator=(const Color &other)
    {
        TRACE_ENTER();
        r=other.r;
        g=other.g;
        b=other.b;
        a=other.a;
        TRACE_EXIT();
        return *this;
    }

    std::string Color::toString() const
    {
        TRACE_ENTER();
        TRACE_EXIT();
        return Color::toString(Color(r,g,b,a)); 
    }

    std::ostream &Color::toStream(std::ostream &out) const
    {
        TRACE_ENTER();
        out << toString(*this); 
        TRACE_EXIT();
        return out;
    }

    ostream &operator<<(ostream &out, const Color &c)
    {
        c.operator<<(out);
        return out;
    }

    std::string Color::toString(const Color &c)
    {   
        // GTL - this may not be a great, read efficient, idea....
        // Should put them all into a BST, and query that.
        if(c==white) return string("white");
        else if(c==black) return string("black");
        else if(c==blue) return string("blue");
        else if(c==green) return string("green");
        else if(c==red) return string("red");
        else if(c==yellow) return string("yellow");
        else if(c==orange) return string("orange");
        else if(c==snow) return string("snow");
        else if(c==snow2) return string("snow2");
        else if(c==snow3) return string("snow3");
        else if(c==snow4) return string("snow4");
        else if(c==ghostwhite) return string("ghostwhite");
        else if(c==whitesmoke) return string("whitesmoke");
        else if(c==gainsboro) return string("gainsboro");
        else if(c==floralwhite) return string("floralwhite");
        else if(c==oldlace) return string("oldlace");
        else if(c==linen) return string("linen");
        else if(c==antiquewhite) return string("antiquewhite");
        else if(c==antiquewhite2) return string("antiquewhite2");
        else if(c==antiquewhite3) return string("antiquewhite3");
        else if(c==antiquewhite4) return string("antiquewhite4");
        else if(c==papayawhip) return string("papayawhip");
        else if(c==blanchedalmond) return string("blanchedalmond");
        else if(c==bisque) return string("bisque");
        else if(c==bisque2) return string("bisque2");
        else if(c==bisque3) return string("bisque3");
        else if(c==bisque4) return string("bisque4");
        else if(c==peachpuff) return string("peachpuff");
        else if(c==peachpuff2) return string("peachpuff2");
        else if(c==peachpuff3) return string("peachpuff3");
        else if(c==peachpuff4) return string("peachpuff4");
        else if(c==navajowhite) return string("navajowhite");
        else if(c==moccasin) return string("moccasin");
        else if(c==cornsilk) return string("cornsilk");
        else if(c==cornsilk2) return string("cornsilk2");
        else if(c==cornsilk3) return string("cornsilk3");
        else if(c==cornsilk4) return string("cornsilk4");
        else if(c==ivory) return string("ivory");
        else if(c==ivory2) return string("ivory2");
        else if(c==ivory3) return string("ivory3");
        else if(c==ivory4) return string("ivory4");
        else if(c==lemonchiffon) return string("lemonchiffon");
        else if(c==seashell) return string("seashell");
        else if(c==seashell2) return string("seashell2");
        else if(c==seashell3) return string("seashell3");
        else if(c==seashell4) return string("seashell4");
        else if(c==honeydew) return string("honeydew");
        else if(c==honeydew2) return string("honeydew2");
        else if(c==honeydew3) return string("honeydew3");
        else if(c==honeydew4) return string("honeydew4");
        else if(c==mintcream) return string("mintcream");
        else if(c==azure) return string("azure");
        else if(c==aliceblue) return string("aliceblue");
        else if(c==lavender) return string("lavender");
        else if(c==lavenderblush) return string("lavenderblush");
        else if(c==mistyrose) return string("mistyrose");
        else if(c==darkslategray) return string("darkslategray");
        else if(c==dimgray) return string("dimgray");
        else if(c==slategray) return string("slategray");
        else if(c==lightslategray) return string("lightslategray");
        else if(c==gray) return string("gray");
        else if(c==lightgray) return string("lightgray");
        else if(c==midnightblue) return string("midnightblue");
        else if(c==navy) return string("navy");
        else if(c==cornflowerblue) return string("cornflowerblue");
        else if(c==darkslateblue) return string("darkslateblue");
        else if(c==slateblue) return string("slateblue");
        else if(c==mediumslateblue) return string("mediumslateblue");
        else if(c==lightslateblue) return string("lightslateblue");
        else if(c==mediumblue) return string("mediumblue");
        else if(c==royalblue) return string("royalblue");
        else if(c==dodgerblue) return string("dodgerblue");
        else if(c==deepskyblue) return string("deepskyblue");
        else if(c==skyblue) return string("skyblue");
        else if(c==lightskyblue) return string("lightskyblue");
        else if(c==steelblue) return string("steelblue");
        else if(c==lightsteelblue) return string("lightsteelblue");
        else if(c==lightblue) return string("lightblue");
        else if(c==powderblue) return string("powderblue");
        else if(c==paleturquoise) return string("paleturquoise");
        else if(c==darkturquoise) return string("darkturquoise");
        else if(c==mediumturquoise) return string("mediumturquoise");
        else if(c==turquoise) return string("turquoise");
        else if(c==cyan) return string("cyan");
        else if(c==lightcyan) return string("lightcyan");
        else if(c==cadetblue) return string("cadetblue");
        else if(c==mediumaquamarine) return string("mediumaquamarine");
        else if(c==aquamarine) return string("aquamarine");
        else if(c==darkgreen) return string("darkgreen");
        else if(c==darkolivegreen) return string("darkolivegreen");
        else if(c==darkseagreen) return string("darkseagreen");
        else if(c==seagreen) return string("seagreen");
        else if(c==mediumseagreen) return string("mediumseagreen");
        else if(c==lightseagreen) return string("lightseagreen");
        else if(c==palegreen) return string("palegreen");
        else if(c==springgreen) return string("springgreen");
        else if(c==lawngreen) return string("lawngreen");
        else if(c==chartreuse) return string("chartreuse");
        else if(c==mediumspringgreen) return string("mediumspringgreen");
        else if(c==greenyellow) return string("greenyellow");
        else if(c==limegreen) return string("limegreen");
        else if(c==yellowgreen) return string("yellowgreen");
        else if(c==forestgreen) return string("forestgreen");
        else if(c==olivedrab) return string("olivedrab");
        else if(c==darkkhaki) return string("darkkhaki");
        else if(c==khaki) return string("khaki");
        else if(c==palegoldenrod) return string("palegoldenrod");
        else if(c==lightgoldenrodyellow) return string("lightgoldenrodyellow");
        else if(c==lightyellow) return string("lightyellow");
        else if(c==gold) return string("gold");
        else if(c==lightgoldenrod) return string("lightgoldenrod");
        else if(c==goldenrod) return string("goldenrod");
        else if(c==darkgoldenrod) return string("darkgoldenrod");
        else if(c==rosybrown) return string("rosybrown");
        else if(c==indianred) return string("indianred");
        else if(c==saddlebrown) return string("saddlebrown");
        else if(c==sienna) return string("sienna");
        else if(c==peru) return string("peru");
        else if(c==burlywood) return string("burlywood");
        else if(c==beige) return string("beige");
        else if(c==wheat) return string("wheat");
        else if(c==sandybrown) return string("sandybrown");
        else if(c==tan) return string("tan");
        else if(c==chocolate) return string("chocolate");
        else if(c==firebrick) return string("firebrick");
        else if(c==brown) return string("brown");
        else if(c==darksalmon) return string("darksalmon");
        else if(c==salmon) return string("salmon");
        else if(c==lightsalmon) return string("lightsalmon");
        else if(c==darkorange) return string("darkorange");
        else if(c==coral) return string("coral");
        else if(c==lightcoral) return string("lightcoral");
        else if(c==tomato) return string("tomato");
        else if(c==orangered) return string("orangered");
        else if(c==hotpink) return string("hotpink");
        else if(c==deeppink) return string("deeppink");
        else if(c==pink) return string("pink");
        else if(c==lightpink) return string("lightpink");
        else if(c==palevioletred) return string("palevioletred");
        else if(c==maroon) return string("maroon");
        else if(c==mediumvioletred) return string("mediumvioletred");
        else if(c==violetred) return string("violetred");
        else if(c==violet) return string("violet");
        else if(c==plum) return string("plum");
        else if(c==orchid) return string("orchid");
        else if(c==mediumorchid) return string("mediumorchid");
        else if(c==darkorchid) return string("darkorchid");
        else if(c==darkviolet) return string("darkviolet");
        else if(c==blueviolet) return string("blueviolet");
        else if(c==purple) return string("purple");
        else if(c==mediumpurple) return string("mediumpurple");
        else if(c==thistle) return string("thistle");
        else
        {
            char buf[12];
            snprintf(buf, sizeof(buf), "0x%02x%02x%02x%02x", c.r, c.g, c.b, c.a); 
            return string(buf); 
        }
    }
}


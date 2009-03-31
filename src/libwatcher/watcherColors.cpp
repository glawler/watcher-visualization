#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include <boost/lexical_cast.hpp>

#include "watcherColors.h"
#include <string>

using namespace boost; 
using namespace std;

BOOST_CLASS_EXPORT_GUID(watcher::event::Color, "watcher::event::Color");

namespace watcher {
    namespace event {
        INIT_LOGGER(Color, "Color");

        const Color Color::black(0x00, 0x00, 0x00, 0x00);
        const Color Color::white(0xff, 0xff, 0xff, 0x00);
        const Color Color::violet(0xee, 0x82, 0xee, 0x00); 
        const Color Color::indigo(0x4b, 0x00, 0x82, 0x00); 
        const Color Color::blue(0x00, 0x00, 0xff, 0x00);
        const Color Color::green(0x00, 0x80, 0x00, 0x00);
        const Color Color::yellow(0xff, 0xff, 0x00, 0x00);
        const Color Color::orange(0xff, 0xa5, 0x00, 0x00);
        const Color Color::red(0xff, 0x00, 0x00, 0x00);
        const Color Color::darkblue(0x00, 0x00, 0x8b, 0x00);
        const Color Color::magenta(0xff, 0x00, 0xff, 0x00);
        const Color Color::gold(0xff, 0xd7, 0xff, 0x00);
        const Color Color::turquoise(0x40, 0xe0, 0xd0, 0x00);
        const Color Color::brown(0xa5, 0x2a, 0x2a, 0x00);
        const Color Color::deeppink(0xff, 0x14, 0x93, 0x00);

        Color::Color() : r(0), g(0), b(0), a(0)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        Color::Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) :
            r(R), g(G), b(B), a(A)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        Color::Color(const uint32_t &color) : r(0), g(0), b(0), a(0)
        {
            TRACE_ENTER();
            r=color>>24;
            g=color>>16;
            b=color>>8;
            a=color;
            TRACE_EXIT();
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
            if (color=="black") *this=Color::black;
            else if(color=="white") *this=Color::white;
            else if(color=="violet") *this=Color::violet;
            else if(color=="indigo") *this=Color::indigo;
            else if(color=="blue") *this=Color::blue;
            else if(color=="green") *this=Color::green;
            else if(color=="yellow") *this=Color::yellow;
            else if(color=="orange") *this=Color::orange;
            else if(color=="red") *this=Color::red;
            else if(color=="darkblue") *this=Color::darkblue;
            else if(color=="gold") *this=Color::gold;
            else if(color=="turquoise") *this=Color::turquoise;
            else if(color=="brown") *this=Color::brown;
            else if(color=="deeppink") *this=Color::deeppink;
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
                b==other.b && 
                a==other.a;
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

        std::ostream &Color::toStream(std::ostream &out) const
        {
            TRACE_ENTER();
            out << toString(*this); 
            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const Color &c)
        {
            TRACE_ENTER();
            c.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        std::string Color::toString(const Color &c)
        {
            if (c==Color::black) return string("black"); 
            else if (c==Color::white) return string("white"); 
            else if (c==Color::violet) return string("violet"); 
            else if (c==Color::indigo) return string("indigo"); 
            else if (c==Color::blue) return string("blue"); 
            else if (c==Color::green) return string("green"); 
            else if (c==Color::yellow) return string("yellow"); 
            else if (c==Color::orange) return string("orange"); 
            else if (c==Color::red) return string("red"); 
            else if (c==Color::darkblue) return string("darkblue"); 
            else if (c==Color::magenta) return string("magenta"); 
            else if (c==Color::gold) return string("gold"); 
            else if (c==Color::turquoise) return string("turquoise"); 
            else if (c==Color::brown) return string("brown"); 
            else if (c==Color::deeppink) return string("deeppink"); 
            else
            {
                char buf[12];
                snprintf(buf, sizeof(buf), "0x%02x%02x%02x%02x", c.r, c.g, c.b, c.a); 
                return string(buf); 
            }
        }

    }
}


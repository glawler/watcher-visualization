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

/**
 * @file watcherColors.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_COLORS_H
#define WATCHER_COLORS_H

#include "logger.h"
#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

namespace watcher
{
    namespace event {
        /// Watcher color is RGB+alpha
        class Color {
            public:
                /** A few default colors. Plenty more at: 
                 * http://htmlhelp.com/cgi-bin/color.cgi
                 * Feel free to add to this list.
                 */
                static const Color black;
                static const Color white;

                static const Color violet;
                static const Color indigo;
                static const Color blue;
                static const Color green;
                static const Color yellow;
                static const Color orange;
                static const Color red;

                static const Color darkblue;
                static const Color magenta;
                static const Color gold;
                static const Color turquoise;
                static const Color brown;
                static const Color deeppink;

                // Static toString()
                /** Convert Color to human readable string value.
                 * @param[in] c color to convert
                 */
                static std::string toString(const Color &c); 

                // Member toString();
                /** Convert Color to human readable string value. */
                std::string toString() const; 

                //
                // Now the real Color class begins...
                //
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a;

                Color();
                //Color(const unsigned char rgba[4]); 

                /** Construct a color from RGB and Alpha parameters.
                 * @param[in] r red
                 * @param[in] g green
                 * @param[in] b blue
                 * @param[in] a alpha
                 */
                Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

                /** Construct color from 32-bit RGB+alpha value (8-bits each)
                 * @param[in] color rgba value
                 */
                Color(const uint32_t &color);       // color == rgba

                /** color = "black", "white", etc or hex value, i.e. "0xff34ds00"
                 * This constructor will throw boost::bad_lexical_cast if the string is 
                 * not properly formatted (able to be converted into a uint32_t). 
                 * @retval false on badly formatted string.
                 */
                bool fromString(const std::string &color);    

                Color(const Color &other);
                ~Color(); 

                /** Compare two colors for equality */
                bool operator==(const Color &c) const;
                /** Compare two colors for inequality */
                bool operator!=(const Color &c) const { return !(*this==c); }
                Color &operator=(const Color &other);

                /** write the human readable color description to an output stream.
                 * @param out output stream
                 * @return reference to output stream
                 */
                std::ostream &toStream(std::ostream &out) const;
                /** write the human readable color description to an output stream.
                 * @param out output stream
                 * @return reference to output stream
                 */
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            protected:
            private:
                friend class boost::serialization::access;
                template <typename Archive>
                void serialize(Archive & ar, const unsigned int /* file_version */)
                {
                    TRACE_ENTER();
                    ar & r;
                    ar & g;
                    ar & b;
                    ar & a;
                    TRACE_EXIT();
                }
                DECLARE_LOGGER();
        };

        // global operators and typedefs.
        typedef boost::shared_ptr<Color> ColorPtr;

        /** write the human readable color description to an output stream.
         * @param out output stream
         * @param[in] c Color to convert
         * @return reference to output stream
         */
        std::ostream &operator<<(std::ostream &out, const Color &c);
    }
}
#endif // WATCHER_COLORS_H

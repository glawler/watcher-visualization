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
 * @file colors.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-09-02
 */
#include "colors.h"

namespace watcher 
{
    /** A namespace to declare predefined watcher::colors. */
    namespace colors 
    {
        Color snow(0xfffafaff);
        Color snow2(0xeee9e9ff);
        Color snow3(0xcdc9c9ff);
        Color snow4(0x8b8989ff);
        Color ghostwhite(0xf8f8ffff);
        Color whitesmoke(0xf5f5f5ff);
        Color gainsboro(0xdccdcff);
        Color floralwhite(0xfffaf0ff);
        Color oldlace(0xfdf5e6ff);
        Color linen(0xfaf0e6ff);
        Color antiquewhite(0xfaebd7ff);
        Color antiquewhite2(0xeedfccff);
        Color antiquewhite3(0xcdc0b0ff);
        Color antiquewhite4(0x8b8378ff);
        Color papayawhip(0xffefd5ff);
        Color blanchedalmond(0xffebcdff);
        Color bisque(0xffe4c4ff);
        Color bisque2(0xeed5b7ff);
        Color bisque3(0xcdb79eff);
        Color bisque4(0x8b7d6bff);
        Color peachpuff(0xffdab9ff);
        Color peachpuff2(0xeecbadff);
        Color peachpuff3(0xcdaf95ff);
        Color peachpuff4(0x8b7765ff);
        Color navajowhite(0xffdeadff);
        Color moccasin(0xffe4b5ff);
        Color cornsilk(0xfff8dcff);
        Color cornsilk2(0xeee8dcff);
        Color cornsilk3(0xcdc8b1ff);
        Color cornsilk4(0x8b8878ff);
        Color ivory(0xfffff0ff);
        Color ivory2(0xeeeee0ff);
        Color ivory3(0xcdcdc1ff);
        Color ivory4(0x8b8b83ff);
        Color lemonchiffon(0xfffacdff);
        Color seashell(0xfff5eeff);
        Color seashell2(0xeee5deff);
        Color seashell3(0xcdc5bfff);
        Color seashell4(0x8b8682ff);
        Color honeydew(0xf0fff0ff);
        Color honeydew2(0xe0eee0ff);
        Color honeydew3(0xc1cdc1ff);
        Color honeydew4(0x838b83ff);
        Color mintcream(0xf5fffaff);
        Color azure(0xf0ffffff);
        Color aliceblue(0xf0f8ffff);
        Color lavender(0xe6e6faff);
        Color lavenderblush(0xfff0f5ff);
        Color mistyrose(0xffe4e1ff);
        Color white(0xffffffff);
        Color black(0x000000ff);
        Color darkslategray(0x2f4f4fff);
        Color dimgray(0x696969ff);
        Color slategray(0x708090ff);
        Color lightslategray(0x778899ff);
        Color gray(0xbebebeff);
        Color lightgray(0xd3d3d3ff);
        Color midnightblue(0x191970ff);
        Color navy(0x000080ff);
        Color cornflowerblue(0x6495edff);
        Color darkslateblue(0x483d8bff);
        Color slateblue(0x6a5acdff);
        Color mediumslateblue(0x7b68eeff);
        Color lightslateblue(0x8470ffff);
        Color mediumblue(0x0000cdff);
        Color royalblue(0x4169e1ff);
        Color blue(0x0000ffff);
        Color dodgerblue(0x1e90ffff);
        Color deepskyblue(0x00bfffff);
        Color skyblue(0x87ceebff);
        Color lightskyblue(0x87cefaff);
        Color steelblue(0x4682b4ff);
        Color lightsteelblue(0xb0c4deff);
        Color lightblue(0xadd8e6ff);
        Color powderblue(0xb0e0e6ff);
        Color paleturquoise(0xafeeeeff);
        Color darkturquoise(0x00ced1ff);
        Color mediumturquoise(0x48d1ccff);
        Color turquoise(0x40e0d0ff);
        Color cyan(0x00ffffff);
        Color lightcyan(0xe0ffffff);
        Color cadetblue(0x5f9ea0ff);
        Color mediumaquamarine(0x66cdaaff);
        Color aquamarine(0x7fffd4ff);
        Color green(0x00ff00ff);
        Color darkgreen(0x006400ff);
        Color darkolivegreen(0x556b2fff);
        Color darkseagreen(0x8fbc8fff);
        Color seagreen(0x2e8b57ff);
        Color mediumseagreen(0x3cb371ff);
        Color lightseagreen(0x20b2aaff);
        Color palegreen(0x98fb98ff);
        Color springgreen(0x00ff7fff);
        Color lawngreen(0x7cfc00ff);
        Color chartreuse(0x7fff00ff);
        Color mediumspringgreen(0x00fa9aff);
        Color greenyellow(0xadff2fff);
        Color limegreen(0x32cd32ff);
        Color yellowgreen(0x9acd32ff);
        Color forestgreen(0x228b22ff);
        Color olivedrab(0x6b8e23ff);
        Color darkkhaki(0xbdb76bff);
        Color khaki(0xf0e68cff);
        Color palegoldenrod(0xeee8aaff);
        Color lightgoldenrodyellow(0xfafad2ff);
        Color lightyellow(0xffffe0ff);
        Color yellow(0xffff00ff);
        Color gold(0xffd700ff);
        Color lightgoldenrod(0xeedd82ff);
        Color goldenrod(0xdaa520ff);
        Color darkgoldenrod(0xb8860bff);
        Color rosybrown(0xbc8f8fff);
        Color indianred(0xcd5c5cff);
        Color saddlebrown(0x8b4513ff);
        Color sienna(0xa0522dff);
        Color peru(0xcd853fff);
        Color burlywood(0xdeb887ff);
        Color beige(0xf5f5dcff);
        Color wheat(0xf5deb3ff);
        Color sandybrown(0xf4a460ff);
        Color tan(0xd2b48cff);
        Color chocolate(0xd2691eff);
        Color firebrick(0xb22222ff);
        Color brown(0xa52a2aff);
        Color darksalmon(0xe9967aff);
        Color salmon(0xfa8072ff);
        Color lightsalmon(0xffa07aff);
        Color orange(0xffa500ff);
        Color darkorange(0xff8c00ff);
        Color coral(0xff7f50ff);
        Color lightcoral(0xf08080ff);
        Color tomato(0xff6347ff);
        Color orangered(0xff4500ff);
        Color red(0xff0000ff);
        Color hotpink(0xff69b4ff);
        Color deeppink(0xff1493ff);
        Color pink(0xffc0cbff);
        Color lightpink(0xffb6c1ff);
        Color palevioletred(0xdb7093ff);
        Color maroon(0xb03060ff);
        Color mediumvioletred(0xc71585ff);
        Color violetred(0xd02090ff);
        Color violet(0xee82eeff);
        Color plum(0xdda0ddff);
        Color orchid(0xda70d6ff);
        Color mediumorchid(0xba55d3ff);
        Color darkorchid(0x9932ccff);
        Color darkviolet(0x9400d3ff);
        Color blueviolet(0x8a2be2ff);
        Color purple(0xa020f0ff);
        Color mediumpurple(0x9370dbff);
        Color thistle(0xd8bfd8ff);
    }
}

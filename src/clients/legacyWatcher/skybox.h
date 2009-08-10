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
 * @file skybox.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef THISISASKYBOXYOUBETCHA_H
#define THISISASKYBOXYOUBETCHA_H

#include <GL/gl.h>
#include "logger.h"

namespace watcher
{
    class Skybox
    {
        public:
            static Skybox *getSkybox();

            void drawSkybox(GLfloat camX, GLfloat camy, GLfloat camz); 

        protected:
        private:
            // No copies thanks.
            Skybox();
            ~Skybox();
            Skybox(const Skybox &);
            Skybox &operator=(const Skybox &); 

            GLfloat width;

            DECLARE_LOGGER();
    }; 


} // end namespace watcher

#endif // THISISASKYBOXYOUBETCHA_H

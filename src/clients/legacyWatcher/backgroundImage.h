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
 * @file backgroundImage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef WATCHER_BG_IMAGRE_H
#define WATCHER_BG_IMAGRE_H
#include <string>
#include "declareLogger.h"
#include <GL/gl.h>

namespace watcher 
{
    class BackgroundImage
    {
        public:
            // is a singleton
            static BackgroundImage &getInstance(); 

            /** 
             * Load a file into the backgroundImage instance. 
             * Parses the filename and calls either loadBMP() or
             * loadPPM(). No other formats are currently supported. 
             *
             * @return bool, true if image loaded, false otherwise.
             */
            bool loadImageFile(const std::string &filename);

            //
            void setDrawingCoords(GLfloat xcoord, GLfloat width, GLfloat ycoord, GLfloat height, GLfloat z);
            void getDrawingCoords(GLfloat &xcoord, GLfloat &width, GLfloat &ycoord, GLfloat &height, GLfloat &z);

            // Draw the background using opengl and the set coords.
            void drawImage(); 

            // if true, center the image next time it is drawn. 
            // (Hacky - only used when executing the self-centering code in the watcher)
            void centerImage(bool); 
            bool centerImage() const; 

            std::string getImageFile() const { return imageFile; }

        protected:

        private:
            DECLARE_LOGGER();

            bool imageLoaded;
            bool imageCenter;

            void setupTexture();

            GLubyte *imageData;
            GLfloat minx, miny, xoffset, yoffset, z;

            int imageWidth, imageHeight;

            GLfloat envColor[4];
            GLfloat borderColor[4];


            BackgroundImage();
            virtual ~BackgroundImage(); 

            std::string imageFile;

			void loadSDLSurfaceTexture(const std::string &filename); 
			bool genDisplayList(); 
			GLuint itemList; 
        	GLuint textureIntID;
    };
}

#endif // WATCHER_BG_IMAGRE_H

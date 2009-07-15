/** 
 * @file backgroundImage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef WATCHER_BG_IMAGRE_H
#define WATCHER_BG_IMAGRE_H

#include "logger.h"

namespace watcher 
{
    class BackgroundImage
    {
        public:
            // is a singleton
            static BackgroundImage &getInstance(); 

            // returns false if unable to load image.
            // Or bad image format.
            bool loadPPMFile(const char *filename);

            // Loads a BMP/DIB file, returns false on error
            bool loadBMPFile(const char *filename);

            //
            void setDrawingCoords(GLfloat xcoord, GLfloat width, GLfloat ycoord, GLfloat height, GLfloat z);
            void getDrawingCoords(GLfloat &xcoord, GLfloat &width, GLfloat &ycoord, GLfloat &height, GLfloat &z);

            // Draw the background using opengl and the set coords.
            void drawImage(); 

            // if true, center the image next time it is drawn. 
            // (Hacky - only used when executing the self-centering code in the watcher)
            void centerImage(bool); 
            bool centerImage() const; 

        protected:

        private:

            bool imageLoaded;
            bool imageCenter;

            void setupTexture();

            GLubyte *imageData;
            GLfloat minx, miny, xoffset, yoffset, z;

            int imageWidth, imageHeight;

            GLfloat envColor[4];
            GLfloat borderColor[4];

            GLenum imageFormat;
            GLenum imageType;

            DECLARE_LOGGER();

            BackgroundImage();
            virtual ~BackgroundImage(); 
    };
}

#endif // WATCHER_BG_IMAGRE_H

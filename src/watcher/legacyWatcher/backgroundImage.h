#ifndef WATCHER_BG_IMAGRE_H
#define WATCHER_BG_IMAGRE_H

#include "des.h" 
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
            void setDrawingCoords(GLfloat minx, GLfloat maxx, GLfloat miny, GLfloat maxy, GLfloat z);
            void getDrawingCoords(GLfloat &minx, GLfloat &maxx, GLfloat &miny, GLfloat &maxy, GLfloat &z);

            // Draw the background using opengl and the set coords.
            void drawImage(); 

        protected:

        private:

            void setupTexture();

            GLubyte *imageData;
            GLfloat minx, miny, maxx, maxy, z;

            int imageWidth, imageHeight;

            GLfloat envColor[4];
            GLfloat borderColor[4];

            GLenum imageFormat;
            GLenum imageType;

            GLuint textureId;

            DECLARE_LOGGER();

            BackgroundImage();
            virtual ~BackgroundImage(); 
    };
}

#endif // WATCHER_BG_IMAGRE_H

#ifndef WATCHER_BG_IMAGRE_H
#define WATCHER_BG_IMAGRE_H

#include "des.h" 
#include "logger.h"

namespace watcher 
{
    class BackgroundImage
    {
        public:
            BackgroundImage();
            virtual ~BackgroundImage(); 

            // returns false if unable to load image.
            // Or bad image format.
            bool loadPPMFile(const char *filename);

            void drawImage(GLfloat minx, GLfloat maxx, GLfloat miny, GLfloat maxy, GLfloat z);

        protected:

        private:

            void setupTexture();

            GLubyte *imageData;
            int width;
            int height;

            GLfloat envColor[4];
            GLfloat borderColor[4];

            DECLARE_LOGGER();
    };
}

#endif // WATCHER_BG_IMAGRE_H

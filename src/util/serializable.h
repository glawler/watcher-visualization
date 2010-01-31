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
 * @file serializable.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2010-01-31
 */
#ifndef THE_BOOST_SERIALIZE_CLASS_IS_A_REAL_PITA
#define THE_BOOST_SERIALIZE_CLASS_IS_A_REAL_PITA

#include <cstddef>      // C++ standard syas this is the include for size_t define
#include <stdint.h>     // uint*_t types. Is platform specific
#include <time.h>       // time_t define. 

// This doesn't really belong in watcher namespace as it's not watcher specific
namespace watcher {
    /** 
     * @class Serializable
     *
     * An abstract base class that defines serialization primatives.
     *
     */
    class Serializable {
        public:

            /** serialize
             * This pure virtual function takes a buffer and buffer
             * size and serializes the class into the buffer. If the 
             * buffer is not large enough, false is returned and the 
             * bufferSize argument contains the minimum size required
             * to serialize the class instance. 
             *
             * @param[IN/OUT] buffer The buffer to serialize the class instance into. 
             * If successful this pointer will point to the end of the serialized data.
             * @param[IN/OUT] bufferSize The size of the buffer. If the buffer
             * is not large enough, false is returned, and the minimum size
             * needed is put in bufferSize.
             * @return true on success, false otherwise
             */
            virtual bool serialize(unsigned char *&buffer, size_t &bufferSize) const = 0;

            /** unserialize
             * This function unserializes the class instance. 
             * @param[IN] buffer The data to unserialize from. If successful, this
             * pointer will point to the end of the data used to unserialize.
             * @param[IN] bufferSize the number of bytes in the buffer. If
             * successful, this will contain the number of bytes left in the buffer.
             * @return true on success, false otherwise
             */
            virtual bool unserialize(unsigned char *&buffer, size_t &bufferSize) = 0;

            /** serialBufferSize
             * This function returns how many bytes are needed to serialize this class 
             * instance.
             * @return size_t The number of bytes needed to serialize the class.
             */
            virtual size_t serialBufferSize(void) const = 0;

        protected:

            // utils to read and write into a binary buffer
            // These functions will read/write arbitrary bytes to the buffer passed
            // and increment the buffer pointer by that size. 
            // The 16 and 32 write/read always assume host byte order.
            static void writeBuffer(const uint8_t &value, void *&buffer);
            static void writeBuffer(const uint16_t &value, void *&buffer);
            static void writeBuffer(const uint32_t &value, void *&buffer);
            static void writeBuffer(const time_t &value, void *&buffer);

            static void write8(const uint8_t &value, void *&buffer);
            static void write16(const uint16_t &value, void *&buffer);
            static void write32(const uint32_t &value, void *&buffer);
            static void writeTimeT(const time_t &value, void *&buffer);

            // These read methods jsut read the data. 
            static void readBuffer(uint8_t &value, void *&buffer);
            static void readBuffer(uint16_t &value, void *&buffer);
            static void readBuffer(uint32_t &value, void *&buffer);
            static void readBuffer(time_t &value, void *&buffer);

            static void read8(uint8_t &value, void *&buffer);
            static void read16(uint16_t &value, void *&buffer);
            static void read32(uint32_t &value, void *&buffer);
            static void readTimeT(time_t &value, void *&buffer);

        private:
    };
}

#endif // THE_BOOST_SERIALIZE_CLASS_IS_A_REAL_PITA


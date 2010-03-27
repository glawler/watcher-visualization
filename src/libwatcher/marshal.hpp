/**@file
 * c++ wrapper around idsCommunications/marshal.h to provide some safety.
 */

#include "idsCommunications/marshal.h"

namespace Marshal {
    class Input {
        private:
            std::istream& is;
        public:
            Input(std::istream& is_) : is(is_) {};

            Input& operator>>(short& v);
            Input& operator>>(unsigned short& v);
            Input& operator>>(int& v);
            Input& operator>>(unsigned int& v);
            Input& operator>>(long& v);
            Input& operator>>(unsigned long& v);
            Input& operator>>(long long& v);
            Input& operator>>(unsigned long long& v);
            Input& operator>>(std::string& v);
            Input& operator>>(double& v);

            //read a collection
            template <typename OutputIterator> Input& getCollection (OutputIterator it);
    };

    Input& Input::operator>>(short& v)
    {
        char buf[sizeof(int16_t)];
        char *p = buf;
        is.read(buf, sizeof(int16_t));
        int16_t val;
        UNMARSHALSHORT(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(unsigned short& v)
    {
        char buf[sizeof(uint16_t)];
        char *p = buf;
        is.read(buf, sizeof(uint16_t));
        int16_t val;
        UNMARSHALSHORT(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(int& v)
    {
        char buf[sizeof(int32_t)];
        char *p = buf;
        is.read(buf, sizeof(int32_t));
        int32_t val;
        UNMARSHALLONG(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(unsigned int& v)
    {
        char buf[sizeof(uint32_t)];
        char *p = buf;
        is.read(buf, sizeof(uint32_t));
        uint32_t val;
        UNMARSHALLONG(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(long& v)
    {
        char buf[sizeof(uint32_t)];
        char *p = buf;
        is.read(buf, sizeof(uint32_t));
        int32_t val;
        UNMARSHALLONG(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(unsigned long& v)
    {
        char buf[sizeof(uint32_t)];
        char *p = buf;
        is.read(buf, sizeof(uint32_t));
        uint32_t val;
        UNMARSHALLONG(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(long long& v)
    {
        char buf[sizeof(uint64_t)];
        char *p = buf;
        is.read(buf, sizeof(uint64_t));
        int32_t val;
        UNMARSHALLONG(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(unsigned long long& v)
    {
        char buf[sizeof(uint64_t)];
        char *p = buf;
        is.read(buf, sizeof(uint64_t));
        uint32_t val;
        UNMARSHALLONG(p, val);
        v = val;
        return *this;
    }

    Input& Input::operator>>(std::string& s)
    {
        size_t n;
        char buf[256];
        char *p = buf;

        is.read(buf, 1);
        n = buf[0];
        is.read(buf, n);

        std::string ns(buf, n);
        swap(s, ns);

        return *this;
    }

    Input& Input::operator>>(double& v)
    {
        return *this;
    }

    template <typename OutputIterator> Input& Input::getCollection (OutputIterator it)
    {
        size_t n;
        char buf;

        // get number of items in collection
        is.read(&buf, 1);
        n = buf;

        for (size_t i = 0; i < n; ++i) {
            typename OutputIterator::container_type::value_type tmp;
            *this >> tmp;
            *it++ = tmp;
        }

        return *this;
    }

    class Output {
        private:
            std::ostream& os;
        public:
            Output(std::ostream& os_) : os(os_) {};
            Output& operator<<(short v);
            Output& operator<<(unsigned short v);
            Output& operator<<(int v);
            Output& operator<<(unsigned int v);
            Output& operator<<(long v);
            Output& operator<<(unsigned long v);
            Output& operator<< (long long v);
            Output& operator<< (unsigned long long v);
            Output& operator<< (const std::string&);
    };

    Output& Output::operator<< (short v)
    {
        char buf[sizeof(int16_t)];
        char *p = buf;
        MARSHALSHORT(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (unsigned short v)
    {
        char buf[sizeof(uint16_t)];
        char *p = buf;
        MARSHALSHORT(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (int v)
    {
        char buf[sizeof(int32_t)];
        char *p = buf;
        MARSHALLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (unsigned int v)
    {
        char buf[sizeof(uint32_t)];
        char *p = buf;
        MARSHALLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (long v)
    {
        char buf[sizeof(int32_t)];
        char *p = buf;
        MARSHALLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (unsigned long v)
    {
        char buf[sizeof(uint32_t)];
        char *p = buf;
        MARSHALLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (long long v)
    {
        char buf[sizeof(uint64_t)];
        char *p = buf;
        MARSHALLONGLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (unsigned long long v)
    {
        char buf[sizeof(uint64_t)];
        char *p = buf;
        MARSHALLONGLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator<< (const std::string& s)
    {
        os.put(s.length());
        os.write(s.c_str(), s.length());
        return *this;
    }
}

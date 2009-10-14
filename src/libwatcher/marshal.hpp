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

    class Output {
        private:
            std::ostream& os;
        public:
            Output(std::ostream& os_) : os(os_) {};
            Output& operator>>(short v);
            Output& operator>>(long v);
    };

    Output& Output::operator>> (short v)
    {
        char buf[sizeof(short)];
        char *p = buf;
        MARSHALSHORT(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }

    Output& Output::operator>> (long v)
    {
        char buf[sizeof(long)];
        char *p = buf;
        MARSHALLONG(p, v);
        os.write(buf, sizeof(buf));
        return *this;
    }
}

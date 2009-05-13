#ifndef util_assert_h
#define util_assert_h

namespace util {

    template <class Except> inline void Assert(bool expr)
    {
        if (not expr)
            throw Except();
    }

}

#endif /* util_assert_h */

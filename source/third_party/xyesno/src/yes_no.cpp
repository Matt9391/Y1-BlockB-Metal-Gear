#include "yes_no.h"

namespace utl
{
    yes_no::yes_no(eyes_no value)
        : m_value(value)
    {
    }

    yes_no::operator bool() const
    {
        return m_value == eyes_no::yes;
    }
}

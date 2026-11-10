#pragma once

namespace utl
{
    class yes_no
    {
    public:
        enum eyes_no
        {
            no = 0,
            yes = 1
        };

        yes_no(eyes_no value);

        operator bool() const;

    private:
        eyes_no m_value;
    };

    template <typename Tag>
    class yes_no_t : public yes_no
    {
    public:
        using yes_no::yes_no;

        yes_no_t(const bool& value)
            : yes_no(value ? eyes_no::yes : eyes_no::no)
        {
        }

        yes_no_t(bool&& value) = delete;
    };
}

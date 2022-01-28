#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <gtest/gtest.h>

class object
{
public:
    object()
            : tag(boost::uuids::random_generator()())
            , state(0)
    {}

    explicit object(int state)
            : tag(boost::uuids::random_generator()())
            , state(state)
    {}

    object(object const& rhs)
            : tag(rhs.tag)
            , state(rhs.state)
    {}

    bool operator==(object const& rhs) const {
        return tag == rhs.tag;
    }
    bool operator!=(object const& rhs) const {
        return !(operator==(rhs));
    }

    object& operator=(object const& rhs) {
        tag = rhs.tag;
        state = rhs.state;
        return *this;
    }

    int get_state() const { return state; }
    void set_state(int new_state) { state = new_state; }

private:
    boost::uuids::uuid tag;

    int state;
};

template <typename elem, typename traits>
std::basic_ostream<elem, traits>& operator<<(std::basic_ostream<elem, traits>& os, object const& o)
{
    os << o.get_state();
    return os;
}

TEST(BoostTest, UUID) {
    object o1(1);
    object o2 = o1;
    EXPECT_EQ(o1, o2);
}
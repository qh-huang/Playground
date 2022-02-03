#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <gtest/gtest.h>


TEST(BoostTest, UUID) {
    boost::uuids::uuid uuid_1 = boost::uuids::random_generator()();
    boost::uuids::uuid uuid_2 = boost::uuids::random_generator()();
    std::cout << "uuid_1: " << uuid_1 << std::endl;
    std::cout << "uuid_2: " << uuid_2 << std::endl;
    EXPECT_NE(uuid_1, uuid_2);
}
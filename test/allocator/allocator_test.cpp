#include <gtest/gtest.h>

#include "karma_test.h"
#include "maxmin_test.h"
#include "static_test.h"

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
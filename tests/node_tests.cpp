#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "../node/include/message.h"

using namespace std;

/**
 * Google Test Fixture class for set up environment and do post test operations
 * automatically
 */
class MyTestFixture : public ::testing::Test {
 public:
  void SetUp() override {
    cout << "\npre test operation executed.";
    // Setup code here
  }

  void TearDown() override {
    cout << "\npost test operation executed.\n";
    // Teardown code here
  }
};


TEST_F(MyTestFixture, TestCase1) {
  EXPECT_EQ(Message::buildFirstMessage(3, 5), "00322001234561111111111111111005");
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
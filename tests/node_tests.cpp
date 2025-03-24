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
    //cout << "\npre test operation executed.";
    // Setup code here
  }

  void TearDown() override {
    //cout << "\npost test operation executed.\n";
    // Teardown code here
  }
};


TEST_F(MyTestFixture, Test_Initial_Message) {
  EXPECT_EQ(Message::buildFirstMessage(3, 5), "00322001234561111111111111111005");
}

TEST_F(MyTestFixture, Test_Process_Message1) {
  EXPECT_EQ(Message::processMessage(3,"00522101234561111111111111111003",32), "00322201234561111111111111111005");
}
TEST_F(MyTestFixture, Test_Process_Message2) {
  EXPECT_EQ(Message::processMessage(3,"00322101234561111111111111111005",32), "");
}
TEST_F(MyTestFixture, Test_Process_Message3) {
  EXPECT_EQ(Message::processMessage(3,"0052210123456111111111111111003",31), "");
}

TEST_F(MyTestFixture, Test_Id_Message) {
  EXPECT_EQ(Message::buildIdMessage(3), "003");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
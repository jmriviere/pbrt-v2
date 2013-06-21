#include "host.h"
#include "gtest/gtest.h"

#define TEST_PATH "/home/poupine/Thesis/pbrt-v2/src/tests/kernels"

// The fixture for testing class Foo.
class HostTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  HostTest() {
    // You can do set-up work for each test here.
  }

  virtual ~HostTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case.

  Host* h = &Host::instance();

};

TEST_F(HostTest, IsSingleton) {

	ASSERT_NE(h, NULL);
	ASSERT_NE(*h, NULL);

}

TEST_F(HostTest, KernelBuildSuccessful) {

	ASSERT_NO_THROW(h->buildKernels(TEST_PATH));

}

TEST_F(HostTest, KernelTest) {
	int i = 0;

	cl::Event event;

	cl::Kernel k = h->retrieveKernel("test");
	cl::Buffer buf(*(h->_context), CL_MEM_WRITE_ONLY, sizeof(int), NULL, NULL);
	k.setArg(0, buf);
	h->_queue->enqueueTask(k, NULL, &event);

	event.wait();

	h->_queue->enqueueReadBuffer(buf, CL_TRUE, 0, sizeof(int), &i, NULL, NULL);

	ASSERT_EQ(150, i);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

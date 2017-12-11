
#include "gtest/gtest.h"

#include "enfield/Arch/Architectures.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

TEST(IBMQX2Tests, IntantiationTest) {
    InitializeAllArchitectures();
    ArchGraph::uRef graph = efd::CreateArchitecture("ibmqx2");

    ASSERT_FALSE(graph == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);

    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(3, 4));
    ASSERT_TRUE(graph->hasEdge(3, 2));
    ASSERT_TRUE(graph->hasEdge(4, 2));
    
    ASSERT_TRUE(graph->isReverseEdge(1, 0));
    ASSERT_TRUE(graph->isReverseEdge(2, 0));
    ASSERT_TRUE(graph->isReverseEdge(2, 1));
    ASSERT_TRUE(graph->isReverseEdge(4, 3));
    ASSERT_TRUE(graph->isReverseEdge(2, 3));
    ASSERT_TRUE(graph->isReverseEdge(2, 4));
}

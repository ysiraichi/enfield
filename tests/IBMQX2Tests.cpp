
#include "gtest/gtest.h"

#include "enfield/Arch/Architectures.h"
#include "enfield/Support/RTTI.h"

#include <string>

using namespace efd;

TEST(IBMQX2Tests, IntantiationTest) {
    std::unique_ptr<Graph> graph = ArchIBMQX2::Create();
    ASSERT_FALSE(graph == nullptr);

    ASSERT_EQ(graph->size(), 5);
    ASSERT_TRUE(instanceOf<ArchGraph>(graph.get()));

    ArchGraph* aGraph = dynCast<ArchGraph>(graph.get());
    ASSERT_FALSE(aGraph == nullptr);

    ASSERT_TRUE(aGraph->hasEdge(0, 1));
    ASSERT_TRUE(aGraph->hasEdge(0, 2));
    ASSERT_TRUE(aGraph->hasEdge(1, 2));
    ASSERT_TRUE(aGraph->hasEdge(3, 4));
    ASSERT_TRUE(aGraph->hasEdge(3, 2));
    ASSERT_TRUE(aGraph->hasEdge(4, 2));

    ASSERT_TRUE(aGraph->hasEdge(1, 0));
    ASSERT_TRUE(aGraph->hasEdge(2, 0));
    ASSERT_TRUE(aGraph->hasEdge(2, 1));
    ASSERT_TRUE(aGraph->hasEdge(4, 3));
    ASSERT_TRUE(aGraph->hasEdge(2, 3));
    ASSERT_TRUE(aGraph->hasEdge(2, 4));
    
    ASSERT_TRUE(aGraph->isReverseEdge(1, 0));
    ASSERT_TRUE(aGraph->isReverseEdge(2, 0));
    ASSERT_TRUE(aGraph->isReverseEdge(2, 1));
    ASSERT_TRUE(aGraph->isReverseEdge(4, 3));
    ASSERT_TRUE(aGraph->isReverseEdge(2, 3));
    ASSERT_TRUE(aGraph->isReverseEdge(2, 4));
}

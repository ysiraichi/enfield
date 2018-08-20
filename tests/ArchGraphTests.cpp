#include "gtest/gtest.h"

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/JsonParser.h"

#include <string>

using namespace efd;

TEST(ArchGraphTests, TreeCreationTest) {
    const std::string gStr =
"{\n\
    \"qubits\": 5,\n\
    \"registers\": [ {\"name\": \"q\", \"qubits\": 5} ],\n\
    \"adj\": [\n\
        [ {\"v\": \"q[1]\"}, {\"v\": \"q[2]\"} ],\n\
        [ {\"v\": \"q[3]\"}, {\"v\": \"q[4]\"} ],\n\
        [],\n\
        [],\n\
        []\n\
    ]\n\
}";

    auto graph = JsonParser<ArchGraph>::ParseString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));

    ASSERT_TRUE(!graph->hasEdge(1, 0));
    ASSERT_TRUE(!graph->hasEdge(2, 0));
    ASSERT_TRUE(!graph->hasEdge(3, 1));
    ASSERT_TRUE(!graph->hasEdge(4, 1));
}

TEST(ArchGraphTests, SomeReverseEdgesTest) {
    const std::string gStr =
"{\n\
    \"qubits\": 5,\n\
    \"registers\": [ {\"name\": \"q\", \"qubits\": 5} ],\n\
    \"adj\": [\n\
        [ {\"v\": \"q[1]\"}, {\"v\": \"q[2]\"} ],\n\
        [ {\"v\": \"q[0]\"}, {\"v\": \"q[2]\"},\n\
          {\"v\": \"q[3]\"}, {\"v\": \"q[4]\"} ],\n\
        [],\n\
        [],\n\
        [ {\"v\": \"q[1]\"} ]\n\
    ]\n\
}";

    auto graph = JsonParser<ArchGraph>::ParseString(gStr);
    ASSERT_FALSE(graph.get() == nullptr);

    ASSERT_EQ(graph->size(), (uint32_t) 5);
    ASSERT_TRUE(graph->hasEdge(0, 1));
    ASSERT_TRUE(graph->hasEdge(1, 0));
    ASSERT_TRUE(graph->hasEdge(0, 2));
    ASSERT_TRUE(graph->hasEdge(1, 2));
    ASSERT_TRUE(graph->hasEdge(1, 3));
    ASSERT_TRUE(graph->hasEdge(1, 4));
    ASSERT_TRUE(graph->hasEdge(4, 1));

    ASSERT_TRUE(!graph->hasEdge(2, 0));
    ASSERT_TRUE(!graph->hasEdge(2, 1));
    ASSERT_TRUE(!graph->hasEdge(3, 1));

    ASSERT_FALSE(!graph->hasEdge(1, 0));
    ASSERT_FALSE(!graph->hasEdge(4, 1));
}

TEST(ArchGraphTests, MultipleRegisterArchitecture) {
    {
        const std::string gStr =
"{\n\
    \"qubits\": 5,\n\
    \"registers\": [\n\
        {\"name\": \"r\", \"qubits\": 1},\n\
        {\"name\": \"q\", \"qubits\": 4}\n\
    ],\n\
    \"adj\": [\n\
        [ {\"v\": \"q[0]\"}, {\"v\": \"q[1]\"} ],\n\
        [ {\"v\": \"q[2]\"}, {\"v\": \"q[3]\"} ],\n\
        [],\n\
        [],\n\
        []\n\
    ]\n\
}";

        auto graph = JsonParser<ArchGraph>::ParseString(gStr);
        ASSERT_FALSE(graph.get() == nullptr);

        ASSERT_EQ(graph->size(), (uint32_t) 5);
        ASSERT_TRUE(graph->hasEdge(0, 1));
        ASSERT_TRUE(graph->hasEdge(0, 2));
        ASSERT_TRUE(graph->hasEdge(1, 3));
        ASSERT_TRUE(graph->hasEdge(1, 4));

        ASSERT_TRUE(!graph->hasEdge(1, 0));
        ASSERT_TRUE(!graph->hasEdge(2, 0));
        ASSERT_TRUE(!graph->hasEdge(3, 1));
        ASSERT_TRUE(!graph->hasEdge(4, 1));
    }
    {
        const std::string gStr =
"{\n\
    \"qubits\": 5,\n\
    \"registers\": [\n\
        {\"name\": \"q\", \"qubits\": 2},\n\
        {\"name\": \"r\", \"qubits\": 3}\n\
    ],\n\
    \"adj\": [\n\
        [ {\"v\": \"q[1]\"}, {\"v\": \"r[0]\"} ],\n\
        [ {\"v\": \"q[0]\"}, {\"v\": \"r[0]\"},\n\
          {\"v\": \"r[1]\"}, {\"v\": \"r[2]\"} ],\n\
        [],\n\
        [],\n\
        [ {\"v\": \"q[1]\"} ]\n\
    ]\n\
}";

        auto graph = JsonParser<ArchGraph>::ParseString(gStr);
        ASSERT_FALSE(graph.get() == nullptr);

        ASSERT_EQ(graph->size(), (uint32_t) 5);
        ASSERT_TRUE(graph->hasEdge(0, 1));
        ASSERT_TRUE(graph->hasEdge(1, 0));
        ASSERT_TRUE(graph->hasEdge(0, 2));
        ASSERT_TRUE(graph->hasEdge(1, 2));
        ASSERT_TRUE(graph->hasEdge(1, 3));
        ASSERT_TRUE(graph->hasEdge(1, 4));
        ASSERT_TRUE(graph->hasEdge(4, 1));

        ASSERT_TRUE(!graph->hasEdge(2, 0));
        ASSERT_TRUE(!graph->hasEdge(2, 1));
        ASSERT_TRUE(!graph->hasEdge(3, 1));

        ASSERT_FALSE(!graph->hasEdge(1, 0));
        ASSERT_FALSE(!graph->hasEdge(4, 1));
    }
}

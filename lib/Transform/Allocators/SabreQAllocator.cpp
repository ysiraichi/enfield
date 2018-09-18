using namespace efd;

SabreQAllocator::SabreQAllocator(ArchGraph::sRef ag)
    : QbitAllocator(ag) {}

Mapping SabreQAllocator::allocateWithInitialMapping(const Mapping& initialMapping,
                                                    QModule::Ref qmod,
                                                    bool issueInstructions) {
    auto mapping = initialMapping;
    auto stmtNumber = qmod->getNumberOfStmts();

    auto depBuilder = PassCache::Get<DependencyBuilderWrapperPass>(qmod)->getData();
    auto cGraph = PassCache::Get<CircuitGraphBuilderPass>(qmod)->getData();

    auto it = cGraph.build_iterator();
    auto xbitNumber = cGraph.size();

    std::map<Node::Ref, uint32_t> reached;
    std::set<Node::Ref> pastLookAhead;

    std::vector<Node::uRef> newStatements;

    for (uint32_t i = 0; i < xbitNumber; ++i) {
        it.next(i);
        ++reached[it.get(i)];
    }

    while (true) {
        bool changed;

        do {
            changed = false;

            for (uint32_t i = 0; i < xbitNumber; ++i) {
                if (it[i]->isGateNode() && it[i]->numberOfXbits() == reached[it.get(i)]) {
                    switch (it.get(i)->getKind()) {
                        case Node::Kind::K_QOP_U:
                        case Node::Kind::K_QOP_CX:
                        case Node::Kind::K_QOP_GEN:
                            if (it[i]->numberOfXbits() > 1) {
                                auto deps = depBuilder.getDeps(it.get(i));

                                if (deps.size() != 1) {
                                    ERR << "Unable to handle more than ONE dependency "
                                        << "in: `" << it.get(i)->toString(falsel) << "` "
                                        << std::endl;
                                    EFD_ABORT();
                                }

                                auto dep = deps[0];
                                uint32_t u = mapping[dep.mFrom], v = mapping[dep.mTo];

                                if (!mArchGraph.hasEdge(u, v) &&
                                    !mArchGraph.hasEdge(v, u)) {
                                    break;
                                }
                            }

                        case Node::Kind::K_QOP_BARRIER:
                        case Node::Kind::K_QOP_MEASURE:
                            changed = true;
                            it.next(i);
                            ++reached[it.get(i)];
                            break;

                        default:
                            break;
                    }
                }
            }
        } while (changed);

        std::set<std::pair<Node::Ref, Dep>> currentLayer;
        std::set<std::pair<Node::Ref, Dep>> nextLayer;

        uint32_t offset = std::numeric_limits<uint32_t>::max();

        for (uint32_t i = 0; i < xbitNumber; ++i) {
            if (it[i]->isGateNode() && it[i]->numberOfXbits() == reached[it.get(i)]) {
                auto node = it.get(i);
                auto dep = depBuilder.getDeps(node)[0];
                pastLookAhead.insert(node);
                currentLayer.insert(std::make_pair(node, dep));
                offset = std::min(offset, std::distance(qmod->stmt_begin(),
                                                        qmod->findStatement(it.get(i))));
            }
        }

        for (uint32_t i = offset, maxI = offset + mMaxLookAhead; i < stmtNumber && i < maxI; ++i) {
            auto node = qmod->getStatement(i);
            if (pastLookAhead.find(node) == pastLookAhead.end()) {
                nextLayer.insert(std::make_pair(node, depBuilder.getDeps(node)[0]));
            }
        }

        std::set<uint32_t> usedQubits;
        std::set<std::pair<double, Swap>> swapCandidates;

        for (auto node : currentLayer) {
            usedQubits.insert(mapping[dep.mFrom]);
            usedQubits.insert(mapping[dep.mTo]);
        }

        auto invM = InvertMapping(mapping);

        for (auto u : usedQubits) {
            for (auto v : mArchGraph->adj(u)) {
                auto cpy = mapping;
                std::swap(cpy[inv[u]], cpy[inv[v]]);

                double currentLCost = 0;
                double nextLCost = 0;

                for (auto pair : currentLayer) {
                    currentLCost += mDistance[cpy[pair.first.mFrom]][cpy[pair.first.mTo]];
                }

                for (auto pair : nextLayer) {
                    nextLCost += mDistance[cpy[pair.first.mFrom]][cpy[pair.first.mTo]];
                }

                double cost = currentLCost + 0.5 * nextLCost;
                swapCandidates.insert(std::make_pair(cost, Swap { u, v }));
            }
        }

        auto swap = swapCandidates.begin()->second;
        std::swap(mapping[inv[swap.u]], mapping[inv[swap.v]]);
    }
}

Mapping SabreQAllocator::allocate(QModule::Ref qmod) {
}

SabreQAllocator::uRef SabreQAllocator::Create(ArchGraph::sRef ag) {
    return uRef(new SabreQAllocator(ag));
}

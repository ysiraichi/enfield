#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/RTTI.h"

using namespace efd;

uint8_t CircuitGraphBuilderPass::ID = 0;

bool CircuitGraphBuilderPass::run(QModule* qmod) {
    auto& graph = mData;

    auto xtonpass = PassCache::Get<XbitToNumberWrapperPass>(qmod);
    auto xton = xtonpass->getData();

    auto qubits = xton.getQSize();
    auto cbits = xton.getCSize();

    graph.init(qubits, cbits);

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        auto node = it->get();
        auto qop = dynCast<NDQOp>(node);

        std::vector<Xbit> xbits;

        if (auto ifstmt = dynCast<NDIfStmt>(node)) {
            qop = ifstmt->getQOp();
            auto cbitstr = ifstmt->getCondId()->getVal();

            for (auto cbit : xton.getRegUIds(cbitstr)) {
                xbits.push_back(Xbit::C(cbit));
            }

        } else if (auto measure = dynCast<NDQOpMeasure>(node)) {
            auto cbitstr = measure->getCBit()->toString();
            xbits.push_back(Xbit::C(xton.getCUId(cbitstr)));
        }

        auto qargs = qop->getQArgs();

        for (uint32_t i = 0, e = qargs->getChildNumber(); i < e; ++i) {
            auto qarg = qargs->getChild(i);
            xbits.push_back(Xbit::Q(xton.getQUId(qarg->toString())));
        }

        graph.append(xbits, node);
    }

    return false;
}

CircuitGraphBuilderPass::uRef CircuitGraphBuilderPass::Create() {
    return uRef(new CircuitGraphBuilderPass());
}

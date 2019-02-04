#include "enfield/Transform/Driver.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/CNOTLBOWrapperPass.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Allocators/Allocators.h"
#include "enfield/Transform/DependencyGraphBuilderPass.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/Stats.h"
#include "enfield/Support/Defs.h"

using namespace efd;

static Stat<uint32_t> StatNofEdges
("DGEdges", "Number of edges in the dependency graph.");
static Stat<uint32_t> StatNofVertices
("DGVertices", "Number of vertices in the dependency graph.");
static Stat<double> StatDepGraphDensity
("DGDensity", "Density of the dependency graph.");

QModule::uRef efd::Compile(QModule::uRef qmod, CompilationSettings settings) {
    bool success = true;
    QModule::uRef qmodCopy;

    if (settings.verify) {
        qmodCopy = qmod->clone();
    }

    PassCache::Run<FlattenPass>(qmod.get());

    if (settings.reorder) {
        PassCache::Run<CNOTLBOWrapperPass>(qmod.get());
    }

    auto xbitPass = PassCache::Get<XbitToNumberWrapperPass>(qmod.get());
    auto xbitToNumber = xbitPass->getData(); 

    EfdAbortIf(xbitToNumber.getQSize() > settings.archGraph->size(),
               "Using more qbits than the maximum permitted by the architecture (max `"
               << settings.archGraph->size() << "`): `"
               << xbitToNumber.getQSize() << "`.");

    auto allocPass = CreateQbitAllocator(settings.allocator, settings.archGraph);
    allocPass->setGateWeightMap(settings.gWeightMap);
    PassCache::Run(qmod.get(), allocPass.get());

    auto revPass = ReverseEdgesPass::Create(settings.archGraph);
    PassCache::Run(qmod.get(), revPass.get());

    if (settings.verify) {
        auto mapping = allocPass->getData();

        auto aVerifierPass = ArchVerifierPass::Create(settings.archGraph);
        auto sVerifierPass = SemanticVerifierPass::Create(std::move(qmodCopy), mapping);
        sVerifierPass->setInlineAll(ExtractGateNames(settings.gWeightMap));

        PassCache::Run(qmod.get(), aVerifierPass.get());
        success = success && aVerifierPass->getData();

        PassCache::Run(qmod.get(), sVerifierPass.get());
        auto sVerifierData = sVerifierPass->getData();
        success = success && sVerifierData.isSuccess();

        if (!aVerifierPass->getData()) {
            ERR << "Architecture restrictions violated in compiled code." << std::endl;
        }

        if (sVerifierData.isError()) {
            ERR << "Compiled code is semantically different from source code." << std::endl;
            ERR << sVerifierData.getErrorMessage() << std::endl;
        }

        if (!success) ERR << "Compilation failed." << std::endl;
    }

    if (!success && !settings.force) qmod.reset(nullptr);
    else if (!success && settings.force) WAR << "Printing incorrect QModule." << std::endl;
    return qmod;
}

QModule::uRef efd::ParseFile(std::string filepath) {
    QModule::uRef qmod(nullptr);
    std::string path;
    std::string filename;

    if (filepath != "") {
        // Spliting filepath into 'filename' and 'path'.
        auto lastslash = filepath.find_last_of('/');

        if (lastslash != std::string::npos) {
            path = filepath.substr(0, lastslash + 1);
            filename = filepath.substr(lastslash + 1, std::string::npos);
        } else {
            path = "./";
            filename = filepath;
        }

        qmod.reset(QModule::Parse(filename, path).release());
    }

    return qmod;
}

void efd::PrintToStream(QModule::Ref qmod, std::ostream& o, bool pretty) {
    qmod->print(o, pretty);
}

void efd::PrintDependencyGraph(QModule::Ref qmod, std::ostream& o) {
    auto depgraphPass = PassCache::Get<DependencyGraphBuilderPass>(qmod);
    auto graph = depgraphPass->getData();
    o << graph->dotify() << std::endl;

    StatNofVertices = graph->size();
    StatNofEdges = 0;

    for (uint32_t i = 0, e = graph->size(); i < e; ++i)
        StatNofEdges += graph->succ(i).size();

    StatDepGraphDensity = ((double) StatNofEdges.getVal()) /
        ((double) StatNofVertices.getVal() * StatNofVertices.getVal());
}

void efd::Init(int argc, char** argv) {
    InitializeAllQbitAllocators();
    InitializeAllArchitectures();
    ParseArguments(argc, argv);
    InitializeLogs();
}

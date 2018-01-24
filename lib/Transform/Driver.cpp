#include "enfield/Transform/Driver.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/CNOTLBOWrapperPass.h"
#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Allocators/Allocators.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Support/Defs.h"

using namespace efd;

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

    std::shared_ptr<ArchGraph> archGraph;
    if (HasArchitecture(settings.architecture)) {
        archGraph = CreateArchitecture(settings.architecture);
    } else {
        archGraph = ArchGraph::Read(settings.architecture);
    }

    assert(xbitToNumber.getQSize() <= archGraph->size() &&
            "Using more qbits than the maximum permitted by the architecture.");

    auto allocPass = CreateQbitAllocator(settings.allocator, archGraph);
    allocPass->setInlineAll(settings.basis);
    PassCache::Run(qmod.get(), allocPass.get());

    auto revPass = ReverseEdgesPass::Create(archGraph);
    PassCache::Run(qmod.get(), revPass.get());

    if (settings.verify) {
        auto mapping = allocPass->getData().mInitial;

        auto aVerifierPass = ArchVerifierPass::Create(archGraph);
        auto sVerifierPass = SemanticVerifierPass::Create(std::move(qmodCopy), mapping);
        sVerifierPass->setInlineAll(settings.basis);

        PassCache::Run(qmod.get(), aVerifierPass.get());
        success = success && aVerifierPass->getData();

        PassCache::Run(qmod.get(), sVerifierPass.get());
        success = success && sVerifierPass->getData();

        if (!aVerifierPass->getData()) {
            ERR << "Architecture restrictions violated in compiled code." << std::endl;
        }

        if (!sVerifierPass->getData()) {
            ERR << "Compiled code is semantically different from source code." << std::endl;
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

void efd::PrintToStream(QModule::Ref qmod, std::ostream& o) {
    qmod->print(o, true);
}

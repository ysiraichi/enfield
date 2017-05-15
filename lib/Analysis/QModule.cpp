#include "enfield/Analysis/Driver.h"
#include "enfield/Analysis/QModule.h"
#include "enfield/Analysis/QModulefyPass.h"

efd::QModule::QModule() : mAST(nullptr), mVersion(nullptr) {
}

efd::QModule::Iterator efd::QModule::reg_begin() {
    return mRegDecls.begin();
}

efd::QModule::ConstIterator efd::QModule::reg_begin() const {
    return mRegDecls.begin();
}

efd::QModule::Iterator efd::QModule::reg_end() {
    return mRegDecls.end();
}

efd::QModule::ConstIterator efd::QModule::reg_end() const {
    return mRegDecls.end();
}

efd::QModule::Iterator efd::QModule::gates_begin() {
    return mGates.begin();
}

efd::QModule::ConstIterator efd::QModule::gates_begin() const {
    return mGates.begin();
}

efd::QModule::Iterator efd::QModule::gates_end() {
    return mGates.end();
}

efd::QModule::ConstIterator efd::QModule::gates_end() const {
    return mGates.end();
}

efd::QModule::Iterator efd::QModule::stmt_begin() {
    return mStatements.begin();
}

efd::QModule::ConstIterator efd::QModule::stmt_begin() const {
    return mStatements.begin();
}

efd::QModule::Iterator efd::QModule::stmt_end() {
    return mStatements.end();
}

efd::QModule::ConstIterator efd::QModule::stmt_end() const {
    return mStatements.end();
}

void efd::QModule::print(std::ostream& O, bool pretty) const {
    O << toString(pretty);
}

std::string efd::QModule::toString(bool pretty) const {
    return mAST->toString(pretty);
}

std::unique_ptr<efd::QModule> efd::QModule::GetFromAST(NodeRef ref) {
    QModulefyPass* pass = QModulefyPass::Create();
    ref->apply(pass);
    std::unique_ptr<QModule> qmod(pass->mMod);
    qmod->mAST = ref;
    return qmod;
}

std::unique_ptr<efd::QModule> efd::QModule::Parse(std::string filename, std::string path) {
    NodeRef ast = efd::ParseFile(filename, path);

    if (ast != nullptr)
        return GetFromAST(ast);

    return std::unique_ptr<QModule>(nullptr);
}

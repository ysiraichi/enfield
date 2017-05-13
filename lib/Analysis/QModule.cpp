#include "enfield/Analysis/Driver.h"
#include "enfield/Analysis/QModule.h"

efd::QModule::QModule(NodeRef ref) : mAST(ref) {
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

std::unique_ptr<efd::QModule> efd::QModule::GetFromAST(NodeRef ref) {
}

std::unique_ptr<efd::QModule> efd::QModule::Parse(std::string filename, std::string path) {
    NodeRef ast = efd::ParseFile(filename, path);

    if (ast != nullptr)
        return GetFromAST(ast);

    return std::unique_ptr<QModule>(nullptr);
}

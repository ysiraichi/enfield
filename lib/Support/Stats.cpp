#include "enfield/Support/Stats.h"
#include "enfield/Support/Defs.h"

#include <memory>
#include <map>

namespace efd {
    class StatsPool {
        private:
            typedef std::map<std::string, StatBase*> StatMap;

            StatMap mMap;

        public:
            void addStat(StatBase* stat);
            bool hasStat(std::string name);

            void print(std::ostream& out);
    };
}

void efd::StatsPool::addStat(StatBase* stat) {
    EfdAbortIf(hasStat(stat->getName()),
               "Stat with the same name already defined: `" << stat->getName() << "`.");

    mMap[stat->getName()] = stat;
}

bool efd::StatsPool::hasStat(std::string name) {
    return mMap.find(name) != mMap.end();
}

void efd::StatsPool::print(std::ostream& out) {
    for (auto pair : mMap) {
        if (!pair.second->isZero())
            pair.second->print(out);
    }
}

static std::shared_ptr<efd::StatsPool> getPool() {
    static std::shared_ptr<efd::StatsPool> Pool(new efd::StatsPool());
    return Pool;
}

efd::StatBase::StatBase(std::string name, std::string description) :
    mName(name), mDescription(description) {
    mPool = getPool();
    mPool->addStat(this);
}

void efd::StatBase::print(std::ostream& out) {
    out << toString() << std::endl;
}

std::string efd::StatBase::getName() const {
    return mName;
}

std::string efd::StatBase::getDescription() const {
    return mDescription;
}

void efd::PrintStats(std::ostream& out) {
    auto Pool = getPool();

    out << std::endl;
    out << " ==-------------- Stats --------------==" << std::endl;
    Pool->print(out);
    out << " ==-----------------------------------==" << std::endl;
}

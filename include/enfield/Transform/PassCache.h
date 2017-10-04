#ifndef __EFD_PASS_CACHE_H__
#define __EFD_PASS_CACHE_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include <set>

namespace efd {
    /// \brief Static class that caches passes that were run by this compiler.
    class PassCache {
        public:
            typedef std::unordered_map<unsigned, Pass::sRef> PassMap;
            typedef std::unordered_map<QModule::Ref, PassMap> QModPassesMap;

        private:
            static QModPassesMap mPasses;

        public:
            PassCache() = delete;

            /// \brief Runs the pass \p T in \p qmod.
            ///
            /// This will cache the pass run and return it if called without  any
            /// modifications to the module.
            template <typename T>
            static void Run(QModule::Ref qmod) {
                if (mPasses.find(qmod) != mPasses.end()) {
                    auto& passmap = mPasses[qmod];
                    if (passmap.find(T::ID) != passmap.end())
                        return;
                } else {
                    mPasses[qmod] = PassMap();
                }

                Pass::sRef pass = T::Create();

                // qmod was modified, so we reset all passes already computed
                // in this cache.
                if (pass->run(qmod)) mPasses[qmod].clear();
                else mPasses[qmod][T::ID] = pass;
            }

            /// \brief Wrapper that runs a created pass.
            ///
            /// Should be used in order to maintain consistent the data in the
            /// cache.
            template <typename T>
            static void Run(QModule::Ref qmod, std::shared_ptr<T> pass) {
                auto ans = pass->run(qmod);
                if (ans) mPasses[qmod].clear();
            }

            /// \brief Returns true if this pass was already run for this module.
            template <typename T>
            static bool Has(QModule::Ref qmod) {
                if (mPasses.find(qmod) == mPasses.end()) return false;

                auto& passmap = mPasses[qmod];
                if (passmap.find(T::ID) == passmap.end())
                    return false;

                return true;
            }

            /// \brief Gets a shared pointer to the pass \p T run in \p qmod.
            template <typename T>
            static Pass::sRef Get(QModule::Ref qmod) {
                assert(Has<T>(qmod) && "Trying to get pass that was not cached.");
                return mPasses[qmod][T::ID];
            }
    };
}

#endif

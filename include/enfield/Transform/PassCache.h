#ifndef __EFD_PASS_CACHE_H__
#define __EFD_PASS_CACHE_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include <set>

namespace efd {
    /// \brief Static class that caches passes that were run by this compiler.
    class PassCache {
        public:
            typedef std::unordered_map<uint8_t*, Pass::sRef> PassMap;
            typedef std::unordered_map<QModule::Ref, PassMap> QModPassesMap;

        private:
            static QModPassesMap mPasses;

        public:
            PassCache() = delete;

            /// \brief Clears the cache for a certain \p qmod, or simply clears all cache.
            static void Clear(QModule::Ref qmod = nullptr) {
                if (qmod != nullptr) {
                    mPasses.erase(qmod);
                } else {
                    mPasses.clear();
                }
            }

            /// \brief Returns true if this pass was already run for this module.
            template <typename T>
            static bool Has(QModule::Ref qmod) {
                if (mPasses.find(qmod) == mPasses.end()) return false;

                auto& passmap = mPasses[qmod];
                if (passmap.find(&T::ID) == passmap.end())
                    return false;

                return true;
            }

            /// \brief Runs the pass \p T in \p qmod.
            ///
            /// This will cache the pass run and return it if called without  any
            /// modifications to the module.
            template <typename T>
            static void Run(QModule::Ref qmod) {
                if (Has<T>(qmod)) return;
                if (mPasses.find(qmod) == mPasses.end()) mPasses[qmod] = PassMap();
                Pass::sRef pass = T::Create();

                // qmod was modified, so we reset all passes already computed
                // in this cache.
                if (pass->run(qmod)) mPasses[qmod].clear();
                else mPasses[qmod][&T::ID] = pass;
            }

            /// \brief Wrapper that runs a created pass.
            ///
            /// Should be used in order to maintain consistent the data in the
            /// cache.
            template <typename T>
            static void Run(QModule::Ref qmod, T* pass) {
                auto ans = pass->run(qmod);
                if (ans) mPasses[qmod].clear();
            }

            /// \brief Gets a shared pointer to the pass \p T run in \p qmod. If it
            /// does not exist, it tries to run.
            template <typename T>
            static T* Get(QModule::Ref qmod) {
                if (!Has<T>(qmod)) Run<T>(qmod);
                return (T*) mPasses[qmod][&T::ID].get();
            }
    };
}

#endif

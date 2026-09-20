#include "editor/cancel/InteractionCancellationRegistry.h"
#include <vector>

namespace catchim::editor {

InteractionCancellationRegistry& InteractionCancellationRegistry::instance() {
    static InteractionCancellationRegistry s_instance;
    return s_instance;
}

CancelToken InteractionCancellationRegistry::registerCanceller(CancelFn fn) {
    if (!fn) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    CancelToken token = nextToken_++;
    cancellers_[token] = std::move(fn);
    return token;
}

bool InteractionCancellationRegistry::unregisterCanceller(CancelToken token) {
    if (token == 0) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return cancellers_.erase(token) > 0;
}

bool InteractionCancellationRegistry::cancelInteraction() {
    std::vector<CancelFn> active;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cancellers_.empty()) {
            return false;
        }
        active.reserve(cancellers_.size());
        for (auto& [token, fn] : cancellers_) {
            if (fn) {
                active.push_back(std::move(fn));
            }
        }
        cancellers_.clear();
    }

    for (const auto& cancel : active) {
        try {
            cancel();
        } catch (...) {
            // Suppress exceptions from user cancel handlers to guarantee full sweep
        }
    }

    return true;
}

size_t InteractionCancellationRegistry::activeCancellerCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cancellers_.size();
}

void InteractionCancellationRegistry::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cancellers_.clear();
}

} // namespace catchim::editor

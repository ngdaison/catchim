#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <cstdint>

namespace catchim::editor {

using CancelFn = std::function<void()>;
using CancelToken = uint64_t;

/**
 * @brief Manages cancellation callbacks for active interactions (drag, selection, modal, etc.)
 * Corresponds to web/src/editor/cancel-interaction.ts.
 */
class InteractionCancellationRegistry {
public:
    InteractionCancellationRegistry() = default;
    ~InteractionCancellationRegistry() = default;

    InteractionCancellationRegistry(const InteractionCancellationRegistry&) = delete;
    InteractionCancellationRegistry& operator=(const InteractionCancellationRegistry&) = delete;
    InteractionCancellationRegistry(InteractionCancellationRegistry&&) noexcept = default;
    InteractionCancellationRegistry& operator=(InteractionCancellationRegistry&&) noexcept = default;

    static InteractionCancellationRegistry& instance();

    CancelToken registerCanceller(CancelFn fn);
    bool unregisterCanceller(CancelToken token);

    bool cancelInteraction();
    bool cancelAll() { return cancelInteraction(); }

    size_t activeCancellerCount() const;
    void clear();

    class ScopedRegistration {
    public:
        ScopedRegistration(InteractionCancellationRegistry& registry, CancelFn fn)
            : registry_(&registry), token_(registry.registerCanceller(std::move(fn))) {}

        ~ScopedRegistration() {
            if (registry_ && token_ != 0) {
                registry_->unregisterCanceller(token_);
            }
        }

        ScopedRegistration(const ScopedRegistration&) = delete;
        ScopedRegistration& operator=(const ScopedRegistration&) = delete;
        ScopedRegistration(ScopedRegistration&& other) noexcept
            : registry_(other.registry_), token_(other.token_) {
            other.registry_ = nullptr;
            other.token_ = 0;
        }
        ScopedRegistration& operator=(ScopedRegistration&& other) noexcept {
            if (this != &other) {
                if (registry_ && token_ != 0) {
                    registry_->unregisterCanceller(token_);
                }
                registry_ = other.registry_;
                token_ = other.token_;
                other.registry_ = nullptr;
                other.token_ = 0;
            }
            return *this;
        }

        void dismiss() {
            if (registry_ && token_ != 0) {
                registry_->unregisterCanceller(token_);
                token_ = 0;
            }
        }

        CancelToken token() const noexcept { return token_; }

    private:
        InteractionCancellationRegistry* registry_{nullptr};
        CancelToken token_{0};
    };

    ScopedRegistration createScopedRegistration(CancelFn fn) {
        return ScopedRegistration(*this, std::move(fn));
    }

private:
    mutable std::mutex mutex_;
    CancelToken nextToken_{1};
    std::unordered_map<CancelToken, CancelFn> cancellers_;
};

} // namespace catchim::editor

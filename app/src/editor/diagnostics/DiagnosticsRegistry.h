#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

namespace catchim::editor {

enum class DiagnosticSeverity {
    Caution,
    Error
};

struct DiagnosticDefinition {
    std::string id;
    std::string scope; // e.g. "timeline", "media", "audio", "export", "general"
    DiagnosticSeverity severity{DiagnosticSeverity::Caution};
    std::string message;
};

struct DiagnosticRule {
    DiagnosticDefinition definition;
    std::function<bool()> check; // returns true if diagnostic issue is active
};

class DiagnosticsRegistry {
public:
    static DiagnosticsRegistry& instance();

    void registerRule(DiagnosticRule rule);
    void registerRule(
        std::string id,
        std::string scope,
        DiagnosticSeverity severity,
        std::string message,
        std::function<bool()> check
    );

    std::vector<DiagnosticDefinition> getActiveDiagnostics(
        std::optional<std::string> scope = std::nullopt
    ) const;

    size_t count() const noexcept { return rules_.size(); }
    void clear() noexcept { rules_.clear(); }

    using ListenerId = uint64_t;
    ListenerId subscribe(std::function<void()> listener);
    void unsubscribe(ListenerId id);
    void notify();

private:
    DiagnosticsRegistry() = default;

    std::vector<DiagnosticRule> rules_;
    std::vector<std::pair<ListenerId, std::function<void()>>> listeners_;
    ListenerId nextListenerId_{1};
};

} // namespace catchim::editor

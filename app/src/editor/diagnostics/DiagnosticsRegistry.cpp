#include "editor/diagnostics/DiagnosticsRegistry.h"
#include <algorithm>

namespace catchim::editor {

DiagnosticsRegistry& DiagnosticsRegistry::instance() {
    static DiagnosticsRegistry registry;
    return registry;
}

void DiagnosticsRegistry::registerRule(DiagnosticRule rule) {
    rules_.push_back(std::move(rule));
    notify();
}

void DiagnosticsRegistry::registerRule(
    std::string id,
    std::string scope,
    DiagnosticSeverity severity,
    std::string message,
    std::function<bool()> check
) {
    DiagnosticRule rule;
    rule.definition = {std::move(id), std::move(scope), severity, std::move(message)};
    rule.check = std::move(check);
    registerRule(std::move(rule));
}

std::vector<DiagnosticDefinition> DiagnosticsRegistry::getActiveDiagnostics(
    std::optional<std::string> scope
) const {
    std::vector<DiagnosticDefinition> active;

    for (const auto& rule : rules_) {
        if (scope.has_value() && rule.definition.scope != *scope) {
            continue;
        }

        if (rule.check && rule.check()) {
            active.push_back(rule.definition);
        }
    }

    return active;
}

DiagnosticsRegistry::ListenerId DiagnosticsRegistry::subscribe(std::function<void()> listener) {
    ListenerId id = nextListenerId_++;
    listeners_.emplace_back(id, std::move(listener));
    return id;
}

void DiagnosticsRegistry::unsubscribe(ListenerId id) {
    listeners_.erase(
        std::remove_if(listeners_.begin(), listeners_.end(), [id](const auto& pair) {
            return pair.first == id;
        }),
        listeners_.end()
    );
}

void DiagnosticsRegistry::notify() {
    for (const auto& [id, listener] : listeners_) {
        if (listener) {
            listener();
        }
    }
}

} // namespace catchim::editor

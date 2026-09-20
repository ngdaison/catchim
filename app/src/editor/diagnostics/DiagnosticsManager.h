#pragma once

#include "DiagnosticsRegistry.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace catchim::editor {

class EditorCore;

struct DiagnosticRegistration : public DiagnosticDefinition {
    std::function<bool(EditorCore&)> check;
};

class DiagnosticsManager {
public:
    explicit DiagnosticsManager(EditorCore& editor);
    ~DiagnosticsManager() = default;

    DiagnosticsManager(const DiagnosticsManager&) = delete;
    DiagnosticsManager& operator=(const DiagnosticsManager&) = delete;
    DiagnosticsManager(DiagnosticsManager&&) = default;
    DiagnosticsManager& operator=(DiagnosticsManager&&) = default;

    void registerDiagnostic(DiagnosticRegistration registration);
    std::vector<DiagnosticDefinition> getActive(const std::optional<std::string>& scope = std::nullopt) const;

    size_t subscribe(std::function<void()> listener);
    void unsubscribe(size_t listenerId);
    void notify();

    size_t registrationCount() const noexcept { return registrations_.size(); }
    void clear() noexcept;

private:
    EditorCore& editor_;
    std::vector<DiagnosticRegistration> registrations_;
    std::unordered_map<size_t, std::function<void()>> listeners_;
    size_t nextListenerId_{1};
};

inline constexpr const char* TRANSCRIPTION_DIAGNOSTICS_SCOPE = "transcription";

void registerTranscriptionDiagnostics(DiagnosticsManager& diagnostics);

} // namespace catchim::editor

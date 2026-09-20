#include "DiagnosticsManager.h"
#include "editor/core/EditorCore.h"

namespace catchim::editor {

DiagnosticsManager::DiagnosticsManager(EditorCore& editor)
    : editor_(editor) {
}

void DiagnosticsManager::registerDiagnostic(DiagnosticRegistration registration) {
    registrations_.push_back(std::move(registration));
    notify();
}

std::vector<DiagnosticDefinition> DiagnosticsManager::getActive(
    const std::optional<std::string>& scope) const {

    std::vector<DiagnosticDefinition> active;
    for (const auto& reg : registrations_) {
        if (scope.has_value() && reg.scope != *scope) {
            continue;
        }

        if (reg.check && reg.check(editor_)) {
            active.push_back(static_cast<DiagnosticDefinition>(reg));
        }
    }
    return active;
}

size_t DiagnosticsManager::subscribe(std::function<void()> listener) {
    if (!listener) return 0;
    const size_t id = nextListenerId_++;
    listeners_[id] = std::move(listener);
    return id;
}

void DiagnosticsManager::unsubscribe(size_t listenerId) {
    listeners_.erase(listenerId);
}

void DiagnosticsManager::notify() {
    for (const auto& [id, listener] : listeners_) {
        if (listener) {
            listener();
        }
    }
}

void DiagnosticsManager::clear() noexcept {
    registrations_.clear();
    notify();
}

void registerTranscriptionDiagnostics(DiagnosticsManager& diagnostics) {
    DiagnosticRegistration reg;
    reg.id = "transcription.no_audio";
    reg.scope = TRANSCRIPTION_DIAGNOSTICS_SCOPE;
    reg.severity = DiagnosticSeverity::Caution;
    reg.message = "No audio detected. Add a clip with audio to the timeline first.";
    reg.check = [](EditorCore& editor) -> bool {
        // Return true if NO audio detected
        auto* project = editor.project().getActive();
        if (!project) return false;

        const auto* timeline = project->activeTimeline();
        if (!timeline) return true;

        // Check audio tracks
        for (const auto& track : timeline->audioTracks()) {
            if (!track.clips().empty()) {
                return false; // Has audio!
            }
        }
        // Check overlay & main tracks for audio clips
        for (const auto* track : timeline->allTracks()) {
            for (const auto& clip : track->clips()) {
                if (clip.type() == ClipType::Audio) {
                    return false; // Has audio!
                }
            }
        }
        return true; // No audio detected -> diagnostic is active
    };

    diagnostics.registerDiagnostic(std::move(reg));
}

} // namespace catchim::editor

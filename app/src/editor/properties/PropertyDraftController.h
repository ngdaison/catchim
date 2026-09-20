#pragma once
// Feature 225 -- mirrors web/src/components/editor/panels/properties/hooks/use-property-draft.ts
#include <string>
#include <functional>
#include <optional>

namespace catchim::editor {

class PropertyDraftController {
public:
    static bool looksLikeExpression(const std::string& input);

    PropertyDraftController() = default;
    explicit PropertyDraftController(std::string sourceDisplay);

    bool isEditing() const noexcept { return isEditing_; }
    std::string displayValue() const;

    void setSourceDisplay(std::string sourceDisplay);
    void startEditing();
    void updateDraft(std::string nextDraft);
    void cancelEditing();

    // Evaluates draft (if math expression) and returns final committed string
    std::string commitEditing(bool supportsExpressions = true);

    void scrubTo(double value, const std::function<void(double)>& onPreview);

private:
    std::string sourceDisplay_;
    std::string draft_;
    bool isEditing_{false};
};

} // namespace catchim::editor

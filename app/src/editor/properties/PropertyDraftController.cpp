// Feature 225 -- mirrors web/src/components/editor/panels/properties/hooks/use-property-draft.ts
#include "editor/properties/PropertyDraftController.h"
#include "core/math/MathExpressionEvaluator.h"
#include <algorithm>

namespace catchim::editor {

bool PropertyDraftController::looksLikeExpression(const std::string& input) {
    size_t first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return false;
    size_t last = input.find_last_not_of(" \t\r\n");
    std::string trimmed = input.substr(first, last - first + 1);

    if (trimmed.find_first_of("+*/") != std::string::npos) {
        return true;
    }
    size_t minusPos = trimmed.find('-');
    return minusPos != std::string::npos && minusPos > 0;
}

PropertyDraftController::PropertyDraftController(std::string sourceDisplay)
    : sourceDisplay_(std::move(sourceDisplay)) {}

std::string PropertyDraftController::displayValue() const {
    return isEditing_ ? draft_ : sourceDisplay_;
}

void PropertyDraftController::setSourceDisplay(std::string sourceDisplay) {
    sourceDisplay_ = std::move(sourceDisplay);
    if (!isEditing_) {
        draft_.clear();
    }
}

void PropertyDraftController::startEditing() {
    isEditing_ = true;
    draft_ = sourceDisplay_;
}

void PropertyDraftController::updateDraft(std::string nextDraft) {
    draft_ = std::move(nextDraft);
}

void PropertyDraftController::cancelEditing() {
    isEditing_ = false;
    draft_.clear();
}

std::string PropertyDraftController::commitEditing(bool supportsExpressions) {
    if (!isEditing_) {
        return sourceDisplay_;
    }

    std::string finalVal = draft_;
    if (supportsExpressions && looksLikeExpression(draft_)) {
        auto evalResult = core::MathExpressionEvaluator::evaluateMathExpression(draft_);
        if (evalResult.has_value()) {
            finalVal = core::MathExpressionEvaluator::formatNumberForDisplay(*evalResult);
        }
    }

    isEditing_ = false;
    draft_.clear();
    sourceDisplay_ = finalVal;
    return finalVal;
}

void PropertyDraftController::scrubTo(double value, const std::function<void(double)>& onPreview) {
    if (onPreview) {
        onPreview(value);
    }
}

} // namespace catchim::editor

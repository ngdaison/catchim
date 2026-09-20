#include "PatternCraftGradients.h"

#include <cctype>

namespace catchim::render {

const std::vector<std::string>& PatternCraftGradients::getGradients() {
    static const std::vector<std::string> s_gradients = {
        "radial-gradient(circle at 30% 70%, rgba(173, 216, 230, 0.35), transparent 60%), radial-gradient(circle at 70% 30%, rgba(255, 182, 193, 0.4), transparent 60%), white",
        "radial-gradient(circle at 20% 80%, rgba(255, 182, 153, 0.3) 0%, transparent 50%), radial-gradient(circle at 80% 20%, rgba(255, 244, 214, 0.5) 0%, transparent 50%), radial-gradient(circle at 40% 40%, rgba(255, 182, 153, 0.1) 0%, transparent 50%), #fff8f0",
        "radial-gradient(circle at 20% 80%, rgba(255, 160, 146, 0.25) 0%, transparent 50%), radial-gradient(circle at 80% 20%, rgba(255, 244, 228, 0.3) 0%, transparent 50%), radial-gradient(circle at 40% 40%, rgba(255, 160, 146, 0.15) 0%, transparent 50%), #fef9f7",
        "radial-gradient(circle at center, #8FFFB0, transparent), white",
        "radial-gradient(circle at top right, rgba(173, 109, 244, 0.5), transparent 70%), white",
        "radial-gradient(circle at top right, rgba(56, 193, 182, 0.5), transparent 70%), white",
        "radial-gradient(circle at top right, rgba(255, 140, 60, 0.5), transparent 70%), white",
        "radial-gradient(circle at top right, rgba(70, 130, 180, 0.5), transparent 70%), white",
        "radial-gradient(circle at top left, rgba(173, 109, 244, 0.5), transparent 70%), white",
        "linear-gradient(120deg, #d5c5ff 0%, #a7f3d0 50%, #f0f0f0 100%)",
        "radial-gradient(ellipse 85% 65% at 8% 8%, rgba(175, 109, 255, 0.42), transparent 60%), radial-gradient(ellipse 75% 60% at 75% 35%, rgba(255, 235, 170, 0.55), transparent 62%), radial-gradient(ellipse 70% 60% at 15% 80%, rgba(255, 100, 180, 0.40), transparent 62%), radial-gradient(ellipse 70% 60% at 92% 92%, rgba(120, 190, 255, 0.45), transparent 62%), linear-gradient(180deg, #f7eaff 0%, #fde2ea 100%)",
        "radial-gradient(ellipse 80% 60% at 70% 20%, rgba(175, 109, 255, 0.85), transparent 68%), radial-gradient(ellipse 70% 60% at 20% 80%, rgba(255, 100, 180, 0.75), transparent 68%), radial-gradient(ellipse 60% 50% at 60% 65%, rgba(255, 235, 170, 0.98), transparent 68%), radial-gradient(ellipse 65% 40% at 50% 60%, rgba(120, 190, 255, 0.3), transparent 68%), linear-gradient(180deg, #f7eaff 0%, #fde2ea 100%)",
        "linear-gradient(135deg, #F8BBD9 0%, #FDD5B4 25%, #FFF2CC 50%, #E1F5FE 75%, #BBDEFB 100%)",
        "linear-gradient(180deg, rgba(245,245,220,1) 0%, rgba(255,223,186,0.8) 25%, rgba(255,182,193,0.6) 50%, rgba(147,112,219,0.7) 75%, rgba(72,61,139,0.9) 100%), radial-gradient(circle at 30% 20%, rgba(255,255,224,0.4) 0%, transparent 50%), radial-gradient(circle at 70% 80%, rgba(72,61,139,0.6) 0%, transparent 70%), radial-gradient(circle at 50% 60%, rgba(147,112,219,0.3) 0%, transparent 60%)",
        "linear-gradient(45deg, #FFB3D9 0%, #FFD1DC 20%, #FFF0F5 40%, #E6F3FF 60%, #D1E7FF 80%, #C7E9F1 100%)",
        "linear-gradient(270deg, #FFECB3 0%, #FFE0B2 20%, #FFCDD2 40%, #F8BBD9 60%, #E1BEE7 80%, #D1C4E9 100%)",
        "radial-gradient(circle at 50% 100%, rgba(255, 69, 0, 0.6) 0%, transparent 60%), radial-gradient(circle at 50% 100%, rgba(255, 140, 0, 0.4) 0%, transparent 70%), radial-gradient(circle at 50% 100%, rgba(255, 215, 0, 0.3) 0%, transparent 80%)",
        "radial-gradient(ellipse at 20% 30%, rgba(56, 189, 248, 0.4) 0%, transparent 60%), radial-gradient(ellipse at 80% 70%, rgba(139, 92, 246, 0.3) 0%, transparent 70%), radial-gradient(ellipse at 60% 20%, rgba(236, 72, 153, 0.25) 0%, transparent 50%), radial-gradient(ellipse at 40% 80%, rgba(34, 197, 94, 0.2) 0%, transparent 65%), #000000",
        "radial-gradient(70% 55% at 50% 50%, #2a5d77 0%, #184058 18%, #0f2a43 34%, #0a1b30 50%, #071226 66%, #040d1c 80%, #020814 92%, #01040d 97%, #000309 100%), radial-gradient(160% 130% at 10% 10%, rgba(0,0,0,0) 38%, #000309 76%, #000208 100%), radial-gradient(160% 130% at 90% 90%, rgba(0,0,0,0) 38%, #000309 76%, #000208 100%)",
        "linear-gradient(0deg, rgba(0,0,0,0.6), rgba(0,0,0,0.6)), radial-gradient(68% 58% at 50% 50%, #c81e3a 0%, #a51d35 16%, #7d1a2f 32%, #591828 46%, #3c1722 60%, #2a151d 72%, #1f1317 84%, #141013 94%, #0a0a0a 100%), radial-gradient(90% 75% at 50% 50%, rgba(228,42,66,0.06) 0%, rgba(228,42,66,0) 55%), radial-gradient(150% 120% at 8% 8%, rgba(0,0,0,0) 42%, #0b0a0a 82%, #070707 100%), radial-gradient(150% 120% at 92% 92%, rgba(0,0,0,0) 42%, #0b0a0a 82%, #070707 100%), radial-gradient(60% 50% at 50% 60%, rgba(240,60,80,0.06), rgba(0,0,0,0) 60%), #050505",
        "radial-gradient(ellipse 70% 55% at 50% 50%, rgba(255, 20, 147, 0.15), transparent 50%), radial-gradient(ellipse 160% 130% at 10% 10%, rgba(0, 255, 255, 0.12), transparent 60%), radial-gradient(ellipse 160% 130% at 90% 90%, rgba(138, 43, 226, 0.18), transparent 65%), radial-gradient(ellipse 110% 50% at 80% 30%, rgba(255, 215, 0, 0.08), transparent 40%), #000000",
        "radial-gradient(circle at 50% 50%, rgba(147, 51, 234, 0.2) 0%, rgba(147, 51, 234, 0.12) 25%, rgba(147, 51, 234, 0.05) 35%, transparent 50%), #000000"
    };
    return s_gradients;
}

const std::vector<std::string>& PatternCraftGradients::getSolidColors() {
    static const std::vector<std::string> s_solidColors = {
        "#ffffff", "#000000", "#ffe2e2", "#ffc9c9", "#ffa2a2", "#ff6467", "#fb2c36", "#e7000b", "#c10007", "#9f0712", "#82181a", "#460809",
        "#fff7ed", "#ffedd4", "#ffd6a7", "#ffb86a", "#ff8904", "#ff6900", "#f54900", "#ca3500", "#9f2d00", "#7e2a0c", "#441306",
        "#fffbeb", "#fef3c6", "#fee685", "#ffd230", "#ffb900", "#fe9a00", "#e17100", "#bb4d00", "#973c00", "#7b3306", "#461901",
        "#fefce8", "#fef9c2", "#fff085", "#ffdf20", "#fdc700", "#f0b100", "#d08700", "#a65f00", "#894b00", "#733e0a", "#432004",
        "#f7fee7", "#ecfcca", "#d8f999", "#bbf451", "#9ae600", "#7ccf00", "#5ea500", "#497d00", "#3c6300", "#35530e", "#192e03",
        "#f0fdf4", "#dcfce7", "#b9f8cf", "#7bf1a8", "#05df72", "#00c950", "#00a63e", "#008236", "#016630", "#0d542b", "#032e15",
        "#ecfdf5", "#d0fae5", "#a4f4cf", "#5ee9b5", "#00d492", "#00bc7d", "#009966", "#007a55", "#006045", "#004f3b", "#002c22",
        "#f0fdfa", "#cbfbf1", "#96f7e4", "#46ecd5", "#00d5be", "#00bba7", "#009689", "#00786f", "#005f5a", "#0b4f4a", "#022f2e",
        "#ecfeff", "#cefafe", "#a2f4fd", "#53eafd", "#00d3f2", "#00b8db", "#0092b8", "#007595", "#005f78", "#104e64", "#042f40"
    };
    return s_solidColors;
}

std::optional<std::string> PatternCraftGradients::findGradientByIndex(size_t index) {
    const auto& gradients = getGradients();
    if (index < gradients.size()) {
        return gradients[index];
    }
    return std::nullopt;
}

bool PatternCraftGradients::isValidHexColor(const std::string& hex) noexcept {
    if (hex.empty() || hex[0] != '#') {
        return false;
    }
    if (hex.size() != 4 && hex.size() != 7 && hex.size() != 9) {
        return false;
    }
    for (size_t i = 1; i < hex.size(); ++i) {
        if (!std::isxdigit(static_cast<unsigned char>(hex[i]))) {
            return false;
        }
    }
    return true;
}

} // namespace catchim::render

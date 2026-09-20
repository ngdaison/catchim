#include "ExportMimeTypesAndLayers.h"

namespace catchim::editor {

std::string_view ExportMimeTypes::getMimeTypeForExtension(std::string_view ext) noexcept {
    if (ext == "webm" || ext == ".webm") {
        return WEBM;
    }
    if (ext == "mp4" || ext == ".mp4") {
        return MP4;
    }
    return MP4;
}

std::string_view ExportMimeTypes::getExtensionForMimeType(std::string_view mime) noexcept {
    if (mime == WEBM) {
        return "webm";
    }
    if (mime == MP4) {
        return "mp4";
    }
    return "mp4";
}

const char* PreviewPenCursor::getSvgContent() noexcept {
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\">\n"
           "  <path d=\"M 1 1 L 5 2 L 13 10 L 10 13 L 2 5 Z\" fill=\"white\" stroke=\"#111\" stroke-width=\"1\" stroke-linejoin=\"round\"/>\n"
           "  <path d=\"M 1 1 L 5 2 L 2 5 Z\" fill=\"#111\"/>\n"
           "</svg>";
}

} // namespace catchim::editor

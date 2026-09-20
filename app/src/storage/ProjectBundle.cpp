#include "storage/ProjectBundle.h"
#include <fstream>
#include <unordered_set>

namespace catchim::storage {

BundleValidationReport ProjectBundle::validateMediaReferences(
    const editor::Project& project,
    const std::filesystem::path& mediaSearchDir
) {
    BundleValidationReport report;
    std::unordered_set<std::string> checkedIds;

    for (const auto& scene : project.scenes()) {
        for (const auto* track : scene.timeline().allTracks()) {
            for (const auto& clip : track->clips()) {
                if (!clip.mediaId().isEmpty()) {
                    std::string mid = clip.mediaId().str();
                    if (checkedIds.insert(mid).second) {
                        report.totalMediaReferences++;
                        bool found = false;
                        if (std::filesystem::exists(mediaSearchDir)) {
                            for (const auto& entry : std::filesystem::directory_iterator(mediaSearchDir)) {
                                if (entry.path().stem().string().find(mid) != std::string::npos ||
                                    entry.path().filename().string().find(mid) != std::string::npos) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (found) {
                            report.foundCount++;
                        } else {
                            report.missingCount++;
                            report.missingIds.push_back(mid);
                        }
                    }
                }
            }
        }
    }

    return report;
}

bool ProjectBundle::createBundle(
    const editor::Project& project,
    const std::filesystem::path& targetBundleDir,
    const std::filesystem::path& sourceMediaDir
) {
    std::error_code ec;
    std::filesystem::create_directories(targetBundleDir / "media", ec);
    if (ec) return false;

    // Serialize and write project.json
    nlohmann::json json = editor::ProjectSerializer::serialize(project);
    std::ofstream ofs(targetBundleDir / "project.json");
    if (!ofs.is_open()) return false;
    ofs << json.dump(2);
    ofs.close();

    // Copy any matching media from sourceMediaDir
    if (std::filesystem::exists(sourceMediaDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(sourceMediaDir)) {
            if (entry.is_regular_file()) {
                std::filesystem::copy_file(
                    entry.path(),
                    targetBundleDir / "media" / entry.path().filename(),
                    std::filesystem::copy_options::overwrite_existing,
                    ec
                );
            }
        }
    }

    return true;
}

size_t ProjectBundle::relinkMedia(
    nlohmann::json& projectJson,
    const std::unordered_map<std::string, std::string>& oldToNewPaths
) {
    size_t relinkedCount = 0;
    if (oldToNewPaths.empty()) return 0;

    auto checkAndReplace = [&](nlohmann::json& val) {
        if (val.is_string()) {
            std::string str = val.get<std::string>();
            auto it = oldToNewPaths.find(str);
            if (it != oldToNewPaths.end()) {
                val = it->second;
                relinkedCount++;
            }
        }
    };

    // Recursively walk JSON to replace any matching path strings
    std::function<void(nlohmann::json&)> walk = [&](nlohmann::json& node) {
        if (node.is_object()) {
            for (auto& [k, v] : node.items()) {
                if (v.is_string()) {
                    checkAndReplace(v);
                } else if (v.is_structured()) {
                    walk(v);
                }
            }
        } else if (node.is_array()) {
            for (auto& elem : node) {
                if (elem.is_string()) {
                    checkAndReplace(elem);
                } else if (elem.is_structured()) {
                    walk(elem);
                }
            }
        }
    };

    walk(projectJson);
    return relinkedCount;
}

} // namespace catchim::storage

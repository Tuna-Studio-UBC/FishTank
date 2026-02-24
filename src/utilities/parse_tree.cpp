#include "parse_tree.hpp"

std::unordered_map<std::string, AIAction> action_map = {
    {"PURSUE", AIAction::PURSUE}, 
    {"IDLE", AIAction::IDLE}, 
    {"DISTRACTED", AIAction::DISTRACTED}
};

AIAction str_to_ai_action(const std::string &str) {
    auto it = action_map.find(str);
    return it != action_map.end() ? it->second : AIAction::IDLE;
}

inline std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

ScriptedTree load_tree(const std::string &filename) {
    ScriptedTree tree;
    spdlog::debug("parsing file {}", filename);

    const std::string base_path = DECISION_TREES;
    std::string full_path = base_path + filename;

    std::ifstream file(full_path);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string token;

        TreeNode node;
        std::getline(ss, token, ',');
        node.id = std::stoi(trim(token));
        std::getline(ss, token, ',');
        node.type = (trim(token) == "Condition") ? NodeType::CONDITION : NodeType::ACTION;
        std::getline(ss, token, ',');
        node.condition_name = trim(token);
        std::getline(ss, token, ',');
        node.true_child = trim(token).empty() ? -1 : std::stoi(trim(token));
        std::getline(ss, token, ',');
        node.false_child = trim(token).empty() ? -1 : std::stoi(trim(token));
        std::getline(ss, token, ',');
        if (!trim(token).empty()) {
            node.action = str_to_ai_action(trim(token));
        }

        tree.nodes[node.id] = node;
        spdlog::debug("parsed node {}", node.id);
    }

    if (!tree.nodes.empty()) {
        tree.root_id = 1;
    }

    for (auto &p : tree.nodes) {
        spdlog::debug("Node {} type {} cond '{}' true_child {} false_child {}", p.first,
                      (p.second.type == NodeType::CONDITION ? "CONDITION" : "ACTION"), p.second.condition_name,
                      p.second.true_child, p.second.false_child);
    }
    return tree;
}
#include "pcg/pcg_trace.h"

namespace atlas {
namespace pcg {

PCGTraceRecorder::PCGTraceRecorder() {
    reset();
}

void PCGTraceRecorder::reset() {
    root_ = {};
    root_.label  = "Root";
    root_.valid  = true;
    stack_.clear();
    stack_.push_back(&root_);
}

void PCGTraceRecorder::pushNode(const PCGTraceNode& node) {
    if (stack_.empty()) return;

    PCGTraceNode* parent = stack_.back();
    parent->children.push_back(node);
    stack_.push_back(&parent->children.back());
}

void PCGTraceRecorder::popNode() {
    if (stack_.size() > 1) {
        stack_.pop_back();
    }
}

void PCGTraceRecorder::annotate(const std::string& label) {
    if (stack_.empty()) return;

    PCGTraceNode leaf{};
    leaf.label = label;
    leaf.valid = true;
    stack_.back()->children.push_back(leaf);
}

void PCGTraceRecorder::setValid(bool v) {
    if (!stack_.empty()) {
        stack_.back()->valid = v;
    }
}

const PCGTraceNode& PCGTraceRecorder::root() const {
    return root_;
}

bool PCGTraceRecorder::hasTrace() const {
    return !root_.children.empty();
}

std::string PCGTraceRecorder::dump() const {
    std::string out;
    for (size_t i = 0; i < root_.children.size(); ++i) {
        bool last = (i == root_.children.size() - 1);
        dumpRecursive(root_.children[i], "", last, out);
    }
    return out;
}

void PCGTraceRecorder::dumpRecursive(const PCGTraceNode& node,
                                      const std::string& prefix,
                                      bool last,
                                      std::string& out) {
    out += prefix;
    out += last ? "└─ " : "├─ ";
    out += node.label;

    if (node.seed != 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), " [0x%016llX]",
                 static_cast<unsigned long long>(node.seed));
        out += buf;
    }

    if (!node.valid) {
        out += " (INVALID)";
    }
    out += "\n";

    std::string childPrefix = prefix + (last ? "   " : "│  ");
    for (size_t i = 0; i < node.children.size(); ++i) {
        bool childLast = (i == node.children.size() - 1);
        dumpRecursive(node.children[i], childPrefix, childLast, out);
    }
}

} // namespace pcg
} // namespace atlas

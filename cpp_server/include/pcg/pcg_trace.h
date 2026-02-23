#ifndef EVE_PCG_TRACE_H
#define EVE_PCG_TRACE_H

#include "pcg_context.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Single node in the PCG debug trace tree.
 *
 * Every procedural generation step emits a trace node so that the
 * entire seed → object derivation can be inspected after the fact.
 * This is essential for debugging determinism regressions and
 * multiplayer desyncs.
 */
struct PCGTraceNode {
    uint64_t    seed;
    PCGDomain   domain;
    uint64_t    objectId;
    std::string label;
    bool        valid;   ///< Did the generation pass constraints?

    std::vector<PCGTraceNode> children;
};

/**
 * @brief Records a hierarchical trace of all PCG operations.
 *
 * Generators call pushNode() at entry and popNode() at exit.  The
 * resulting tree can be dumped for debug visualization:
 *
 *   UniverseSeed: 0xDEADBEEF
 *   └─ Sector(42)
 *      └─ Ship(1007)
 *         ├─ Hull: Cruiser_Mk2
 *         ├─ Engines: 3x IonDrive
 *         ├─ Weapons: 6x MediumRail
 *         └─ Result: VALID
 *
 * Thread-safety: NOT thread-safe.  Use one recorder per thread or
 * synchronise externally.
 */
class PCGTraceRecorder {
public:
    PCGTraceRecorder();

    /** Start a new trace tree, discarding any previous data. */
    void reset();

    /** Push a child node under the current scope. */
    void pushNode(const PCGTraceNode& node);

    /** Pop back to the parent scope. */
    void popNode();

    /** Add a leaf annotation to the current node (no push/pop). */
    void annotate(const std::string& label);

    /** Mark the current node as valid / invalid. */
    void setValid(bool v);

    /** @return Read-only reference to the root of the trace tree. */
    const PCGTraceNode& root() const;

    /** @return true if any nodes have been recorded. */
    bool hasTrace() const;

    /** Flatten the tree into a human-readable multi-line string. */
    std::string dump() const;

private:
    PCGTraceNode              root_;
    std::vector<PCGTraceNode*> stack_;   ///< Current path from root.

    static void dumpRecursive(const PCGTraceNode& node,
                              const std::string& prefix,
                              bool last,
                              std::string& out);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_TRACE_H

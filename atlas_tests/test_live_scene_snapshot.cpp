/**
 * Tests for LiveSceneManager snapshot/rollback:
 *   - TakeSnapshot captures current state
 *   - SnapshotCount increments
 *   - SnapshotLabel returns correct labels
 *   - RollbackToSnapshot restores state
 *   - Rollback out-of-bounds returns false
 */

#include "tools/LiveSceneManager.h"
#include "tools/PCGOverrideStore.h"
#include "tools/ViewportPanel.h"
#include "tools/PCGPreviewPanel.h"
#include <cassert>
#include <string>

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// LiveSceneManager Snapshot/Rollback tests
// ══════════════════════════════════════════════════════════════════

void test_snapshot_count_default() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);
    assert(lsm.SnapshotCount() == 0);
}

void test_snapshot_take() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();
    size_t idx = lsm.TakeSnapshot("Before edit");
    assert(idx == 0);
    assert(lsm.SnapshotCount() == 1);
    assert(lsm.SnapshotLabel(0) == "Before edit");
}

void test_snapshot_auto_label() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();
    size_t idx = lsm.TakeSnapshot();
    assert(idx == 0);
    assert(lsm.SnapshotLabel(0) == "Snapshot 0");
}

void test_snapshot_multiple() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();

    lsm.TakeSnapshot("First");
    lsm.TakeSnapshot("Second");
    lsm.TakeSnapshot("Third");

    assert(lsm.SnapshotCount() == 3);
    assert(lsm.SnapshotLabel(0) == "First");
    assert(lsm.SnapshotLabel(1) == "Second");
    assert(lsm.SnapshotLabel(2) == "Third");
}

void test_snapshot_rollback() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.SetSeed(42);
    lsm.PopulateDefaultScene();
    lsm.TakeSnapshot("Original");

    // Change the seed
    lsm.SetSeed(999);
    lsm.TakeSnapshot("Modified");

    assert(lsm.CurrentSeed() == 999);

    // Rollback to the original snapshot
    bool ok = lsm.RollbackToSnapshot(0);
    assert(ok);
    assert(lsm.CurrentSeed() == 42);
}

void test_snapshot_rollback_out_of_bounds() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    bool ok = lsm.RollbackToSnapshot(0);
    assert(!ok);

    lsm.TakeSnapshot("Only");
    ok = lsm.RollbackToSnapshot(99);
    assert(!ok);
}

void test_snapshot_preserves_overrides() {
    ViewportPanel vp;
    PCGPreviewPanel pcg;
    LiveSceneManager lsm(&vp, &pcg);

    lsm.PopulateDefaultScene();

    // Take a snapshot before adding any overrides
    lsm.TakeSnapshot("Clean");
    assert(lsm.OverrideStore().Overrides().empty());

    // Add an override by moving an object
    vp.SelectObject(1);
    vp.TranslateSelected(50.0f, 0.0f, 0.0f);
    lsm.CaptureViewportChanges();
    assert(!lsm.OverrideStore().Overrides().empty());

    // Rollback to the clean snapshot
    bool ok = lsm.RollbackToSnapshot(0);
    assert(ok);
    assert(lsm.OverrideStore().Overrides().empty());
}

/**
 * Tests for MultiSelectionManager:
 *   - Default state (empty)
 *   - Select / deselect
 *   - Toggle
 *   - IsSelected
 *   - Count and Empty
 *   - ClearSelection
 *   - SetSelection (replace)
 *   - SelectMultiple (additive)
 *   - SelectedSorted ordering
 *   - Duplicate select is no-op
 */

#include <cassert>
#include <algorithm>
#include "../cpp_client/include/editor/multi_selection_manager.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// MultiSelectionManager tests
// ══════════════════════════════════════════════════════════════════

void test_multi_sel_defaults() {
    MultiSelectionManager sel;
    assert(sel.Count() == 0);
    assert(sel.Empty());
    assert(sel.SelectedSorted().empty());
}

void test_multi_sel_select() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.Select(2);
    assert(sel.Count() == 2);
    assert(!sel.Empty());
    assert(sel.IsSelected(1));
    assert(sel.IsSelected(2));
}

void test_multi_sel_select_duplicate() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.Select(1);
    assert(sel.Count() == 1);
}

void test_multi_sel_deselect() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.Select(2);
    assert(sel.Deselect(1));
    assert(sel.Count() == 1);
    assert(!sel.IsSelected(1));
    assert(sel.IsSelected(2));
}

void test_multi_sel_deselect_not_found() {
    MultiSelectionManager sel;
    assert(!sel.Deselect(99));
}

void test_multi_sel_toggle() {
    MultiSelectionManager sel;
    assert(sel.Toggle(1) == true);  // now selected
    assert(sel.IsSelected(1));

    assert(sel.Toggle(1) == false); // now deselected
    assert(!sel.IsSelected(1));
    assert(sel.Count() == 0);
}

void test_multi_sel_clear() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.Select(2);
    sel.Select(3);
    sel.ClearSelection();
    assert(sel.Count() == 0);
    assert(sel.Empty());
}

void test_multi_sel_set_selection() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.Select(2);
    sel.SetSelection({10, 20, 30});
    assert(sel.Count() == 3);
    assert(!sel.IsSelected(1));
    assert(!sel.IsSelected(2));
    assert(sel.IsSelected(10));
    assert(sel.IsSelected(20));
    assert(sel.IsSelected(30));
}

void test_multi_sel_select_multiple() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.SelectMultiple({2, 3, 4});
    assert(sel.Count() == 4);
    assert(sel.IsSelected(1));
    assert(sel.IsSelected(4));
}

void test_multi_sel_select_multiple_overlap() {
    MultiSelectionManager sel;
    sel.Select(1);
    sel.SelectMultiple({1, 2, 3});
    assert(sel.Count() == 3);
}

void test_multi_sel_sorted() {
    MultiSelectionManager sel;
    sel.Select(5);
    sel.Select(1);
    sel.Select(3);
    sel.Select(8);
    auto sorted = sel.SelectedSorted();
    assert(sorted.size() == 4);
    assert(sorted[0] == 1);
    assert(sorted[1] == 3);
    assert(sorted[2] == 5);
    assert(sorted[3] == 8);
}

void test_multi_sel_is_selected() {
    MultiSelectionManager sel;
    sel.Select(42);
    assert(sel.IsSelected(42));
    assert(!sel.IsSelected(43));
}

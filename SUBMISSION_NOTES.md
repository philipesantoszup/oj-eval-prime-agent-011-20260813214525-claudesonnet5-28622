# Submission Notes

## Submission 1 (ID: 836579)
- Score: 100/100
- All test groups accepted: one, two, three, four, five (+ .memcheck variants)
- Implementation: Leftist heap (mergeable heap) with O(log n) merge complexity
- Exception safety: mergeNodes() only mutates node fields after successful
  recursive merge calls, so a thrown exception from `cmp` leaves both input
  trees completely unmodified. push/pop/merge catch and rethrow as
  sjtu::runtime_error while guaranteeing state restoration.
- Memory: cloneTree() with rollback-on-exception during copy construction;
  deleteTree() is iterative (no stack overflow) to release all nodes.

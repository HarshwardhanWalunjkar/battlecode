# Current planned work — v8 investigation and implementation

## Request and constraints

Review recent Lozer matches in depth: immediately lost trapped Slithery dragons,
opening hunting/retreat and splitting, late resource urgency and confused feeding.
Address recurring mistakes in our policy, not tuning against a named opponent.
Preserve effective pathfinding and frozen v7, all older releases, MainBaselineBuild,
user notes and research/. MASTER read; focused rules only. Maintain full plain-English
Gamplan, exact versions and timing. No upload. Keep tests conservative and purposeful.

## Plan

1. Verify current source/submission, fetch newest Lozer series and a small distinct
   comparison sample. Reconstruct actions and snapshots; distinguish forced deaths
   from avoidable choices, and direct enemy attacks from our initiated trades.
2. Trace exact opening decisions, body knowledge and legal split alternatives.
   Audit recurring offensive opportunities, evasive choices and resource access.
3. Trace actual endgame station/offer/acknowledgement/death/pickup sequences and
   whether worker movement is useful before any sacrifice. Do not label corpse
   meals intentional without supporting events.
4. Select evidence-backed fixes, implement one complete next version from v7,
   preserving pathfinding and winning cases. Update this tracker before changes.
5. Focused correctness scenarios and small predeclared sandbox validation. Freeze
   and package only the finished source. No intermediate release or submission.

## In progress

Working source is being compared to frozen v7. A local receipt shows v7 was uploaded
by the user; live status and fresh-window markers are being refreshed. Independent
audits are reviewing Slithery initial geometry/splitting and coordination, while
the main agent fetches and reviews new games and opening aggression. Agents write
only reports/diagnostics in evaluations/v8-review; main agent owns bot edits.

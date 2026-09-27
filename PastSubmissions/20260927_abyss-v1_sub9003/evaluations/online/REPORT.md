# First online loss audit — 2026-09-27

Downloaded all **20 game records and original replays** from unranked series
**409862** (blauerdrache, rating 1522) and **409982** (bongcloud, rating 1586).
Both used our submission **9003 / abyss-v1 / version 1**, as team A.
Result: **0 wins, 0 draws, 20 losses**. Nine losses were elimination; eleven
reached the round limit and lost on longest living dragon.

The server returned no match error logs. The replays nevertheless include
actions, deaths, our CPU counts, and full board history. They were decoded with
the parser bundled in the official unswbc 1.2.2 viewer. Opponents' CPU counts
are absent; absence must not be interpreted as zero CPU use.

## Confirmed findings

1. **We repeatedly spend scarce dragons on one-for-one trades.** Of 114 own
   deaths in head collisions, 99 came from our turns initiating a collision
   with an enemy, nine came from an enemy initiating, and six came from three
   collisions between our own dragons. This is not simply an enemy outmaneuvering
   us in every fight. Our policy explicitly prioritizes qualifying head trades
   ahead of ordinary movement. It checks local length and whether an ally exists,
   but does not require that we can sustain the exchange against their population.
   The replay identifies the initiator; it does not prove every trade was avoidable.
2. **Both opponents build much larger populations.** Their peak living counts
   range from 21 to 64 across these games. Our normal expansion target is four,
   with exceptions for starting units and emergency splitting. On Autarky
   against bongcloud, at the start of round 100 we have two dragons and four
   total segments; they have 31 dragons and 71 segments. They ultimately win
   with 40 dragons, despite their longest being only four segments.
   The tactical implication is that preserving one large scorer cannot work
   without enough surviving support and access to food. Simply removing the
   population cap is a hypothesis to test, not a demonstrated fix.
3. **Our portal policy prevents productive exploration.** In both Portals games,
   our three starting dragons never make a non-adjacent head transition, collect
   no net length, and finish with two dragons of lengths three and two.
   At round 100 we still have our starting total of nine segments; the opponents
   have 84 and 85. Their recorded non-adjacent head transitions total 691 and 494.
   These counts are a conservative measure of distant portal travel, not a count
   of every possible portal crossing. Our code refuses an unseen exit whenever
   an ordinary safe move exists, allowing endless movement without entering
   resource-rich areas. This is a concrete policy weakness, not evidence of a
   portal transition arithmetic bug.
4. **Some map starts require better handling of long bodies and escape.** Our
   original dragon 0 hits kelp in round 3 on both Autarky games. Dragon 0 dies
   in round 2 on both Prisoners Dilemma games; dragons 0, 2 and 8 do likewise
   on both Slithery Fight games. The code disables all splits and sprints when
   it cannot reconstruct the entire body, including bodies extending outside
   initial vision. A short local diagnostic on Autarky reproduces dragon 0's
   four decisions and its deliberate fatal fallback with only five of its 14
   segments reconstructed. However, other dragons diverge from the server
   replay, so this is **not** a fully reproduced match or proof of a particular
   successful alternative. Targeted map-start tests are needed.
5. **The judge budget is not causing these losses.** Across **27,586 own turns**,
   the replay records zero timeouts or budget-exceeded flags. Peak reported
   usage is **21,997,079 / 100,000,000 points**. Own deaths are 114 head
   collisions, 30 kelp collisions, ten other-body collisions, and one self
   collision; none has the no-valid-action reason.

## What to change and test next

These are proposed changes, not implemented behavior:

- Replace the automatic small-dragon head trade with an exchange decision that
  accounts for our remaining support, enemy crowding, and a safe alternative.
- Test faster expansion against swarm opponents, including early replacement
  of losses and a later switch toward growing surviving scorers.
- Allow calculated portal exploration when safe local movement produces no
  food or access to new territory; test unseen and occupied exits separately.
- Handle partially visible starting bodies without automatically ruling out
  every legal split or useful sprint. Test the repeated fatal map starts first.
- Add these opponents' observed population patterns and the actual online maps
  to evaluation. The earlier weak local baselines did not establish competitive
  strength; their win rate should not have been treated as evidence of a best
  strategy.

The large number of opponent deaths alone does not establish intentional food
transfer. blauerdrache has explicit suicide actions; bongcloud has many self
collisions. Their purpose cannot be inferred reliably from counts alone.

## Reproduce the audit

The original `.replay` files, raw details, decoded `.events.json`, and
`summary.json` are retained in this directory. Raw JSON and replays are excluded
from version control to avoid committing bulky account records. The fetching
tool keeps bearer credentials on the contest host when following replay redirects.

```sh
.venv/bin/python tools/decode_replays.py evaluations/online/*.replay
.venv/bin/python tools/analyze_replays.py
```

`tools/trace_replay.py` is an exploratory diagnostic that plays recorded actions
through the local engine while inspecting our decisions. Its saved runs report
global replay mismatches and are not valid opponent benchmarks or proof of
server-identical reproduction. The confirmed counts above come directly from
the downloaded server replays.

## Submission timing

The server snapshot confirms version 1 is active, both challenge sets are
unranked, and `kStart` / `kFreshAt` are null. No ranked history was returned.
These games do not establish a fresh-bot window start. Snapshot verification is
time-specific; autoscrims or later user activity can change this.

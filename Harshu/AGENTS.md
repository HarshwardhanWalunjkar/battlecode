# Battlecode project reference

For bot design, implementation, debugging, or evaluation, read
[documentation/MASTER.md](documentation/MASTER.md) first. Read the complete
documentation only when the user explicitly requests a complete review. If the
master does not answer a question, read only the relevant focused documentation.

Use [documentation/README.md](documentation/README.md) to find focused references
and official sources. Distinguish documented rules from strategic inferences and
unresolved source conflicts. Do not describe proposed tactics as engine-tested.

When rules or toolkit behavior change, update the master and affected focused
pages together. Record the source/review date and any tested engine version.

## Collaboration and strategy

- Maintain `current planned work.md` before implementation and update it as
  work completes. Include the objective, current decisions, changed files,
  verification performed, remaining work and constraints so another model can
  resume without repeating the investigation.
- Maintain `Gamplan.md` (the user's chosen spelling), with clear sections for
  movement, food, splitting, sonar, opponents, and every other implemented policy.
  Use plain English, explain unfamiliar terms, and include every threshold,
  exception, priority, and tradeoff needed to understand the actual strategy.
  Keep implemented behavior separate from proposed ideas and measured results.
- For suggestions, start with the current strategy and the relevant question,
  not a full context/documentation reread. Reject clear degradations briefly
  with a reason. For plausible improvements, check the master and then only the
  needed focused rules before giving a concise comparison.
- Track submission uploads, code identity, first ranked play, effective ranked
  count, and the team's last fresh-bot window in `submissions/`. Upload time is
  not the start of the 12-hour Elo window. Never invent unknown server history.
- Give explicit steps and a brief plain-English reason when user action is
  required; a submission request normally only needs its command.
- Prioritize rank/rating and practical human-designed opponents. Consider
  predictable habits, adaptation, and public replay exposure, but do not weaken
  play merely to hide strategy. Label untested opponent assumptions.

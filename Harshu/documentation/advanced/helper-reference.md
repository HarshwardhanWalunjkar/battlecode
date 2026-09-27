# Helper Reference

[Master reference](../MASTER.md#helper-and-protocol) · [Index](../README.md)

Source: [Official Helper Reference](https://game.battlecode.au/docs/helper) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Python member lookup

| Object | Members |
| --- | --- |
| `Direction` | `value`, `get_direction_list`, `get_offset`, `get_opposite`, `get_left`, `get_right` |
| `Team` | `get_enemy_team` |
| `Position` | `add_dir`, `is_in_map`, `is_in_vision` |
| `Game` | `get_round_num`, `get_map_size`, `get_unit_limit` |
| `Vision` | `get_tiles`, `get_tile` |
| `Tile` | `edges`, `get_edge`, `get_dragon`, `has_pearl`, `get_pearl_time`, `get_position` |
| `DragonPart` | `get_position`, `get_id`, `get_team`, `get_dir`, `is_head` |
| `Edge` | `is_passable`, `is_portal`, `get_edge_type`, `get_portal_id` |
| `Controller` state | `get_length`, `get_unit_count`, `get_head`, `get_id`, `get_team`, `get_dir`, `get_vision`, `get_tiles`, `get_tile`, `get_position` |
| `Controller` actions | `make_move`, `make_moves`, `can_split`, `do_split` |
| `Controller` sonar | `get_sonar_messages`, `get_sonar_echoes`, `send_sonar` |
| `Controller` debugging | `output_log`, `draw_indicator_dot`, `draw_indicator_line`, `set_indicator_string` |

Edge types: `EMPTY`, `KELP`, `PORTAL`. Vision returns 49 tiles in row order. A passable edge does not prove a safe destination. Helpers are editable project copies; upgrading the toolkit alone does not refresh an existing copy. Examples differ on whether `Direction.value` is callable: inspect the installed helper.

#pragma once
namespace policy {
constexpr int beam_width = 7;
constexpr int lookahead = 10;
constexpr int candidate_limit = 14;
constexpr int max_sprint = 3;
constexpr int split_length = 6;
constexpr int max_team = 4;
constexpr int split_before = 260;
constexpr int split_cooldown = 12;
constexpr double food_reward = 12.0;
constexpr double sprint_cost = 14.0;
constexpr double danger_one = 130.0;
constexpr double danger_two = 48.0;
constexpr double danger_three = 12.0;
constexpr double search_seconds = 0.052;
}

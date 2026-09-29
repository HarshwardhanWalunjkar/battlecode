#pragma once
#include "maps/autarky.hpp"
#include "maps/default.hpp"
#include "maps/devil.hpp"
#include "maps/dilemma.hpp"
#include "maps/portals.hpp"
#include "maps/queen_of_spades.hpp"
#include "maps/schooltime.hpp"
#include "maps/slithery_fight.hpp"
#include "maps/trauma.hpp"
#include "maps/trophy.hpp"
namespace abyss::atlas {
inline constexpr std::array<const Map*,10> catalog{{&map_autarky::data,&map_default::data,&map_devil::data,&map_dilemma::data,&map_portals::data,&map_queen_of_spades::data,&map_schooltime::data,&map_slithery_fight::data,&map_trauma::data,&map_trophy::data}};
}

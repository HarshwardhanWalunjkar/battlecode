#include "strategy.hpp"
int main() {
    try {
        auto [ct,game]=unswbc::init();
        abyss::Brain brain(game.width,game.height,ct.get_id(),ct.get_team().value=='A'?0:1);
        while(unswbc::update(ct,game)) {
            brain.observe(ct,game);
            auto decision=brain.choose();
            brain.commit(decision,ct);
            unswbc::end_turn();
        }
    } catch(std::exception const&) {
        // The engine closes stdin at death/game end. Never emit garbage after EOF.
        return std::cin.eof()?0:1;
    }
}

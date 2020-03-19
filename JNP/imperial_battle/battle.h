#ifndef __BATTLE_H__
#define __BATTLE_H__

#include "rebelfleet.h"
#include "imperialfleet.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <memory>

using namespace std;


class SpaceBattle {

private:
	vector<shared_ptr<RebelStarship>> rebelShips;
	vector<shared_ptr<ImperialStarship>> imperialShips;
	
	Timer timer;
	Timer::Time time0;
	
	bool battleStarted;

	void attack(shared_ptr<ImperialStarship> &imperial, shared_ptr<RebelStarship> &rebeliant) {
		rebeliant.get()->takeDamage(imperial.get()->getAttackPower());
		if (rebeliant.get()->isAttacker()) {
		    imperial.get()->takeDamage(rebeliant.get()->getAttackPower());
		}
    	}

	void attack_fleet() {
		for(auto imperial : this->imperialShips) {
			for (auto rebeliant : this->rebelShips) {
				if (imperial.get()->getShield() > 0 && rebeliant.get()->getShield() > 0)
					attack(imperial, rebeliant);
			}
		}
	}

	size_t countImperialShips(const shared_ptr<ImperialStarship> &ship) {
		return ship.get()->countShips();
	}
	
	bool AttackTime() {
		if((timer.getTime() % 2 == 0 || timer.getTime() % 3 == 0) && timer.getTime() % 5 != 0) {
		    return true;
		} else {
		    return false;
		}
	}

public:
    SpaceBattle() {
        battleStarted = false;
    }

    size_t countRebelFleet() {
		size_t i = 0;
        
		for(auto ship : rebelShips) {
			if(ship.get()->getShield() > 0) {
				i++;
			}
        	}
		return i;
	}

	size_t countImperialFleet() {
		size_t i = 0;
		for(auto ship : imperialShips) {
			i += countImperialShips(ship);
		}
       		return i;
	}
	
	void tick(Timer::Time timeStep) {
		size_t imperialsAlive = countImperialFleet();
		size_t rebelsAlive = countRebelFleet();

		if (!battleStarted) {
		    battleStarted = true;
		    timer.setTime(time0);
        	}

		if(rebelsAlive == 0) {
			if(imperialsAlive == 0) {
				cout << "DRAW\n";
			} else {
				cout << "IMPERIUM WON\n";
			}
		} else if(imperialsAlive == 0){
			cout << "REBELLION WON\n";
		} else if (isAttackTime()) {
            		attack_fleet();
		}

        	timer.tick(timeStep);
	}

	class Builder {
	private:
		shared_ptr<SpaceBattle> battle;

	public:
		Builder() {
		    battle = make_shared<SpaceBattle>();
		}

		Builder & maxTime(Timer::Time time) {
			battle.get()->timer.setMaxTime(time);
			return *this;
		}

		Builder & startTime(Timer::Time time) {
			battle.get()->time0 = time;
			return *this;
		}

		Builder & ship(shared_ptr<ImperialStarship> ship) {
			battle.get()->imperialShips.push_back(move(ship));
			return *this;
		}

		Builder & ship(shared_ptr<RebelStarship> ship) {
			battle.get()->rebelShips.push_back(move(ship));
			return *this;
		}

		SpaceBattle build() {
			return *battle.get();
		}
	};	
};

#endif // __BATTLE_H__

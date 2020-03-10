#ifndef __IMPERIALFLEET_H__
#define __IMPERIALFLEET_H__

#include "helper.h"

#include <type_traits>
#include <initializer_list>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

class ImperialStarship : public AttackingStarship {
public:

    ImperialStarship(const ShieldPoints &shield, const AttackPower &attackPower) {
        Starship::shield = shield;
        AttackingStarship::attackPower = attackPower;
    }

    ImperialStarship()
            : AttackingStarship() {}

};

class TIEFighter : public ImperialStarship {
public:

    TIEFighter(const ShieldPoints &shield, const AttackPower &attackPower)
            : ImperialStarship(shield, attackPower) {}
};

class ImperialDestroyer : public ImperialStarship {
public:

    ImperialDestroyer(const ShieldPoints &shield, const AttackPower &attackPower)
            : ImperialStarship(shield, attackPower) {}
};

class DeathStar : public ImperialStarship {
public:

    DeathStar(const ShieldPoints &shield, const AttackPower &attackPower)
            : ImperialStarship(shield, attackPower) {}
};

class Squadron : public ImperialStarship {
public:

    Squadron(const initializer_list<shared_ptr<ImperialStarship>> &list)
            : ImperialStarship(),
              ships(list) {}

    explicit Squadron(const vector<shared_ptr<ImperialStarship>> &vect)
            : ImperialStarship(),
              ships(move(vect)) {}


    AttackPower getAttackPower() const override {
        AttackPower attack = 0;
        for (const auto &i : ships)
            if (i.get()->getShield() != 0)
                attack += i.get()->getAttackPower();
        return attack;
    }

    ShieldPoints getShield() const override {
        ShieldPoints shield = 0;
        for (auto &i : ships)
            shield += i.get()->getShield();
        return shield;
    }

    void takeDamage(const AttackPower &dmg) override {
        for (auto &i : ships)
            i.get()->takeDamage(dmg);
    }

    size_t countShips() override {
        size_t shipsCount = 0 ;

        for (auto ship : ships) {
            shipsCount += ship.get()->countShips();
        }

        return shipsCount;
    }

private:
    vector<shared_ptr<ImperialStarship>> ships;
};

using ShieldPoints = Starship::ShieldPoints;
using AttackPower = Starship::AttackPower;

shared_ptr<TIEFighter> createTIEFighter(const ShieldPoints &shield, const AttackPower &attackPower) { 
    return make_shared<TIEFighter>(shield, attackPower); 
}

shared_ptr<DeathStar> createDeathStar(const ShieldPoints &shield, const AttackPower &attackPower) { 
    return make_shared<DeathStar>(shield, attackPower);
}

shared_ptr<ImperialDestroyer> createImperialDestroyer(const ShieldPoints &shield, const AttackPower &attackPower) { 
    return make_shared<ImperialDestroyer>(shield, attackPower);
}

shared_ptr<Squadron> createSquadron(const vector<shared_ptr<ImperialStarship>> &vect) { 
    return make_shared<Squadron>(vect);
}

shared_ptr<Squadron> createSquadron(const initializer_list<shared_ptr<ImperialStarship>> &list) { 
    return make_shared<Squadron>(list); 
}

#endif /* __IMPERIALFLEET_H__ */


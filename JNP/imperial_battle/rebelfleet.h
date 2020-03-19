#ifndef __REBELFLEET_H__
#define __REBELFLEET_H__

#include "helper.h"

#include <cassert>
#include <type_traits>
#include <memory>

using namespace std;

class RebelStarship : virtual public Starship, virtual public AttackingStarship {
public:

    RebelStarship(const ShieldPoints &shield, const Speed &speed) {
        Starship::shield = shield;
        RebelStarship::speed = speed;
    }

    RebelStarship()
            : Starship(),
              speed() {}

    virtual bool isAttacker() {
        return false;
    }

    virtual Speed getSpeed() const { return speed; }

protected:

    Speed speed;
};

class AttackingRebelStarship : public RebelStarship {
public:

    AttackingRebelStarship(const ShieldPoints &shield, const Speed &speed, const AttackPower &attackPower) {
        Starship::shield = shield;
        RebelStarship::speed = speed;
        AttackingStarship::attackPower = attackPower;
    }

    bool isAttacker() override {
        return true;
    }
};

class Explorer : public RebelStarship {
public:
    Explorer(const ShieldPoints &shieldPoints, const Speed &speed)
            : RebelStarship(shieldPoints, speed) {
        assert(speed >= 299796 && speed <= 2997960);
    }
};

class StarCruiser : public AttackingRebelStarship {
public:

    StarCruiser(const ShieldPoints &shield, const Speed &speed, const AttackPower &attackPower)
            : AttackingRebelStarship(shield, speed, attackPower) {
        assert(speed >= 99999 && speed <= 299795);
    }
};

class XWing : public AttackingRebelStarship {
public:

    XWing(const ShieldPoints &shieldPoints, const Speed &speed, const AttackPower &attackPower)
            : AttackingRebelStarship(shieldPoints, speed, attackPower) {
        assert(speed >= 299796 && speed <= 2997960);
    }
};

using AttackPower = Starship::AttackPower;
using ShieldPoints = Starship::ShieldPoints;
using Speed = Starship::Speed;

shared_ptr<Explorer> createExplorer(const ShieldPoints &shieldPoints, const Speed &speed) { 
    return make_shared<Explorer>(shieldPoints, speed);
}

shared_ptr<StarCruiser> createStarCruiser(const ShieldPoints &shield, const Speed &speed, const AttackPower &attackPower) { 
    return make_shared<StarCruiser>(shield, speed, attackPower);
}

shared_ptr<XWing> createXWing(const ShieldPoints &shield, const Speed &speed, const AttackPower &attackPower) { 
    return make_shared<XWing>(shield, speed, attackPower);
}

#endif /* __REBELFLEET_H__ */


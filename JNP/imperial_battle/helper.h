#ifndef __HELPER_H__
#define __HELPER_H__

#include <type_traits>
#include <memory>

class Starship {
public:

    using AttackPower = int;
    using ShieldPoints = int;
    using Speed = int;

    Starship()
            : shield() {}

    explicit Starship(const ShieldPoints &shield)
            : shield(shield) {}


    virtual void takeDamage(const AttackPower &damage) {
        if (damage <= shield)
            shield -= damage;
        else
            shield = 0;
    }

    virtual ShieldPoints getShield() const {
        return shield;
    }


protected:

    ShieldPoints shield;
};

class AttackingStarship : virtual public Starship {
public:

    AttackingStarship(const ShieldPoints &shield, const AttackPower &attackPower)
            : Starship(shield),
              attackPower(attackPower) {}

    AttackingStarship()
            : Starship(),
              attackPower() {}


    virtual AttackPower getAttackPower() const {
        return attackPower;
    }

    virtual size_t countShips() {
        if (shield > 0) {
            return 1;
        } else {
            return 0;
        }
    }

protected:

    AttackPower attackPower;
};

class Timer {
public:
    using Time = unsigned int;

    Timer(Time time) {
        this->time = time;
        setMax = false;
    }

    Timer() : Timer(0) {}

    void setTime(Time time) {
        this->time = time;

        if (setMax) {
            this->time %= (maxTime+1);
        }
    }

    void setMaxTime(Time time) {
        setMax = true;
        maxTime = time;
        setTime(this->time);
    }

    Time getTime() {
        return time;
    }

    void tick(Time delay) {
        setTime(time+delay);
    }

private:
    Time time;
    Time maxTime;
    bool setMax;
};

#endif /* __HELPER_H__ */


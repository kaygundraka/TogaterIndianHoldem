#pragma once

#include <2d\CCActionInterval.h>

using namespace cocos2d;

class CCShake : public CCActionInterval
{
    // Code by Francois Guibert
    // Contact: www.frozax.com - http://twitter.com/frozax - www.facebook.com/frozax
public:
    CCShake();

    // Create the action with a time and a strength (same in x and y)
    static CCShake* actionWithDuration(float d, float strength);
    // Create the action with a time and strengths (different in x and y)
    static CCShake* actionWithDuration(float d, float strength_x, float strength_y);
    bool initWithDuration(float d, float strength_x, float strength_y);

    virtual void startWithTarget(CCNode *pTarget);
    virtual void update(float time);
    virtual void stop(void);

protected:
    // Initial position of the shaked node
    float _initial_x, _initial_y;
    // Strength of the action
    float _strength_x, _strength_y;
};
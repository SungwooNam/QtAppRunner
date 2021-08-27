#ifndef __FIXTURE_HPP__
#define __FIXTURE_HPP__

#include "AppRunner.hpp"

class Fixture
{
protected:
    Fixture()
    {
    }

    static Fixture* fixture_;

public:
    Fixture( Fixture& ) = delete;
    void operator=(const Fixture& ) = delete;

    static Fixture* instance()
    {
        if(fixture_==nullptr){
            fixture_ = new Fixture();
        }
        return fixture_;
    }

    AppRunner* appRunner() { return &App_; }

private:
    AppRunner App_;
};

#endif
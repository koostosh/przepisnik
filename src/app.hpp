
#pragma once

#include <memory>

class AppI
{
protected:
    AppI()
    {
    };
public:
    virtual ~AppI()
    {
    };
    virtual bool EventLoop() = 0;
    virtual bool BeginFrame() = 0;
    virtual bool EndFrame() = 0;

    static std::unique_ptr<AppI> Create();
};

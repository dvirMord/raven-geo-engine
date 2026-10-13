#pragma once
#include <functional>
#include "../Models/TelemetryFrame.hpp"

namespace Network 
{
    class ITelemetryReceiver 
    {
    public:
        using FrameCallback = std::function<void(const Models::TelemetryFrame&)>;

        virtual ~ITelemetryReceiver() = default;
        virtual void start(FrameCallback onFrameReceived) = 0;
        virtual void stop() = 0;
    };
}
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include "Common/Constants.hpp"
#include "Network/TcpSocketReceiver.hpp"
#include "Math/GeoCalculator.hpp"

int main() 
{
    spdlog::info(Constants::Logs::ENGINE_STARTING);

    auto calculator = std::make_shared<Math::GeoCalculator>();
    Network::TcpSocketReceiver receiver(Constants::Network::DEFAULT_HOST, Constants::Network::LTS_SOCKET_PORT);

    // Start background receiver with a simple callback
    receiver.start([&calculator](const Models::TelemetryFrame& frame) {
        calculator->calculateRay(frame);
    });

    spdlog::info(Constants::Logs::RUNNING_WAITING);
    std::cin.get();

    spdlog::info(Constants::Logs::ENGINE_STOPPING);
    receiver.stop();

    return 0;
}
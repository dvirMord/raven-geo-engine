#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <asio.hpp>

#include "ITelemetryReceiver.hpp"

namespace Network
{
    class TcpSocketReceiver : public ITelemetryReceiver
    {
    public:
        TcpSocketReceiver(std::string host, int port);
        ~TcpSocketReceiver() override;

        void start(FrameCallback onFrameReceived) override;
        void stop() override;

    private:
        void runWorkerLoop(FrameCallback onFrameReceived);
        bool connectToLts(asio::ip::tcp::socket& socket);
        void parseAndDispatchFrame(const std::string& jsonLine, const FrameCallback& callback);

        std::string _host;
        int _port;
        std::atomic<bool> _isRunning{ false };
        std::thread _workerThread;

        asio::io_context _ioContext;
        std::unique_ptr<asio::ip::tcp::socket> _socket;
    };
}
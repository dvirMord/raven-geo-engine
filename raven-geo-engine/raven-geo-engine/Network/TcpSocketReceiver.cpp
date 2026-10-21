#include "TcpSocketReceiver.hpp"
#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "../Common/Constants.hpp"

using json = nlohmann::json;
using asio::ip::tcp;

namespace Network
{
	TcpSocketReceiver::TcpSocketReceiver(std::string host, int port)
		: _host(std::move(host)), _port(port)
	{}

	TcpSocketReceiver::~TcpSocketReceiver() 
	{
		stop();
	}

	void TcpSocketReceiver::start(FrameCallback onFrameReceived)
	{
		if (this->_isRunning.load())
		{
			return;
		}
		this->_isRunning.store(true);
		this->_workerThread = std::thread(&TcpSocketReceiver::runWorkerLoop, this, onFrameReceived);
	}

	void TcpSocketReceiver::stop()
	{
		if (!this->_isRunning.load())
		{
			return;
		}

		this->_isRunning.store(false);

		if (this->_socket && this->_socket->is_open())
		{
			this->_socket->close();
		}

		if (_workerThread.joinable())
		{
			_workerThread.join();
		}
	}

	bool TcpSocketReceiver::connectToLts(tcp::socket& socket)
	{
		try
		{
			tcp::endpoint endpoint(asio::ip::make_address(this->_host), this->_port);
			
			socket.connect(endpoint);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
	
	void TcpSocketReceiver::runWorkerLoop(FrameCallback onFrameReceived)
	{
		while (this->_isRunning.load())
		{
			this->_socket = std::make_unique<tcp::socket>(this->_ioContext);
			spdlog::info(Constants::Logs::ATTEMPTING_CONNECT, _host, _port);
			
			if (!connectToLts(*_socket))
			{
				spdlog::warn(Constants::Logs::CONNECT_FAILED);
				std::this_thread::sleep_for(std::chrono::seconds(Constants::Network::RECONNECT_DELAY_SECONDS));
				continue;
			}
		
			if (!_isRunning.load())
			{
				break;
			}

			spdlog::info(Constants::Logs::CONNECTED_SUCCESS);

			asio::streambuf streamBuffer;
			while (_isRunning.load())
			{
				asio::error_code ec;

				size_t bytesRead = asio::read_until(*_socket, streamBuffer, Constants::Network::DELIMITER, ec);

				if (ec)
				{
					spdlog::warn(Constants::Logs::CONNECTION_LOST);
					break; 
				}
				std::istream is(&streamBuffer);
				std::string jsonLine;
				std::getline(is, jsonLine);

				if (!jsonLine.empty())
				{
					parseAndDispatchFrame(jsonLine, onFrameReceived);
				}
			}
			if (this->_socket && this->_socket->is_open())
			{
				asio::error_code ec;
				this->_socket->close(ec);
			}
		}
	}

	void TcpSocketReceiver::parseAndDispatchFrame(const std::string& jsonLine, const FrameCallback& callback)
	{
		try
		{
			auto j = json::parse(jsonLine);

			Models::TelemetryFrame frame{};
			frame.sensorLatitude = j.value(Constants::TelemetryJsonKeys::SENSOR_LATITUDE, 0.0);
			frame.sensorLongitude = j.value(Constants::TelemetryJsonKeys::SENSOR_LONGITUDE, 0.0);
			frame.sensorAltitudeMeters = j.value(Constants::TelemetryJsonKeys::SENSOR_ALTITUDE_METERS, 0.0);
			frame.platformHeadingDeg = j.value(Constants::TelemetryJsonKeys::PLATFORM_HEADING_DEG, 0.0);
			frame.platformPitchDeg = j.value(Constants::TelemetryJsonKeys::PLATFORM_PITCH_DEG, 0.0);
			frame.platformRollDeg = j.value(Constants::TelemetryJsonKeys::PLATFORM_ROLL_DEG, 0.0);
			frame.gimbalAzimuthDeg = j.value(Constants::TelemetryJsonKeys::GIMBAL_AZIMUTH_DEG, 0.0);
			frame.gimbalElevationDeg = j.value(Constants::TelemetryJsonKeys::GIMBAL_ELEVATION_DEG, 0.0);
			frame.gimbalRollDeg = j.value(Constants::TelemetryJsonKeys::GIMBAL_ROLL_DEG, 0.0);
			frame.frameCenterLatitude = j.value(Constants::TelemetryJsonKeys::FRAME_CENTER_LATITUDE, std::optional<double>{});
			frame.frameCenterLongitude = j.value(Constants::TelemetryJsonKeys::FRAME_CENTER_LONGITUDE, std::optional<double>{});
			frame.frameCenterElevation = j.value(Constants::TelemetryJsonKeys::FRAME_CENTER_ELEVATION, std::optional<double>{});

			callback(frame);
		}
		catch (const std::exception& ex)
		{
			spdlog::error(Constants::Logs::PARSE_ERROR, ex.what());
		}
	}
}
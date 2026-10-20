#pragma once

namespace Constants 
{
    namespace Network 
    {
        inline constexpr char DEFAULT_HOST[] = "127.0.0.1";
        inline constexpr int LTS_SOCKET_PORT = 18080;
        inline constexpr int RECONNECT_DELAY_SECONDS = 2;
        inline constexpr int RECONNECT_DELAY_MS = 2000;
        inline constexpr char DELIMITER = '\n';
    }

    namespace Logs 
    {
        inline constexpr char ENGINE_STARTING[] = "Starting raven-geo-engine...";
        inline constexpr char ENGINE_STOPPING[] = "Stopping raven-geo-engine...";
        inline constexpr char ATTEMPTING_CONNECT[] = "Connecting to LTS at {}:{}...";
        inline constexpr char CONNECTED_SUCCESS[] = "Connected to LTS successfully!";
        inline constexpr char CONNECT_FAILED[] = "Connection failed. Retrying in 2000 ms...";
        inline constexpr char CONNECTION_LOST[] = "Connection lost with LTS. Attempting to reconnect...";
        inline constexpr char PARSE_ERROR[] = "Failed to parse telemetry JSON: {}";
        inline constexpr char RUNNING_WAITING[] = "Running... Press ENTER to exit.";
        inline constexpr char CALC_RAY[] = "Creating ray for gimbalAzimuthDeg: {}";
    }

    namespace TelemetryJsonKeys 
    {
        inline constexpr char SENSOR_LATITUDE[] = "SensorLatitude";
        inline constexpr char SENSOR_LONGITUDE[] = "SensorLongitude";
        inline constexpr char SENSOR_ALTITUDE_METERS[] = "SensorAltitudeMeters";
        inline constexpr char PLATFORM_HEADING_DEG[] = "PlatformHeadingDeg";
        inline constexpr char PLATFORM_PITCH_DEG[] = "PlatformPitchDeg";
        inline constexpr char PLATFORM_ROLL_DEG[] = "PlatformRollDeg";
        inline constexpr char GIMBAL_AZIMUTH_DEG[] = "GimbalAzimuthDeg";
        inline constexpr char GIMBAL_ELEVATION_DEG[] = "GimbalElevationDeg";
        inline constexpr char GIMBAL_ROLL_DEG[] = "GimbalRollDeg";
    }

    namespace Math 
    {
        inline constexpr double SEMICIRCLE_DEGREES = 180.0;
    }
}

#pragma once
#include <cstdint>

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
        inline constexpr char CALC_RAY[] = "  drone    {:.6f}, {:.6f} | alt {:.1f} m | looking N {:+.3f} E {:+.3f} D {:+.3f}";
		inline constexpr char CALC_Z_POINT_WARNING[] = "The ray is pointing upwards / above the horizon (negative z direction): Ray: {}";
		inline constexpr char DRONE_BELOW_GROUND_WARNING[] = "The drone is below the ground level. Drone altitude: {:.2f} m, ground altitude: {:.2f} m";
		inline constexpr char CALC_GROUND_INTERSECTION_ERROR[] = "  HIT      {:.6f}, {:.6f} | ground {:.1f} m | range {:.0f} m | {:.1f} m from frame center";
		inline constexpr char CALC_GROUND_INTERSECTION[] = "  HIT      {:.6f}, {:.6f} | ground {:.1f} m | range {:.0f} m";
		inline constexpr char RAY_FIELDS_MISSING[] = "  SKIPPED  missing telemetry fields: [{}]";
		inline constexpr char FRAME_CENTER_LATITUDE_AND_LONGITUDE_MISSING[] = "  (no frame center in the telemetry - can't measure the error)";
		inline constexpr char START_CALC[] = "----- frame {} -----";
		inline constexpr char TILE_LOADED[] = "Loaded terrain tile: {}";
		inline constexpr char TILE_LOAD_FAILED[] = "Failed to load terrain tile: {}, skipped.";
		inline constexpr char TILES_LOADED_SUMMARY[] = "Loaded {} of {} terrain tiles.";
		inline constexpr char NO_TILES_LOADED[] = "No terrain tiles were loaded - every frame will have no ground height. Check the tile paths.";
		inline constexpr char TILE_PATH_ENV_MISSING[] = "Environment variable {} is not set - that tile is skipped.";
		inline constexpr char DOTENV_LOADED[] = "Loaded settings from {}";
		inline constexpr char DOTENV_NOT_FOUND[] = "No {} file in the working folder - using the system environment only.";
		inline constexpr char RAY_MARCH_STUCK[] = "  ray march stuck: cell exit {:.3f} m is not past the current distance {:.3f} m - giving up as a miss";
		inline constexpr char RAY_MARCH_MISS[] = "  MISS     the ray points up or stays above the terrain for {:.0f} m";
		inline constexpr char RAY_MARCH_NO_TERRAIN_DATA[] = "  NO DATA  the ray left the loaded DSM tiles before reaching the ground";
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
		inline constexpr char FRAME_CENTER_LATITUDE[] = "FrameCenterLatitude"; 
		inline constexpr char FRAME_CENTER_LONGITUDE[] = "FrameCenterLongitude";
		inline constexpr char FRAME_CENTER_ELEVATION[] = "FrameCenterElevation";
    }

    namespace Math 
    {
        inline constexpr double SEMICIRCLE_DEGREES = 180.0;
		inline constexpr double EARTH_RADIUS_M = 6371000.0;
    }

    namespace DotEnv
    {
        // read from the working folder - in Visual Studio that is the project folder, next to main.cpp
        inline constexpr char FILE_NAME[] = ".env";
    }

    namespace GeoTiff
    {
        // tag holding the size of one cell: [lon size, lat size, 0] in degrees
        inline constexpr uint32_t CELL_SIZE_TAG = 33550;
        // tag tying cell [0][0] to a place on earth: [col, row, 0, lon, lat, 0]
        inline constexpr uint32_t CORNER_TAG = 33922;

        // where each value sits inside the tag arrays
        inline constexpr int CELL_SIZE_LON_INDEX = 0;
        inline constexpr int CELL_SIZE_LAT_INDEX = 1;
        inline constexpr int CORNER_LON_INDEX = 3;
        inline constexpr int CORNER_LAT_INDEX = 4;

        // fewer values than this means the tag is broken - reading the indexes above would overflow
        inline constexpr uint32_t CELL_SIZE_MIN_COUNT = 2;
        inline constexpr uint32_t CORNER_MIN_COUNT = 5;

        // the heights are read straight into a float grid, so every value must be a 32 bit float
        inline constexpr uint16_t HEIGHT_BITS_PER_SAMPLE = 32;

        // one environment variable per DSM tile, each holding the full path of its .tif file
        //kept out of the code because the paths are different on every machine.
        inline constexpr const char* TILE_PATH_ENV_VARS[] =
        {
            "RAVEN_DSM_TILE_CHEYENNE",
            "RAVEN_DSM_TILE_FLORIDA",
            "RAVEN_DSM_TILE_NORTH_CAROLINA",
        };

        // cells per block - each block keeps its max height so a ray can skip it. 16 measured fastest on real flights (rays cross ~100-200 cells, best block is about the square root of that); 60 was 2x slower
        inline constexpr uint16_t BLOCK_SIZE = 16;
    }

    namespace RayMarching
    {
        // the ray gives up after this many meters - past it earth curvature (~8 m at 10 km) makes the flat model wrong
        inline constexpr double MAX_RAY_DISTANCE_METERS = 10000.0;

        // how far past a cell's exit to look up the next cell - a point exactly on the border can return the same cell again
        inline constexpr double NEXT_CELL_NUDGE_METERS = 0.01;
    }
}
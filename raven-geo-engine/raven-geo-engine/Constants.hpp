#pragma once
#include <cstdint>

namespace Constants {

    namespace Server {
        inline constexpr uint16_t DEFAULT_PORT = 18080;
    }

    namespace StatusCodes {
        inline constexpr int OK = 200;
        inline constexpr int BAD_REQUEST = 400;
    }

    namespace Routes {
        inline constexpr char HEALTH[] = "/health";
        inline constexpr char CALCULATE[] = "/api/v1/calculate";
    }

    namespace JsonKeys {
        inline constexpr char MESSAGE[] = "Message";
        inline constexpr char STATUS[] = "status";
        inline constexpr char DATA[] = "data";
        inline constexpr char LATITUDE[] = "latitude";
        inline constexpr char LONGITUDE[] = "longitude";
        inline constexpr char ALTITUDE[] = "altitude";
    }

    namespace Messages {
        inline constexpr char ENGINE_UP[] = "Engine API is UP!";
        inline constexpr char SUCCESS[] = "success";
    }

    namespace Errors {
        inline constexpr char INVALID_JSON[] = "Invalid JSON";
        inline constexpr char MISSING_FIELDS[] = "Missing required fields: latitude, longitude, altitude";
    }

    namespace GeoCalculation {
        inline constexpr char LAT_PREFIX[] = "Calculated intersection for Lat: ";
        inline constexpr char LON_PREFIX[] = ", Lon: ";
        inline constexpr char ALT_PREFIX[] = ", Alt: ";
    }

} // namespace Constants

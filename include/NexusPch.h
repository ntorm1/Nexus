#pragma once
#ifndef PCH_H
#define PCH_H
#define NOMINMAX 
#include <memory>
#include <vector>
#include <optional>

#include <json.hpp>
#include "AgisPointers.h"

using json = nlohmann::json;


#define NEXUS_INTERUPT(msg) \
    do { \
        std::ostringstream oss; \
        oss << "Error in " << __FILE__ \
            << " at line " << __LINE__ << ": " << msg; \
        QMessageBox::critical(nullptr, "Critical Error", QString::fromStdString(oss.str()), QMessageBox::Ok); \
        return; \
    } while (false)

#define NEXUS_TRY_OR_INTERUPT(function) \
    do { \
        auto res = function; \
        if (res.is_exception()) { \
            NEXUS_INTERUPT(res.get_exception()); \
        }\
    } while (false)

#endif
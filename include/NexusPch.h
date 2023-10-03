#pragma once
#ifndef PCH_H
#define PCH_H
#define NOMINMAX 
#define USE_LUAJIT
#include <memory>
#include <vector>
#include <optional>

#include "AgisPointers.h"

#include "Hydra.h"

#include <ankerl/unordered_dense.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>


using namespace rapidjson;

typedef const Hydra* HydraPtr;

#define NEXUS_INTERUPT(msg) \
    do { \
        std::ostringstream oss; \
        oss << "Error in " << __FILE__ \
            << " at line " << __LINE__ << ": " << msg; \
        QMessageBox::critical(nullptr, "Critical Error", QString::fromStdString(oss.str()), QMessageBox::Ok); \
        return; \
    } while (false)

#define NEXUS_DO_OR_INTERUPT(function) \
    do { \
        auto res = function; \
        if (res.is_exception()) { \
            NEXUS_INTERUPT(res.get_exception()); \
        }\
    } while (false)

#endif
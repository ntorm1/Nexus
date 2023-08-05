#pragma once


#define NEXUS_TRY(action) \
    try { \
        action; \
    } catch (const std::exception& e) { \
        QMessageBox::critical(nullptr, "Error", e.what());\
    }


#define NEXUS_THROW(msg)                            \
    do {                                                \
        std::ostringstream oss;                         \
        oss << "Error in " << __FILE__                  \
            << " at line " << __LINE__ << ": " << msg;  \
        throw std::runtime_error(oss.str());            \
    } while (false)


#ifndef VERSION_H
#define VERSION_H

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define BUILD_NUMBER 1

inline const char* getVersionString() {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    return version;
}

inline const char* getVersionStringWithBuild() {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d+%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, BUILD_NUMBER);
    return version;
}

#endif // VERSION_H

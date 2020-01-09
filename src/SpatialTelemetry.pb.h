/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.0-dev */

#ifndef PB_SPATIALTELEMETRY_PB_H_INCLUDED
#define PB_SPATIALTELEMETRY_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _SpatialTelemetry {
    int32_t deviceId;
    int32_t timestamp;
    float latitude;
    float longitude;
    int32_t gsmStrength;
/* @@protoc_insertion_point(struct:SpatialTelemetry) */
} SpatialTelemetry;


/* Initializer values for message structs */
#define SpatialTelemetry_init_default            {0, 0, 0, 0, 0}
#define SpatialTelemetry_init_zero               {0, 0, 0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define SpatialTelemetry_deviceId_tag            1
#define SpatialTelemetry_timestamp_tag           2
#define SpatialTelemetry_latitude_tag            3
#define SpatialTelemetry_longitude_tag           4
#define SpatialTelemetry_gsmStrength_tag         5

/* Struct field encoding specification for nanopb */
#define SpatialTelemetry_FIELDLIST(X, a) \
X(a, STATIC, REQUIRED, INT32, deviceId, 1) \
X(a, STATIC, REQUIRED, INT32, timestamp, 2) \
X(a, STATIC, REQUIRED, FLOAT, latitude, 3) \
X(a, STATIC, REQUIRED, FLOAT, longitude, 4) \
X(a, STATIC, REQUIRED, INT32, gsmStrength, 5)
#define SpatialTelemetry_CALLBACK NULL
#define SpatialTelemetry_DEFAULT NULL

extern const pb_msgdesc_t SpatialTelemetry_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define SpatialTelemetry_fields &SpatialTelemetry_msg

/* Maximum encoded size of messages (where known) */
#define SpatialTelemetry_size                    43

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif

#pragma once

typedef enum {
  ENTITY_WORLD = 0,
  ENTITY_ITEM,
  ENTITY_PLAYER,
  ENTITY_TYPE_COUNT
} EntityType;

typedef enum {
    CAMERA_FORWARD = 1,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
} CameraDirection;

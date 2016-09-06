#pragma once

#define PEEK_TEST

typedef enum {
  PEEK_TYPE_BOTTOM,
  PEEK_TYPE_TOP
} PeekType;

Layer* peek_test_init(GRect screen_bounds, int peek_interval, PeekType peek_type);
void peek_test_unobstructed_area_service_subscribe(UnobstructedAreaHandlers handlers, void *context);
void peek_test_unobstructed_area_service_unsubscribe();
GRect peek_test_get_unobstructed_bounds();

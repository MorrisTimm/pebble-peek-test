#pragma once
#include <pebble.h>

#define PEEK_TEST

Layer* peek_test_init(GRect screen_bounds, int peek_interval);
void peek_test_unobstructed_area_service_subscribe(UnobstructedAreaHandlers handlers, void *context);
void peek_test_unobstructed_area_service_unsubscribe();
GRect peek_test_get_unobstructed_bounds();

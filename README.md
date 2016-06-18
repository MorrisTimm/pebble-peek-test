# peek-test
A way to test Timeline Peek events in CloudPebble.

As I'm writing this there is no way to test Timeline Peek events in CloudPebble, so I wrote an emulator using the same callback handlers to emulate the behaviour of unobstructed area change events.

The resulting animation it not exactly the same as with real events but it is a pretty good approximation.

To use the peek test you have to add the `Layer` returned by `peek_test_init` to your root layer. This will be the layer that shows the peek. Instead of `layer_get_unobstructed_bounds` you have to use `peek_test_get_unobstructed_bounds`. The subscribe and unsubscribe functions juast have an added `peek_test_` in front.

You can use the `PEEKL_TEST` define to determine whether or not the peek test is used. The define is set if `peek_test.h` is included. When used correctly you can easily switch between emulation and real events by just commenting out the `peek_test.h` include.

Usage example:

    #include <peek_test.h>

    GSize available_screen;

    static void update_layers() {
      /* do something based on available_screen */
    }
      
    static void unobstructed_area_will_change(GRect final_unobstructed_screen_area, void *context) {
      APP_LOG(APP_LOG_LEVEL_INFO, "will change = %d/%d", final_unobstructed_screen_area.size.w, final_unobstructed_screen_area.size.h);
    }
      
    static void unobstructed_area_change(AnimationProgress progress, void* context) {
      APP_LOG(APP_LOG_LEVEL_INFO, "change = %d%%", (int)progress);
    #ifdef PEEK_TEST
      GRect unobstructed_area = peek_test_get_unobstructed_bounds();
    #else // PEEK_TEST
      GRect unobstructed_area = layer_get_unobstructed_bounds(window_get_root_layer(my_window));
    #endif // PEEK_TEST
      available_screen = unobstructed_area.size;
      update_layers();
    }
      
    static void unobstructed_area_did_change(void* context) {
      APP_LOG(APP_LOG_LEVEL_INFO, "did_change");
    }

    static void my_window_load(Window *window) {
      Layer* root_layer = window_get_root_layer(window);
      GRect root_layer_bounds = layer_get_bounds(root_layer);
      available_screen = layer_get_unobstructed_bounds(root_layer).size;
      
      UnobstructedAreaHandlers handlers = {
        .will_change = unobstructed_area_will_change,
        .change = unobstructed_area_change,
        .did_change = unobstructed_area_did_change
      };
    #ifdef PEEK_TEST
      static Layer* peek_layer;
      peek_layer = peek_test_init(root_layer_bounds, 5000);
      layer_add_child(root_layer, peek_layer);
      peek_test_unobstructed_area_service_subscribe(handlers, NULL);
    #else // PEEK_TEST
      unobstructed_area_service_subscribe(handlers, NULL);
    #endif // PEEK_TEST
      
      update_layers();
    }
      
    static void my_window_unload(Window *window) {
    #ifdef PEEK_TEST
      peek_test_unobstructed_area_service_unsubscribe();
    #else // PEEK_TEST
      unobstructed_area_service_unsubscribe();
    #endif // PEEK_TEST
    }
    

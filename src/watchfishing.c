#include <pebble.h>

static Window *main_window;

static TextLayer   *time_text_layer;
static BitmapLayer *goober_layer;
static BitmapLayer *timebox_layer;
static BitmapLayer *mouth_connected_layer;
static BitmapLayer *mouth_disconnected_layer;

static GBitmap *goober_bitmap;
static GBitmap *timebox_bitmap;
static GBitmap *mouth_connected_bitmap;
static GBitmap *mouth_disconnected_bitmap;

static GFont time_font;

static void update_time() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
    // remove leading zero if 12h format
    char *s_buffer_ptr = s_buffer;
    if (s_buffer[0] == '0' && !clock_is_24h_style()) {
        s_buffer_ptr++;
    }
    text_layer_set_text(time_text_layer, s_buffer_ptr);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) { update_time(); }

static void bluetooth_callback(bool connected) {
    if (connected) { // goober happy (:
        layer_set_hidden(bitmap_layer_get_layer(mouth_connected_layer), false);
        layer_set_hidden(bitmap_layer_get_layer(mouth_disconnected_layer), true);
    } else { // goober sad ):
        layer_set_hidden(bitmap_layer_get_layer(mouth_connected_layer), true);
        layer_set_hidden(bitmap_layer_get_layer(mouth_disconnected_layer), false);
    }
}

static void show_goober() {
    #if PBL_COLOR
        #if PBL_RECT
            #if PBL_DISPLAY_HEIGHT == 168
                goober_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GOOBER_COLOR);
            #endif
        #elif PBL_ROUND
            goober_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GOOBER_COLOR_ROUND);
        #endif
    #elif PBL_BW
        goober_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GOOBER_BW);
    #endif
    bitmap_layer_set_bitmap(goober_layer, goober_bitmap);
}

static void show_timebox_and_mouth() {
    #if PBL_COLOR
        mouth_connected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOUTH_CONNECTED);
        mouth_disconnected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOUTH_DISCONNECTED);
        timebox_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TIME_BOX);
    #elif PBL_BW
        mouth_connected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOUTH_CONNECTED_BW);
        mouth_disconnected_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOUTH_DISCONNECTED_BW);
	    timebox_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TIME_BOX_BW);
    #endif
    bitmap_layer_set_bitmap(timebox_layer, timebox_bitmap);
    bitmap_layer_set_bitmap(mouth_connected_layer, mouth_connected_bitmap);
    bitmap_layer_set_bitmap(mouth_disconnected_layer, mouth_disconnected_bitmap);
}

static void position_mouth_and_time(GRect bounds) {
    #if PBL_RECT
        timebox_layer = bitmap_layer_create(GRect(22, 10, 100, 48));
        mouth_connected_layer = bitmap_layer_create(GRect(45, 133, 51, 19));
        mouth_disconnected_layer = bitmap_layer_create(GRect(45, 133, 51, 19));
    #elif PBL_ROUND
        timebox_layer = bitmap_layer_create(GRect(40, 20, 100, 48));
        mouth_connected_layer = bitmap_layer_create(GRect(63, 139, 51, 19));
        mouth_disconnected_layer = bitmap_layer_create(GRect(63, 139, 51, 19));
    #endif
    time_text_layer = text_layer_create(GRect(1, PBL_IF_ROUND_ELSE(10, 0), bounds.size.w, 50));
}

static void goober_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    goober_layer = bitmap_layer_create(bounds);

    position_mouth_and_time(bounds);
 
    layer_add_child(window_layer, bitmap_layer_get_layer(goober_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(timebox_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(mouth_connected_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(mouth_disconnected_layer));
    
    // show the silly little guy
    show_goober();
    show_timebox_and_mouth();

    // show time text
    layer_add_child(window_layer, text_layer_get_layer(time_text_layer));
    text_layer_set_background_color(time_text_layer, GColorClear);
    text_layer_set_text_color(time_text_layer, GColorWhite);
    text_layer_set_text(time_text_layer, "4:20"); // haha funny number please laugh
    text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
    time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ACCIDENTAL_PRESIDENCY_50));
    text_layer_set_font(time_text_layer, time_font);
    
    bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void goober_unload(Window *window) {
    gbitmap_destroy(goober_bitmap);
    gbitmap_destroy(timebox_bitmap);
    gbitmap_destroy(mouth_connected_bitmap);
    gbitmap_destroy(mouth_disconnected_bitmap);
    
    bitmap_layer_destroy(goober_layer);
    bitmap_layer_destroy(timebox_layer);
    bitmap_layer_destroy(mouth_connected_layer);
    bitmap_layer_destroy(mouth_disconnected_layer);

    text_layer_destroy(time_text_layer);
    fonts_unload_custom_font(time_font);
}

static void init() {
    main_window = window_create();

    window_set_window_handlers(main_window, (WindowHandlers) {
        .load = goober_load,
        .unload = goober_unload
    });

    window_stack_push(main_window, true);

    update_time();

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    connection_service_subscribe((ConnectionHandlers){
        .pebble_app_connection_handler = bluetooth_callback});
}

static void deinit() { window_destroy(main_window); }

int main(void) {
    init();
    app_event_loop();
    deinit();
}

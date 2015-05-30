#include "window.h"
#include <pebble.h>

// number of updates per second
// must be < 1000 (but 10-20 should look fine)
#define ACCURACY 20

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_background;
static GFont s_res_gothic_24_bold;
static BitmapLayer *background;
static TextLayer *dateLayer;
static Layer *hourLayer;
static Layer *minuteLayer;
static Layer *secondLayer;
static Layer *pivotLayer;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, 1);
  #endif
  
  s_res_image_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  // background
  background = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background, s_res_image_background);
  layer_add_child(window_get_root_layer(s_window), (Layer *)background);
  
  // dateLayer
  dateLayer = text_layer_create(GRect(117, 68, 28, 24));
  text_layer_set_background_color(dateLayer, GColorClear);
  text_layer_set_text(dateLayer, "24");
  text_layer_set_text_alignment(dateLayer, GTextAlignmentCenter);
  text_layer_set_font(dateLayer, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)dateLayer);
  
  // hourLayer
  hourLayer = layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_get_root_layer(s_window), (Layer *)hourLayer);
  
  // minuteLayer
  minuteLayer = layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_get_root_layer(s_window), (Layer *)minuteLayer);
  
  // secondLayer
  secondLayer = layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_get_root_layer(s_window), (Layer *)secondLayer);
  
  // pivotLayer
  pivotLayer = layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_get_root_layer(s_window), (Layer *)pivotLayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  bitmap_layer_destroy(background);
  text_layer_destroy(dateLayer);
  layer_destroy(hourLayer);
  layer_destroy(minuteLayer);
  layer_destroy(secondLayer);
  layer_destroy(pivotLayer);
  gbitmap_destroy(s_res_image_background);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

const GPathInfo HOUR_HAND_PATH_POINTS = {
	5,
	(GPoint[]) {
		{-4,0},
		{4, 0},
		{4, -37},
		{0, -47},
		{-4, -37}
	}
};

const GPathInfo MINUTE_HAND_PATH_POINTS = {
	5,
	(GPoint[]) {
		{-4, 0},
		{4, 0},
		{4, -58},
		{0, -69},
		{-4, -58}
	}
};
const GPathInfo SECOND_HAND_PATH_POINTS = {
	4,
	(GPoint[]) {
		{-3, 15},
		{3, 15},
		{3, -73},
		{-3, -73}
	}
};
static GPath* hourHandPath;
static GPath* minuteHandPath;
static GPath* secondHandPath;

struct tm* getTime() {
	time_t* tmp = malloc(sizeof(*tmp));
	time(tmp);
	struct tm* ret = localtime(tmp);
	free(tmp); // THE STUPIDEST BUG EVER!
	return ret;
}

void updateGenericHand(GPath* path, GContext* ctx, float circleFrac) {
	gpath_rotate_to(path, TRIG_MAX_ANGLE * circleFrac);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_filled(ctx, path);
	gpath_draw_outline(ctx, path);
}

void updateHourHand(Layer* hourLayer, GContext* ctx) {
	struct tm* currentTime = getTime();
	
	updateGenericHand(hourHandPath, ctx, 
		(currentTime->tm_hour % 12) / 12.0 +
		currentTime->tm_min / 720.0
	);
}
void updateMinuteHand(Layer* minuteLayer, GContext* ctx) {
	struct tm* currentTime = getTime();
	
	updateGenericHand(minuteHandPath, ctx,
		currentTime->tm_min / 60.0 +
		currentTime->tm_sec / 3600.0
	);
}
void updateSecondHand(Layer* secondLayer, GContext* ctx) {
	struct tm* currentTime = getTime();
	
	updateGenericHand(secondHandPath, ctx,
		currentTime->tm_sec / 60.0 + 
		time_ms(NULL, NULL) / 60000.0
	);
}
void drawPivot(Layer* pivotLayer, GContext* ctx) {
	GPoint center = {
		.x = 72,
		.y = 84
	};
	
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorWhite);
	
	graphics_fill_circle(ctx, center, 4);
}


void show_window(void) {
  initialise_ui();
	
	GPoint center = {
		.x = 72,
		.y = 84
	};
	
	hourHandPath = gpath_create(&HOUR_HAND_PATH_POINTS);
	layer_set_update_proc(hourLayer, updateHourHand);
	gpath_move_to(hourHandPath, center);
	
	minuteHandPath = gpath_create(&MINUTE_HAND_PATH_POINTS);
	layer_set_update_proc(minuteLayer, updateMinuteHand);
	gpath_move_to(minuteHandPath, center);
	
	secondHandPath = gpath_create(&SECOND_HAND_PATH_POINTS);
	layer_set_update_proc(secondLayer, updateSecondHand);
	gpath_move_to(secondHandPath, center);
	
	layer_set_update_proc(pivotLayer, drawPivot);
	layer_mark_dirty(pivotLayer);
	
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_window(void) {
  window_stack_remove(s_window, true);
}

void handle_second_tick(struct tm * tick_time, TimeUnits units_changed)
{	
	layer_mark_dirty(minuteLayer);
	//layer_mark_dirty(secondLayer); // ticking, not sweeping
	
	for (int i=0; i<ACCURACY; i++)
		app_timer_register(1000 / ACCURACY * i, (void*)layer_mark_dirty, secondLayer);
}

void handle_init()
{
	show_window();
	
	struct tm * currTime = getTime();
	
	static char wubz[] = "00";
	strftime(wubz, sizeof(wubz), "%d", currTime);
	text_layer_set_text(dateLayer, wubz);
	
	layer_mark_dirty(hourLayer);
	layer_mark_dirty(minuteLayer);
	layer_mark_dirty(secondLayer);
	
	tick_timer_service_subscribe(SECOND_UNIT,handle_second_tick);
}
void handle_deinit()
{
	hide_window();
	
	gpath_destroy(hourHandPath);
	gpath_destroy(minuteHandPath);
	gpath_destroy(secondHandPath);
	
	tick_timer_service_unsubscribe();
}
int main(void)
{
	handle_init();
	app_event_loop();
	handle_deinit();
}
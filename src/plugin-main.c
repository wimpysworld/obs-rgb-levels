/*
RGB Levels Filter - modelled on color-key-filter.c in obs distribution
Copyright (C) <2020> petrifiedpenguin <ajscholl@gmail.com>
Copyright (C) <2023> Martin Wimpress <code@wimpress.io>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>
#include "plugin-support.h"

#define SETTINGS_RED_MIN "red_min"
#define SETTINGS_RED_MAX "red_max"
#define SETTINGS_GREEN_MIN "green_min"
#define SETTINGS_GREEN_MAX "green_max"
#define SETTINGS_BLUE_MIN "blue_min"
#define SETTINGS_BLUE_MAX "blue_max"

#define MAX(a, b) \
	(((uint32_t)(a)) > ((uint32_t)(b)) ? ((uint32_t)(a)) : ((uint32_t)(b)))

struct rgb_levels_filter_data {
	obs_source_t *context;
	gs_effect_t *effect;

	gs_eparam_t *min_param;
	gs_eparam_t *scale_param;

	struct vec3 rgb_min;
	struct vec3 rgb_scale;
};

static const char *rgb_levels_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("RGB levels");
}

static inline void rgb_levels_update(void *data, obs_data_t *settings)
{
	struct rgb_levels_filter_data *filter = data;

	uint32_t r_min = (uint32_t)obs_data_get_int(settings, SETTINGS_RED_MIN);
	uint32_t g_min =
		(uint32_t)obs_data_get_int(settings, SETTINGS_GREEN_MIN);
	uint32_t b_min =
		(uint32_t)obs_data_get_int(settings, SETTINGS_BLUE_MIN);
	uint32_t r_max =
		MAX(r_min + 1,
		    (uint32_t)obs_data_get_int(settings, SETTINGS_RED_MAX));
	uint32_t g_max =
		MAX(g_min + 1,
		    (uint32_t)obs_data_get_int(settings, SETTINGS_GREEN_MAX));
	uint32_t b_max =
		MAX(b_min + 1,
		    (uint32_t)obs_data_get_int(settings, SETTINGS_BLUE_MAX));

	vec3_set(&filter->rgb_min, r_min / 255.0f, g_min / 255.0f,
		 b_min / 255.0f);
	vec3_set(&filter->rgb_scale, 255.0f / (r_max - r_min),
		 255.0f / (g_max - g_min), 255.0f / (b_max - b_min));
}

static void rgb_levels_destroy(void *data)
{
	struct rgb_levels_filter_data *filter = data;

	if (filter->effect) {
		obs_enter_graphics();
		gs_effect_destroy(filter->effect);
		obs_leave_graphics();
	}
	bfree(data);
}

static void *rgb_levels_create(obs_data_t *settings, obs_source_t *context)
{
	struct rgb_levels_filter_data *filter =
		bzalloc(sizeof(struct rgb_levels_filter_data));
	char *effect_path = obs_module_file("rgb_levels.effect");
	filter->context = context;

	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	if (filter->effect) {
		filter->min_param =
			gs_effect_get_param_by_name(filter->effect, "rgb_min");
		filter->scale_param = gs_effect_get_param_by_name(
			filter->effect, "rgb_scale");
	}
	obs_leave_graphics();

	bfree(effect_path);
	if (!filter->effect) {
		rgb_levels_destroy(filter);
		return NULL;
	}
	rgb_levels_update(filter, settings);
	return filter;
}

static void rgb_levels_render(void *data, gs_effect_t *effect)
{
	struct rgb_levels_filter_data *filter = data;

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
					     OBS_ALLOW_DIRECT_RENDERING))
		return;

	gs_effect_set_vec3(filter->min_param, &filter->rgb_min);
	gs_effect_set_vec3(filter->scale_param, &filter->rgb_scale);

	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

	UNUSED_PARAMETER(effect);
}

static obs_properties_t *rgb_levels_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_int_slider(props, "red_min", "Red min", 0, 254, 1);
	obs_properties_add_int_slider(props, "red_max", "Red max", 1, 255, 1);
	obs_properties_add_int_slider(props, "green_min", "Green min", 0, 254,
				      1);
	obs_properties_add_int_slider(props, "green_max", "Green max", 1, 255,
				      1);
	obs_properties_add_int_slider(props, "blue_min", "Blue min", 0, 254, 1);
	obs_properties_add_int_slider(props, "blue_max", "Blue max", 1, 255, 1);

	UNUSED_PARAMETER(data);
	return props;
}

static void rgb_levels_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, SETTINGS_RED_MIN, 0);
	obs_data_set_default_int(settings, SETTINGS_GREEN_MIN, 0);
	obs_data_set_default_int(settings, SETTINGS_BLUE_MIN, 0);
	obs_data_set_default_int(settings, SETTINGS_RED_MAX, 255);
	obs_data_set_default_int(settings, SETTINGS_GREEN_MAX, 255);
	obs_data_set_default_int(settings, SETTINGS_BLUE_MAX, 255);
}

struct obs_source_info rgb_levels_filter = {
	.id = "rgb_levels_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = rgb_levels_name,
	.create = rgb_levels_create,
	.destroy = rgb_levels_destroy,
	.video_render = rgb_levels_render,
	.update = rgb_levels_update,
	.get_properties = rgb_levels_properties,
	.get_defaults = rgb_levels_defaults};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_register_source(&rgb_levels_filter);
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}

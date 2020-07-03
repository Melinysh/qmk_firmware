RGB_MATRIX_EFFECT(raindrops_effect)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static HSV hsv_cache[DRIVER_LED_TOTAL];

static HSV custom_SOLID_REACTIVE_math(HSV hsv, uint16_t offset) {
    hsv.s = scale8(250 - offset, hsv.s);
    return hsv;
}

// Modification: Only change the value of the keys that have last been hit.
bool custom_effect_runner_reactive(effect_params_t* params, reactive_f effect_func) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint16_t max_tick = 65535 / rgb_matrix_config.speed;
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint16_t tick = max_tick;

        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == i && g_last_hit_tracker.tick[j] < tick) {
                uint8_t i = g_last_hit_tracker.index[j];
                tick = g_last_hit_tracker.tick[j];
                uint16_t offset = scale16by8(tick, rgb_matrix_config.speed);
                RGB rgb;
                if (offset <= 250) {
                    rgb = hsv_to_rgb(effect_func(rgb_matrix_config.hsv, offset));
                } else { // reset to previous hsv
                    rgb = hsv_to_rgb(hsv_cache[i]);
                }
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                break;
            }
        }
    }
    return led_max < DRIVER_LED_TOTAL;
}

static HSV custom_raindrops_set_color(int i, effect_params_t* params) {
    if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) return hsv_cache[i];
    HSV hsv = {0, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v};

    // Take the shortest path between hues
    int16_t deltaH = ((rgb_matrix_config.hsv.h + 180) % 360 - rgb_matrix_config.hsv.h) / 4;
    if (deltaH > 127) {
        deltaH -= 256;
    } else if (deltaH < -127) {
        deltaH += 256;
    }

    hsv.h   = rgb_matrix_config.hsv.h + (deltaH * (rand() & 0x03));
    RGB rgb = hsv_to_rgb(hsv);
    rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);

    return hsv;
}

static bool raindrops_effect(effect_params_t* params) {
    if (!params->init) {
        // Change one LED 
        if (rand() % 75 /*scale16by8(g_rgb_counters.tick, qadd8(rgb_matrix_config.speed, 16)) % 10*/ == 0) {
            uint8_t led = rand() % DRIVER_LED_TOTAL;
            HSV updatedHSV = custom_raindrops_set_color(led, params);
            hsv_cache[led] = updatedHSV;
        }
        return custom_effect_runner_reactive(params, &custom_SOLID_REACTIVE_math); 
    }

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (int i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        HSV appliedHSV = custom_raindrops_set_color(i, params);
        hsv_cache[i] = appliedHSV;
    }
    return led_max < DRIVER_LED_TOTAL;
}

#endif  // RGB_MATRIX_CUSTOM_EFFECT_IMPLS

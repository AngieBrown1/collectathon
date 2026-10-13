#include <algorithm>

#include <bn_core.h>
#include <bn_display.h>
#include <bn_log.h>
#include <bn_keypad.h>
#include <bn_random.h>
#include <bn_rect.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_size.h>
#include <bn_string.h>
#include <bn_colors.h>
#include <bn_sprite_palette_ptr.h>
#include <bn_backdrop.h>

#include "bn_sprite_items_dot.h"
#include "bn_sprite_items_square.h"
#include "bn_sprite_items_cat.h"
#include "bn_sprite_items_mouse.h"
#include "bn_sprite_items_fish.h"
#include "common_fixed_8x16_font.h"

// Pixels / Frame player moves at
static constexpr bn::fixed SPEED = 2;
static constexpr bn::fixed BOOST_SPEED = 4;
//the speed and stregnth of the jump and gravity to move the cat back down
bn:: fixed jump_speed = 0;
bn::fixed jump_strength = 4;
bn::fixed gravity = 0.2;

// Width and height of the the player and treasure bounding boxes
static constexpr int PLAYER_Y_OFFSET = 8;
static constexpr bn::size PLAYER_SIZE = {8, 8};
static constexpr bn::size TREASURE_SIZE = {8, 8};

// Full bounds of the screen
static constexpr int MIN_Y = -bn::display::height() / 2;
static constexpr int MAX_Y = bn::display::height() / 2;
static constexpr int MIN_X = -bn::display::width() / 2;
static constexpr int MAX_X = bn::display::width() / 2;

// Setting the player start location & treasure start location
static constexpr int PLAYER_START_X = -10;
static constexpr int PLAYER_START_Y = 50;
static constexpr int TREASURE_START_X = -100;
static constexpr int TREASURE_START_Y = 10;

// Number of characters required to show the longest numer possible in an int (-2147483647)
static constexpr int MAX_SCORE_CHARS = 11;
static constexpr int MAX_TEXT_LENGTH = 6; 

// Score location
static constexpr int SCORE_X = 70;
static constexpr int SCORE_Y = -70;

// Boost Location
static constexpr int BOOST_X = 0; 
static constexpr int BOOST_Y = 70; 


struct Player {
    bn::sprite_ptr sprite; 
    int boost_count = 3; 
    int boost_duration = 0; 
    bool boosted = false; 

    Player(int start_x, int start_y) : sprite(bn::sprite_items::cat.create_sprite(start_x, start_y)) {}

    // Boost update per frame
    void boost_update() {
        if (boosted) {
            boost_duration--; 
            if (boost_duration <= 0) {
                boosted = false; 
            }
        }
    }

    // Boost Logic
    void activate_boost() {
        if (boost_count > 0 && !boosted) {
            boosted = true; 
            boost_duration = 180; 
            boost_count--; 
        }
    }

    // Player controls
    void player_controls() {
        bn::fixed current_speed = boosted ? BOOST_SPEED : SPEED; 

        if (bn::keypad::left_held()) sprite.set_x(sprite.x() - current_speed);
        if (bn::keypad::right_held()) sprite.set_x(sprite.x() + current_speed);
        if (bn::keypad::up_held()) sprite.set_y(sprite.y() - current_speed);
        if (bn::keypad::down_held()) sprite.set_y(sprite.y() + current_speed);
        
        
    }

    // Logic for player screen wrap
    void player_screen_wrap(int min_x, int min_y, int max_x, int max_y) {
        if (sprite.x() < min_x) sprite.set_x(max_x); 
        if (sprite.y() < min_y) sprite.set_y(max_y); 
        if (sprite.x() > max_x) sprite.set_x(min_x); 
        if (sprite.y() > max_y) sprite.set_y(min_y);
    }

    bn::rect get_hitbox(int y_offset, int width, int height) const {
        return bn::rect(
            sprite.x().round_integer(), 
            sprite.y().round_integer() + y_offset, 
            width, 
            height
        );
    }

    void reset(int start_x, int start_y) {
        sprite.set_x(start_x); 
        sprite.set_y(start_y); 
        boost_count = 3; 
        boosted = false; 
        boost_duration = 0; 
    }
};

int main()
{
    bn::core::init();

    bn::random rng = bn::random();

    bn::backdrop::set_color(bn::color(20, 15, 31));

    // Will hold the sprites for the score
    bn::vector<bn::sprite_ptr, MAX_SCORE_CHARS> score_sprites = {};
    bn::vector<bn::sprite_ptr, MAX_TEXT_LENGTH> boost_sprites = {};
    bn::sprite_text_generator score_text_generator(common::fixed_8x16_sprite_font);
    bn::sprite_text_generator boost_text_generator(common::fixed_8x16_sprite_font); 
    boost_text_generator.set_center_alignment(); 

    int score{0};
    float blink_speed{0.1f};  
    float blink_phase{0.0f};

    Player player(PLAYER_START_X, PLAYER_START_Y);
    bn::sprite_ptr treasure = bn::sprite_items::mouse.create_sprite(TREASURE_START_X, TREASURE_START_Y);

    while (true)
    {   
        //add jump for the cat
    

        if (bn::keypad::a_pressed()) {
            player.activate_boost(); 
        }
        player.boost_update(); 
        player.player_controls(); 
        player.player_screen_wrap(MIN_X, MIN_Y, MAX_X, MAX_Y);

        if (bn::keypad::start_pressed())
        {
            player.reset(PLAYER_START_X, PLAYER_START_Y);
            treasure.set_position(TREASURE_START_X, TREASURE_START_Y); 
            score = 0;
        }

        // The bounding boxes of the player and treasure, snapped to integer pixels
        bn::rect player_rect = player.get_hitbox(PLAYER_Y_OFFSET, PLAYER_SIZE.width(), PLAYER_SIZE.height());
        bn::rect treasure_rect = bn::rect(treasure.x().round_integer(),
                                          treasure.y().round_integer(),
                                          TREASURE_SIZE.width(),
                                          TREASURE_SIZE.height());

        // If the bounding boxes overlap, set the treasure to a new location an increase score
        if (player_rect.intersects(treasure_rect))
        {
            // Jump to any random point in the screen
            int new_x = rng.get_int(MIN_X, MAX_X);
            int new_y = rng.get_int(MIN_Y, MAX_Y);
            treasure.set_position(new_x, new_y);

            score++;
        }

        boost_sprites.clear();
        bn::string<16> boost_string = player.boosted ? "BOOSTED" : "";
        if (player.boosted) {
            boost_text_generator.generate(BOOST_X, BOOST_Y, boost_string, boost_sprites);
            bn::sprite_palette_ptr boost_text_palette = boost_sprites[0].palette(); 

            if (blink_speed < 1.0f) {
                blink_speed += 0.01f; 
            }
            
            // Add blink speed to blink phase, which increases past the threshold, and then resets
            blink_phase += blink_speed; 

            // Cyclic reset based on pi
            if (blink_phase > 6.28f) {
                blink_phase = -6.28f;
            }

            if (blink_phase < 3.14f) {
                boost_text_palette.set_color(1, bn::colors::gray); 
            } else {
                boost_text_palette.set_color(1, bn::colors::white); 
            }

        } else {
            blink_speed = 0.1f; 
            blink_phase = 0.0f; 
        }

        // Update score display
        bn::string<MAX_SCORE_CHARS> score_string = bn::to_string<MAX_SCORE_CHARS>(score);
        score_sprites.clear();
        score_text_generator.generate(SCORE_X, SCORE_Y,
                                score_string,
                                score_sprites);

        // Update RNG seed every frame so we don't get the same sequence of positions every time
        rng.update();
        
        //cat jumps when b is pressed 
        if(bn::keypad::b_pressed()){
            jump_speed -= jump_strength;
        }
        jump_speed += gravity;

        player.sprite.set_y(player.sprite.y() + jump_speed);

        if(player.sprite.y() >= PLAYER_START_Y){
            player.sprite.set_y(PLAYER_START_Y);
            jump_speed = 0;
        }

        bn::core::update();
    }
}

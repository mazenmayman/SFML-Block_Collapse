
//
// Disclamer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resource, use the helper
// method resourcePath() from ResourcePath.hpp
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

// Here is a small helper for you ! Have a look.
#include "ResourcePath.hpp"

#include <iostream>
#include <iomanip>
#include <random>
#include <time.h>

using namespace sf;
using namespace std;

#define rows   5
#define cols    5
#define tile_size   100
#define space_size  (0.1*tile_size)

#define screen_width    ((cols + 2) * tile_size)
#define screen_height   ((rows + 2) * tile_size)

#define game_width  (cols * tile_size)
#define game_height (rows * tile_size)

#define game_xpos   (tile_size)
#define game_ypos   (tile_size)

#define square_size (tile_size - space_size)


enum Game_Tile { empty, red, green, blue, yellow };

// declare the game_array and the game rectangles
Game_Tile game_array[rows][cols], game_array_copy[rows][cols];
RectangleShape game_rectangles[rows][cols];

void intialize_restart( Text &result_text, bool &game_end);
void update_game(Vector2i mouse_pos);
void gravitate(int row, const int col, const int last_row);
void move(bool once = false);
void get_game_array_copy();
bool game_finished();
bool checker(int row, int col, Game_Tile removed_color, bool remove, int &count, bool finish_check);

void set_tile_color(Game_Tile game_tile, RectangleShape &rect){
    switch (game_tile) {
        case red:
            rect.setFillColor(Color::Red);
            break;
        case green:
            rect.setFillColor(Color::Green);
            break;
        case blue:
            rect.setFillColor(Color::Blue);
            break;
        case yellow:
            rect.setFillColor(Color::Yellow);
            break;
        default:
            rect.setFillColor(Color::Black);
    }
}

int main()
{
    
    srand (time(NULL));
    // Window Creation
    RenderWindow window(VideoMode(screen_width, screen_height), "Block Collapse");
    
    // Rectangle Shape Creation of the border
    RectangleShape game_rect;
    game_rect.setSize(Vector2f(game_width, game_height));
    game_rect.setFillColor(Color::Blue);
    game_rect.setPosition(game_xpos, game_ypos);
    
    // generate random colors and put rectangles in positions.
    float posx = game_xpos + space_size/2, posy = game_ypos + space_size/2;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Game_Tile random_tile = static_cast<Game_Tile>(rand() % 4 + 1);
            game_array[i][j] = random_tile;
            game_rectangles[i][j].setPosition(posx, posy);
            set_tile_color(random_tile, game_rectangles[i][j]);
            game_rectangles[i][j].setSize(Vector2f(square_size, square_size));
            
            posx += tile_size;
        }
        posx = game_xpos + space_size/2;
        posy += tile_size;
    }
    
    
      /*** Texts ***/
    // get the font
    sf::Font font;
    if (!font.loadFromFile(resourcePath() + "sansation.ttf")) {
        cout << "Failed to load sansation" << endl;
        return EXIT_FAILURE;
    }
    // declare the curr_player_text
    Text curr_player_text;
    curr_player_text.setFont(font);
    curr_player_text.setCharacterSize(tile_size/4);
    curr_player_text.setPosition(tile_size*0.1,tile_size*0.1);
    
    // dcelare the result_text
    Text result_text;
    result_text.setFont(font);
    result_text.setCharacterSize(tile_size/4);
    result_text.setPosition(tile_size+space_size/2, screen_height-tile_size+space_size/2);
    
    /*** game_end ***/
    // declare the game_end flag
    bool game_end;
    
    
    while (window.isOpen())
    {
        // flag to check if board has changed
        bool changed = false;
        
        //processing the events and user inputs
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            
            if(game_end){
                if (event.type == Event::KeyPressed)
                {
                    if (event.key.code == Keyboard::Escape)
                        window.close();
                    
                    if (event.key.code == Keyboard::Space){
                        intialize_restart(result_text, game_end);
                    }
                    
                }
            }
            
            else{   // keep playing
                if(event.type == Event::MouseButtonPressed){
                    if(event.mouseButton.button == Mouse::Left){
                        Vector2i mouse_pos = Vector2i (event.mouseButton.x, event.mouseButton.y);
                        update_game(mouse_pos);
                        game_end = game_finished();
                        if(game_end){
                            result_text.setString("Game Over! Press Space to restart");
                        }
                    }
                }
            }

        }
        
        // drawing
        window.clear(Color::Black);
        for (int i = 0; i < rows;i++ )
            for (int j = 0; j < cols; j++)
                window.draw(game_rectangles[i][j]);
        
        window.draw(curr_player_text);
        window.draw(result_text);
        window.display();
    }
}


void intialize_restart(Text &result_text, bool &game_end){
    // reset the game_rectangles & game_array
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Game_Tile random_tile = static_cast<Game_Tile>(rand() % 4 + 1);
            game_array[i][j] = random_tile;
            set_tile_color(random_tile, game_rectangles[i][j]);
        }
    }
    
    // reset the result_text
    result_text.setColor(Color::White);
    result_text.setString("");
    
    game_end = false;
}

void update_game(Vector2i mouse_pos){
    int row = (mouse_pos.y / tile_size) - 1;
    int col = (mouse_pos.x / tile_size) - 1;
    
    // remove one tile
    if(row >= 0 && row < rows && col >= 0 && col < cols){
        Game_Tile removed_color = game_array[row][col];
        int count = 0;
        checker(row, col, removed_color, false, count, false);
        move();
    }
}

void gravitate(int row, const int col, const int last_row) {
    for( ; row > last_row; --row ) {
        game_array[row][col] = game_array[row - 1][col];
        set_tile_color(game_array[row-1][col], game_rectangles[row][col]);
    }
    game_array[last_row][col] = empty;
    set_tile_color(empty, game_rectangles[last_row][col]);
    
}

void move(bool once){

    for(int j = 0; j < cols; j++){
        
        int i = 0;
        // skip all high empty spaces
        while ( i < rows && game_array[i][j] == empty )
            ++i;
        
        int last_row = i;
        
        // start pulling
        for ( ; i < rows; ++i )
            if ( game_array[i][j] == empty ) {
                gravitate(i, j, last_row );
                if ( once ) break;
            }
    }
}

void get_game_array_copy(){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            game_array_copy[i][j] = game_array[i][j];
        }
    }
}

bool game_finished(){
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            int count = 0;
            get_game_array_copy();
            Game_Tile removed_color = game_array[i][j];
            if(checker(i, j, removed_color, false, count, true)){
                return false;
            }
        }
    }
    return true;
}


bool checker(int row, int col, Game_Tile removed_color, bool remove, int &count, bool finish_check){
    if(row < 0 || row > rows-1 || col < 0 || col > cols-1 || removed_color == empty){
        // outside the game
        return 0;
    }
    
    // else, we are inside
    if(finish_check){
        if(game_array_copy[row][col] != removed_color){
            // the game tile is not the same color
            return false;
        }
    }
    else{
        if(game_array[row][col] != removed_color){
            // the game tile is not the same color
            return false;
        }
    }
    
    // Then the curr_tile is is the same color
    if(remove){
        // if it's a recursive call, then empty the curr_cell
        if(finish_check){
            game_array_copy[row][col] = empty;
        }
        else{
            set_tile_color(empty, game_rectangles[row][col]);
            game_array[row][col] = empty;
        }
        count++;
    } // and
    
    // start recurse
    while(checker(row-1, col, removed_color, true, count, finish_check));  // up
    while(checker(row, col+1, removed_color, true, count, finish_check));  // right
    while(checker(row, col-1, removed_color, true, count, finish_check));  // left
    while(checker(row+1, col, removed_color, true, count, finish_check));  // down
    
    if(count){  // empty the original cell
        if(finish_check){
            game_array_copy[row][col] = empty;
        }
        else{
            set_tile_color(empty, game_rectangles[row][col]);
            game_array[row][col] = empty;
        }
        return true;
    }
    return false;
}

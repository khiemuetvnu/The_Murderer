#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include<algorithm>
#include<SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include<vector>

struct bullet {
    int damage = 10;
    int speed = 20;
    SDL_Texture* texture = nullptr;
    SDL_Rect srcRect, dstRect;
    float posX = 0.0f, posY = 0.0f;
    double bullet_angle = 0;
    bool isdead = false;
    bullet(SDL_Renderer* renderer,int damage, int speed, const char* path, int x, int y, int w, int h, double angle);
    void destroy();
};

struct melee {
    int weapon_type = 0;
    Mix_Chunk* attack_sound;
    SDL_Rect hitbox = {0, 0, 0, 0};
    int damage = 0;
    int attack_duration = 200;
    int cool_down_duration = 400;
    int range = 0;
    melee() = default;
    melee(int width, int height, int damage,int range,int attack_duration, int cool_down_duration,const char* attack_soundd,int weapon_type);
};

struct complex_texture
{
    SDL_Texture* texture = nullptr;
    int numofframe = 1;
};


struct GameObject{
std::vector<complex_texture> textures;
int texture_index = 0;
SDL_Texture* texture;
SDL_Rect srcRect,dstRect;
float posX = 0.0f, posY = 0.0f;
float velX = 0.0f, velY = 0.0f;
int objectspeed;
int objectspeed2;
float objecthp;
float max_hp;
double objectangle;
int object_type = 0;

//for animations and actions handle;
bool isdead = false;
bool go_through_able = false;//for intact object only;

bool isrunning = false;
int numofframe;
int currentframe = 0;
int lastframetime = SDL_GetTicks();
int currentframetime = lastframetime;
Uint16 walking_duration;

melee shortrange_weapon;
Uint32 lastattacktime = 0;
bool isreadyforattack = true;
bool isattacking = false;

Mix_Chunk* dead_sound;
bool played_dead_sound = false;
bool played_attack_sound = false;




void anothertexture();
void emplace_backtexture(SDL_Renderer* renderer, const char* texturePath, int numofframe);



//no touching here;

GameObject(SDL_Renderer* renderer,const char* texturePath,const char* d_sound,int x,int y,int w,int h,int objectspeed,float objecthp,double objectangle,int numofframe,Uint16 walking_duration,int object_type);
~GameObject();
void setSourceRect(int x,int y,int w,int h);
void setDestinationRect(int x,int y,int w,int h);
void betterrender(double angle,SDL_Renderer* renderer);
void check_if_died();
void move(float dx,float dy);

void meleeattack();
void calculate_melee_hitbox();
};


bool checkcollision(const SDL_Rect& A,const SDL_Rect& B);

void follow(GameObject& follower,GameObject& target);
void escape1(GameObject& escaper,GameObject& follower,int bgwidth,int bgheight);
void escape2(SDL_Rect &a,SDL_Rect &b,int vol);
void smartEscape(SDL_Rect &a,const SDL_Rect &b,int vol);
int clamp(int value, int min_value, int max_value);
Uint32 getPixel(SDL_Surface* surface, int x, int y);
bool checkPerPixelCollision(SDL_Surface* spriteA, SDL_Rect rectA,SDL_Surface* spriteB, SDL_Rect rectB);


void updateCamera(SDL_Rect &camera,GameObject &player,int SCREEN_WIDTH,int SCREEN_HEIGHT,int WORLD_WIDTH,int WORLD_HEIGHT);
void rendercopytocamera(SDL_Renderer* renderer,SDL_Rect camera,std::vector<GameObject*>& gameobjects);
void rendercopytocamera_for_alive(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects);
void rendercopytocamera_for_dead(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects);
void rendercopytocamera_for_bullet(SDL_Renderer* renderer, SDL_Rect camera, std::vector<bullet*>& bullets);
void Initializer(SDL_Window* &window,SDL_Renderer* &renderer,int SCREEN_WIDTH,int SCREEN_HEIGHT);
void EndEverything(SDL_Window* &window,SDL_Renderer* &renderer);




void check_if_moved(GameObject& moving_things, SDL_Rect& temprect);
void check_attack_and_calculate_damage(const GameObject& attacker,std::vector<GameObject*>& Other_object, float dt);
void check_attack_and_calculate_damage_for_a_and_b(const GameObject& attacker,GameObject& victim, float dt);
bool check_if_collided(const GameObject& moving_things,const std::vector<GameObject*>& Other_object);
bool check_if_collided_plus_physic_1(GameObject& moving_things,const std::vector<GameObject*>& Other_object, float dt);
void greater_check_if_died(std::vector<GameObject*> &objects, GameObject& player);
void ObjectKiller(std::vector<GameObject*> &objects);

struct cell {
    int x, y;
    int cost;
    SDL_Point flowVector;

    cell(); // Constructor
};
bool isFull(std::vector<std::vector<cell>>& matrix,int default_value);
void computeCostField(std::vector<std::vector<cell>>& grid);
void drawVector(std::vector<std::vector<cell>>& arr);
void initialcostfield(std::vector<std::vector<cell>>& grid,const std::vector<GameObject*>other_objects,int cellsize);
bool isValid(int x, int y, int sizex, int sizey, const std::vector<std::vector<cell>>& arr);
void drawbetterVector(std::vector<std::vector<cell>>& arr);
void follow_the_vector(GameObject& npc,const std::vector<std::vector<cell>>& grid, int cellsize, float dt);

void spawn_enemies(std::vector<GameObject*>& moving_things, SDL_Renderer* renderer, int method, GameObject& player);
int count_dead_things(const std::vector<GameObject*>& moving_things);

void handleMovement(GameObject& object, int speed, float dt);
void angle_a_to_b(GameObject &a,GameObject &b);
void angle_otom(GameObject &player,const SDL_Rect &camera);
void angle_to_player(SDL_Rect& tempdstRect,GameObject& npc,GameObject& player);
void angle_otomovement(SDL_Rect& tempdstRect,GameObject& npc);
void physics_1(GameObject& a, GameObject& b, float dt);
void random_movement(GameObject& a, float dt);
//the ultimate love for you;
void the_ultimate_status_handler_for_things(std::vector<GameObject*>other_objects,std::vector<GameObject*>npcs,GameObject& player, float dt);
void the_ultimate_movement_and_status_handler(SDL_Renderer* renderer,std::vector<GameObject*>npcs,const std::vector<std::vector<cell>>& map_grid,int cellsize,std::vector<GameObject*>other_objects,GameObject& player,std::vector<bullet*>& bullets, float dt);
void the_ultimate_movement_and_status_handler_for_player(SDL_Renderer* renderer,GameObject& player,const SDL_Rect& camera,std::vector<GameObject*>other_objects,std::vector<GameObject*> npcs,std::vector<bullet*>& bullets, float dt);
void the_ultimate_animation_handler(GameObject& moving_things,int frame_duration_for_walking_texture);
void the_more_ultimate_animation_handler(std::vector<GameObject*>moving_things);
void the_ultimate_sound_effects_handler(std::vector<GameObject*>moving_things);
void the_ultimate_sound_effects_handler_for_single_object(GameObject& moving_thing);
void handle_movement_of_bullet(std::vector<bullet*>& bullets, float dt);
void handle_collision_of_bullet(std::vector<bullet*>& bullets,std::vector<GameObject*> things, bool is_player = false);
void handle_status_and_delete_dead_bullets(std::vector<bullet*>& bullets,int map_x,int map_y);


bool checkClick(SDL_Rect button, int x, int y);
void game();

extern bool g_SoundOn;
extern int g_TargetFPS;

extern bool g_CheatSpeedBoost;
extern bool g_CheatInvincible;
extern bool g_CheatMultiShot;
extern bool g_CheatAura;
extern bool g_CheatAutoDodge;
extern bool g_CheatSummonMinions;
extern float g_TimeScale;

struct AfterImage {
    SDL_Texture* tex;
    SDL_Rect src, dst;
    double angle;
    float alpha;
};
extern bool g_IsDodging;
extern float g_DodgeTimeLeft;
extern std::vector<AfterImage> g_AfterImages;
extern float g_AfterImageSpawnTimer;

extern float g_DodgeVelocityX;
extern float g_DodgeVelocityY;
extern bool g_PlayerIsDriving;
extern GameObject* g_CurrentCar;
extern float g_CarSpeed;
extern float g_CarAngle;

struct GojoOrb {
    int type; // 0=Blue, 1=Red, 2=Purple
    float posX, posY;
    float dirX, dirY;
    float radius;
    float time_left;
};
extern std::vector<GojoOrb> g_GojoOrbs;

#endif // GAMEOBJECT_H

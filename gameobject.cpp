#include "gameobject.h"
#include<SDL2/SDL_image.h>
#include<SDL2/SDL.h>
#include<bits/stdc++.h>

bool g_SoundOn = true;
int g_TargetFPS = 60;

bool g_CheatSpeedBoost = false;
bool g_CheatInvincible = false;
bool g_CheatMultiShot = false;
bool g_CheatAura = false;
bool g_CheatAutoDodge = false;
bool g_CheatSummonMinions = false;
bool g_IsDodging = false;
float g_DodgeTimeLeft = 0;
std::vector<AfterImage> g_AfterImages;
float g_AfterImageSpawnTimer = 0;
float g_DodgeVelocityX = 0;
float g_DodgeVelocityY = 0;
float g_TimeScale = 1.0f;
bool g_PlayerIsDriving = false;
GameObject* g_CurrentCar = nullptr;
float g_CarSpeed = 0.0f;
float g_CarAngle = 0.0f;
std::vector<GojoOrb> g_GojoOrbs;

void drawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
      if (error <= 0) { ++y; error += ty; ty += 2; }
      if (error > 0) { --x; tx += 2; error += (tx - diameter); }
   }
}

bullet::bullet(SDL_Renderer* renderer,int damage, int speed, const char* path, int x, int y, int w, int h, double angle)
    : damage(damage), speed(speed), bullet_angle(angle) {
    texture = IMG_LoadTexture(renderer,path);
    srcRect = {0, 0, w, h};
    dstRect = {x, y, w, h};
    posX = x;
    posY = y;
}
void bullet::destroy() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

melee::melee(int width, int height, int damage,int range,int attack_duration, int cool_down_duration,const char* attack_soundd,int weapon_type){
    this->weapon_type = weapon_type;
    this->hitbox.w = width;
    this->hitbox.h = height;
    this->damage = damage;
    this->range = range;
    this->attack_duration = attack_duration;
    this->cool_down_duration = cool_down_duration;
    attack_sound = Mix_LoadWAV(attack_soundd);
}



void GameObject::meleeattack()
{
    Uint32 ct = SDL_GetTicks();

    if(isattacking && isreadyforattack)
    {
        lastattacktime = ct;
    }
    if(ct - lastattacktime <= shortrange_weapon.cool_down_duration) isreadyforattack = false;
    else isreadyforattack = true;
    if(lastattacktime == 0)
    {
        isreadyforattack = true;
    }
    if(object_type == 1 && isdead) isreadyforattack = false;
    if(ct - lastattacktime > shortrange_weapon.attack_duration) isattacking = false;
}

void GameObject::calculate_melee_hitbox()
{
    double angle_rad = objectangle * M_PI / 180.0;
    shortrange_weapon.hitbox.x = (double)(dstRect.x + dstRect.w/2 + shortrange_weapon.range * cos(angle_rad)) - shortrange_weapon.hitbox.w/2;
    shortrange_weapon.hitbox.y = (double)(dstRect.y + dstRect.h/2 + shortrange_weapon.range * sin(angle_rad)) - shortrange_weapon.hitbox.h/2;
}

//Constructor
GameObject::GameObject(SDL_Renderer* renderer,const char* texturePath,const char* d_sound,int x,int y,int w,int h,int objectspeed,float objecthp,double objectangle,int numofframe,Uint16 walking_duration,int object_type) {
    texture = IMG_LoadTexture(renderer,texturePath);
    dead_sound = Mix_LoadWAV(d_sound);
    complex_texture a;
    a.texture = texture;
    a.numofframe = numofframe;
    textures.push_back(a);
    srcRect = {0, 0, w, h};
    dstRect = {x, y, w, h};
    posX = x;
    posY = y;
    this->objectspeed = objectspeed;
    this->objecthp = objecthp;
    this->max_hp = objecthp;
    objectspeed2 = objectspeed;
    this->objectangle = objectangle;
    this->numofframe = numofframe;
    this->walking_duration = walking_duration;
    this->object_type = object_type;
}
//animation + texture;
GameObject::~GameObject() {
    // Free all textures in the vector
    for (auto& tex : textures) {
        if (tex.texture) {
            SDL_DestroyTexture(tex.texture);
        }
    }

    // Then free the current texture if it's not in the vector
    if (texture) {
        SDL_DestroyTexture(texture);
    }

}
void GameObject::setSourceRect(int x, int y, int w, int h) {
    srcRect = {x, y, w, h};
}
void GameObject::setDestinationRect(int x, int y, int w, int h) {
    dstRect = {x, y, w, h};
    posX = x;
    posY = y;
}

void GameObject::anothertexture()
{
    texture = textures[texture_index].texture;
    numofframe = textures[texture_index].numofframe;
    currentframe = 0;
}

void GameObject::emplace_backtexture(SDL_Renderer* renderer, const char* texturePath, int numofframe) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, texturePath);
    textures.emplace_back(complex_texture{newTexture, numofframe});
}

void GameObject::move(float dx, float dy) {
    posX += dx;
    posY += dy;
    dstRect.x = (int)posX;
    dstRect.y = (int)posY;
}

void GameObject::check_if_died()
{
    if(objecthp <= 0) isdead = true;
}

void GameObject::betterrender(double angle,SDL_Renderer* renderer)
{
    SDL_RenderCopyEx(renderer, texture, &srcRect, &dstRect,angle,nullptr,SDL_FLIP_NONE);
}


bool checkcollision(const SDL_Rect& A,const SDL_Rect& B){
    if(A.x + A.w <= B.x || B.x + B.w <= A.x) return false;
    if(A.y + A.h <= B.y || B.y + B.h <= A.y) return false;
    return true;
}
void follow(GameObject& follower,GameObject& target)
{
    int dx = target.dstRect.x - follower.dstRect.x;
    int dy = target.dstRect.y - follower.dstRect.y;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        follower.posX += static_cast<float>(follower.objectspeed * dx / distance);
        follower.posY += static_cast<float>(follower.objectspeed * dy / distance);
        follower.dstRect.x = (int)follower.posX;
        follower.dstRect.y = (int)follower.posY;
    }
}

int clamp(int value, int min_value, int max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

Uint32 getPixel(SDL_Surface* surface, int x, int y) {
    Uint8* pixelData = (Uint8*)surface->pixels;
    int bytesPerPixel = surface->format->BytesPerPixel;
    return *(Uint32*)(pixelData + y * surface->pitch + x * bytesPerPixel);
}

bool checkPerPixelCollision(SDL_Surface* spriteA, SDL_Rect rectA,SDL_Surface* spriteB, SDL_Rect rectB)
{
    int xStart = std::max(rectA.x, rectB.x);
    int xEnd   = std::min(rectA.x + rectA.w, rectB.x + rectB.w);
    int yStart = std::max(rectA.y, rectB.y);
    int yEnd   = std::min(rectA.y + rectA.h, rectB.y + rectB.h);

    for (int y = yStart; y < yEnd; y++) {
        for (int x = xStart; x < xEnd; x++) {
            int ax = x - rectA.x, ay = y - rectA.y;
            int bx = x - rectB.x, by = y - rectB.y;

            Uint32 pixelA = getPixel(spriteA, ax, ay);
            Uint32 pixelB = getPixel(spriteB, bx, by);

            if (pixelA != 0x00000000 && pixelB != 0x00000000) {
                return true;
            }
        }
    }
    return false;
}

void updateCamera(SDL_Rect &camera, GameObject& player, int SCREEN_WIDTH, int SCREEN_HEIGHT, int WORLD_WIDTH, int WORLD_HEIGHT)
{
    camera.x = player.dstRect.x + player.dstRect.w / 2 - SCREEN_WIDTH / 2;
    camera.y = player.dstRect.y + player.dstRect.h / 2 - SCREEN_HEIGHT / 2;

    // Giữ camera trong giới hạn của thế giới
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > WORLD_WIDTH - SCREEN_WIDTH) camera.x = WORLD_WIDTH - SCREEN_WIDTH;
    if (camera.y > WORLD_HEIGHT - SCREEN_HEIGHT) camera.y = WORLD_HEIGHT - SCREEN_HEIGHT;
}

void rendercopytocamera(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects)
{
    for (std::size_t i = 0; i < gameobjects.size(); i++)
    {
        SDL_Rect CameraRect = {
            gameobjects[i]->dstRect.x - camera.x,
            gameobjects[i]->dstRect.y - camera.y,
            gameobjects[i]->dstRect.w,
            gameobjects[i]->dstRect.h
        };

        SDL_RenderCopyEx(renderer, gameobjects[i]->texture, &gameobjects[i]->srcRect, &CameraRect,gameobjects[i]->objectangle,nullptr,SDL_FLIP_NONE);
    }
}

void rendercopytocamera_for_alive(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects)
{
    for (std::size_t i = 0; i < gameobjects.size(); i++)
    {
        if((gameobjects[i]->isdead)) continue;
        SDL_Rect CameraRect = {
            gameobjects[i]->dstRect.x - camera.x,
            gameobjects[i]->dstRect.y - camera.y,
            gameobjects[i]->dstRect.w,
            gameobjects[i]->dstRect.h
        };

        SDL_RenderCopyEx(renderer, gameobjects[i]->texture, &gameobjects[i]->srcRect, &CameraRect,gameobjects[i]->objectangle,nullptr,SDL_FLIP_NONE);

        if (gameobjects[i]->max_hp > 0) {
            SDL_Rect hpBg = { CameraRect.x, CameraRect.y - 10, CameraRect.w, 5 };
            float hpPct = gameobjects[i]->objecthp / gameobjects[i]->max_hp;
            if (hpPct < 0) hpPct = 0;
            SDL_Rect hpFg = { CameraRect.x, CameraRect.y - 10, (int)(CameraRect.w * hpPct), 5 };
            
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &hpBg);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &hpFg);
        }
    }
}
void rendercopytocamera_for_dead(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects)
{
    for (std::size_t i = 0; i < gameobjects.size(); i++)
    {
        if(!(gameobjects[i]->isdead)) continue;
        SDL_Rect CameraRect = {
            gameobjects[i]->dstRect.x - camera.x,
            gameobjects[i]->dstRect.y - camera.y,
            gameobjects[i]->dstRect.w,
            gameobjects[i]->dstRect.h
        };

        SDL_RenderCopyEx(renderer, gameobjects[i]->texture, &gameobjects[i]->srcRect, &CameraRect,gameobjects[i]->objectangle,nullptr,SDL_FLIP_NONE);
    }
}

void rendercopytocamera_for_bullet(SDL_Renderer* renderer, SDL_Rect camera, std::vector<bullet*>& bullets)
{
    for(auto z : bullets)
    {
        SDL_Rect camera_rect = {z->dstRect.x - camera.x,z->dstRect.y - camera.y,z->dstRect.w,z->dstRect.h};
        SDL_RenderCopyEx(renderer,z->texture,&z->srcRect,&camera_rect,z->bullet_angle,nullptr,SDL_FLIP_NONE);
    }
}

void Initializer(SDL_Window* &window,SDL_Renderer* &renderer,int SCREEN_WIDTH,int SCREEN_HEIGHT)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    window = SDL_CreateWindow("Murderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}
void handleMovement(GameObject& object, int speed, float dt) {
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_W])
    {
        object.posY -= speed * 60.0f * dt; // Đi lên
    }
    if (keystates[SDL_SCANCODE_S])
    {
        object.posY += speed * 60.0f * dt;
    } // Đi xuống
    if (keystates[SDL_SCANCODE_A])
    {
        object.posX -= speed * 60.0f * dt;
    } // Đi trái
    if (keystates[SDL_SCANCODE_D])
    {
        object.posX += speed * 60.0f * dt;
    } // Đi phải
    object.dstRect.x = (int)object.posX;
    object.dstRect.y = (int)object.posY;
}

void EndEverything(SDL_Window* &window,SDL_Renderer* &renderer)
{
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void greater_check_if_died(std::vector<GameObject*> &objects, GameObject& player)
{
    for(auto it = objects.begin();it != objects.end();)
    {
        float dx = (*it)->posX - player.posX;
        float dy = (*it)->posY - player.posY;
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist > 3500.0f)
        {
            (*it)->isdead = true;
            (*it)->played_dead_sound = true; // allow ObjectKiller to erase it without sound
        }
        (*it)->check_if_died();
        if((*it)->object_type == 1 && (*it)->isdead) (*it)->go_through_able = true;
        it++;
    }
}

void ObjectKiller(std::vector<GameObject*> &objects) {
    for(auto it = objects.begin(); it != objects.end();)
        {
            if((*it)->isdead && (*it)->played_dead_sound)
                { delete *it; it = objects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

void angle_otom(GameObject &player,const SDL_Rect &camera)
{
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        float worldMouseX = mouseX + camera.x;
        float worldMouseY = mouseY + camera.y;

        float dx = worldMouseX - (player.dstRect.x + player.dstRect.w / 2);
        float dy = worldMouseY - (player.dstRect.y + player.dstRect.h / 2);
        player.objectangle = atan2(dy, dx) * 180.0 / M_PI;
}

void angle_to_player(SDL_Rect& tempdstRect,GameObject& npc,GameObject& player)
{
    float dx = npc.dstRect.x - tempdstRect.x;
    float dy = npc.dstRect.y - tempdstRect.y;
    angle_a_to_b(npc,player);
    if(dx != 0 || dy != 0)npc.isrunning = true;
    else npc.isrunning = false;
}

void angle_otomovement(SDL_Rect& tempdstRect,GameObject& npc)
{
    float dx = npc.dstRect.x - tempdstRect.x;
    float dy = npc.dstRect.y - tempdstRect.y;
    if(dx != 0 || dy != 0)
    {
        npc.objectangle = atan2(dy,dx)*180/ M_PI;
        npc.isrunning = true;
    }
    else npc.isrunning = false;
}

void angle_a_to_b(GameObject &a,GameObject &b)
{
    float dx = b.dstRect.x - a.dstRect.x;
    float dy = b.dstRect.y - a.dstRect.y;
    a.objectangle = atan2(dy, dx) * 180.0 / M_PI;
}

void random_movement(GameObject& a, float dt)
{
    int method1 = rand() % 101;
    if(method1 >= 90) a.objectangle = rand() % 361 - 180;
    int method = rand() % 11;
    if(method <= 7)
    {
    a.posX += a.objectspeed * cos(a.objectangle*M_PI/180.0) * 60.0f * dt;
    a.posY += a.objectspeed * sin(a.objectangle*M_PI/180.0) * 60.0f * dt;
    a.dstRect.x = (int)a.posX;
    a.dstRect.y = (int)a.posY;
    }
}

void check_if_moved(GameObject& moving_things, SDL_Rect& temprect) {
    moving_things.isrunning = (moving_things.dstRect.x != temprect.x || moving_things.dstRect.y != temprect.y);
}

bool check_if_collided(const GameObject& moving_things,const std::vector<GameObject*>& Other_object)
{
    for(auto x : Other_object)
    {
        if(checkcollision(x->dstRect,moving_things.dstRect) && !(x->go_through_able)) return true;
    }
    return false;
}

bool check_if_collided_plus_physic_1(GameObject& moving_things,const std::vector<GameObject*>& Other_object, float dt)
{
    bool result = false;
    for(auto x : Other_object)
    {
        int tmp_speed = moving_things.objectspeed;
        while(checkcollision(x->dstRect,moving_things.dstRect) && !(x->go_through_able))
        {
            moving_things.objectspeed = 0;
            physics_1(moving_things,*x, dt);
            result = true;
        }
        moving_things.objectspeed = tmp_speed;
    }
    return result;
}

void check_attack_and_calculate_damage(const GameObject& attacker,std::vector<GameObject*>& Other_object, float dt)
{
    for(auto& x : Other_object)
    {
        if(attacker.isattacking && checkcollision(attacker.shortrange_weapon.hitbox,x->dstRect))
        {
            if (attacker.object_type == 2 && x->object_type == 2) continue; // Minions don't hurt Minions
            if (attacker.object_type == 0 && x->object_type == 0) continue; // Enemies don't hurt Enemies
            x->objecthp -= attacker.shortrange_weapon.damage * 60.0f * dt;
        }
    }
}

void check_attack_and_calculate_damage_for_a_and_b(const GameObject& attacker,GameObject& victim, float dt)
{
    if(attacker.isattacking && checkcollision(attacker.shortrange_weapon.hitbox,victim.dstRect))
       {
           if (g_CheatAutoDodge) {
               g_IsDodging = true;
               g_DodgeTimeLeft = 1.5f;
               g_AfterImages.push_back({victim.texture, victim.srcRect, victim.dstRect, victim.objectangle, 255.0f});
               g_DodgeVelocityX = -cos(attacker.objectangle * M_PI / 180.0) * 800.0f;
               g_DodgeVelocityY = -sin(attacker.objectangle * M_PI / 180.0) * 800.0f;
               return;
           }
           if (g_CheatInvincible || g_PlayerIsDriving) return;
           if(attacker.objecthp > 0 || attacker.object_type == 1)victim.objecthp -= attacker.shortrange_weapon.damage * 60.0f * dt;
       }
}

int calculate_distance(GameObject& a,GameObject& b)
{
    int x = abs(a.dstRect.x - b.dstRect.x);
    int y = abs(a.dstRect.y - b.dstRect.y);
    return sqrt(x*x + y*y);
}

void the_ultimate_status_handler_for_things(std::vector<GameObject*>other_objects,std::vector<GameObject*>npcs,GameObject& player, float dt)
{
    for(auto x : other_objects)
    {
        if(x->isdead) x->isattacking = true;
        x->meleeattack();
        x->calculate_melee_hitbox();
        check_attack_and_calculate_damage_for_a_and_b(*x,player, dt);
        check_attack_and_calculate_damage(*x,npcs, dt);
    }
}

void physics_1(GameObject& a, GameObject& b, float dt) {
    float dx = a.dstRect.x+a.dstRect.w/2 - b.dstRect.x - b.dstRect.w;
    float dy = a.dstRect.y + a.dstRect.h/2 - b.dstRect.y - b.dstRect.h/2;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance == 0) return;

    float pushX = (dx / distance) * 5;
    float pushY = (dy / distance) * 5;

    a.posX += pushX;
    a.posY += pushY;
    a.dstRect.x = (int)a.posX;
    a.dstRect.y = (int)a.posY;
}

void the_ultimate_movement_and_status_handler(SDL_Renderer* renderer,std::vector<GameObject*>npcs,const std::vector<std::vector<cell>>& map_grid,int cellsize,std::vector<GameObject*>other_objects,GameObject& player,std::vector<bullet*>& bullets, float dt)
{
        int i = 0;
        std::vector<SDL_Rect> npcstempsrcrect;
        for(auto &x : npcs)
        {
            npcstempsrcrect.push_back(x->dstRect);
            if(x->isdead) x->objectspeed = 0;
        }
        for(auto &x : npcs)
        {
            if(x->isdead)
            {
                continue;
            }
            x->posX += x->velX * dt;
            x->posY += x->velY * dt;
            x->dstRect.x = (int)x->posX;
            x->dstRect.y = (int)x->posY;
            x->velX *= 0.9f;
            x->velY *= 0.9f;
            if (x->object_type == 2) {
                GameObject* target = nullptr;
                int min_dist = 2000;
                for (auto e : npcs) {
                    if (e->object_type == 0 && !e->isdead) {
                        int dist = calculate_distance(*x, *e);
                        if (dist < min_dist) { min_dist = dist; target = e; }
                    }
                }
                if (target) {
                    float dx = target->dstRect.x - x->dstRect.x;
                    float dy = target->dstRect.y - x->dstRect.y;
                    float len = sqrt(dx*dx + dy*dy);
                    if (len > 0 && len <= 1000) {
                        x->posX += (dx/len) * 60 * dt;
                        x->posY += (dy/len) * 60 * dt;
                        x->dstRect.x = (int)x->posX;
                        x->dstRect.y = (int)x->posY;
                        x->objectangle = atan2(dy, dx) * 180 / M_PI;
                    }
                    if((min_dist < 150) || (min_dist <= 500 && x->shortrange_weapon.weapon_type == 1)) {
                        x->isattacking = true;
                    }
                } else {
                    float dx = player.dstRect.x - x->dstRect.x;
                    float dy = player.dstRect.y - x->dstRect.y;
                    float len = sqrt(dx*dx + dy*dy);
                    if (len > 150) {
                        x->posX += (dx/len) * 60 * dt;
                        x->posY += (dy/len) * 60 * dt;
                        x->dstRect.x = (int)x->posX;
                        x->dstRect.y = (int)x->posY;
                        x->objectangle = atan2(dy, dx) * 180 / M_PI;
                    }
                }
            } else {
                if(calculate_distance(*x,player) <= 1000)follow_the_vector(*x,map_grid,cellsize, dt);
                else random_movement(*x, dt);
                if(check_if_collided_plus_physic_1(*x,other_objects, dt))x->isattacking = true;

                if((calculate_distance(*x,player) < 150) || (calculate_distance(*x,player) <= 500 && x->shortrange_weapon.weapon_type == 1))
                {
                    x->isattacking = true;
                }
            }

            if(x->isattacking && !(x->played_attack_sound) && x->shortrange_weapon.weapon_type == 1)
            {
                double rad = x->objectangle * M_PI / 180.0;
                bullet *bullet1 = new bullet(renderer,5000,40,"bullets/bullet1.png",(x->dstRect.x+x->dstRect.w/2)+40*cos(rad),(x->dstRect.y+x->dstRect.h/2)+40*sin(rad),16,6,x->objectangle);
                bullets.push_back(bullet1);
            }
            x->meleeattack();
            x->calculate_melee_hitbox();
            
            if (x->object_type == 2) {
                check_attack_and_calculate_damage(*x, npcs, dt);
            } else {
                check_attack_and_calculate_damage_for_a_and_b(*x,player, dt);
                check_attack_and_calculate_damage(*x,other_objects, dt);
            }
        }
        for(auto &x : npcs)
        {
            if(x->isdead)
            {
                i++;
                continue;
            }
            if(calculate_distance(*x,player) <= 1000) angle_to_player(npcstempsrcrect[i++],*x,player);
            else angle_otomovement(npcstempsrcrect[i++],*x);
        }
}

void the_ultimate_movement_and_status_handler_for_player(SDL_Renderer* renderer,GameObject& player,const SDL_Rect& camera,std::vector<GameObject*>other_objects,std::vector<GameObject*> npcs,std::vector<bullet*>& bullets, float dt)
{
    if(player.isdead)
    {
        player.objectspeed = 0;
        return;
    }
    SDL_Rect tempplayerdstrect = player.dstRect;
    if (g_IsDodging) {
        player.posX += g_DodgeVelocityX * dt;
        player.posY += g_DodgeVelocityY * dt;
        player.dstRect.x = (int)player.posX;
        player.dstRect.y = (int)player.posY;
        g_DodgeVelocityX *= 0.85f;
        g_DodgeVelocityY *= 0.85f;
    } else {
        if (g_PlayerIsDriving) {
            const Uint8* keystates = SDL_GetKeyboardState(NULL);
            if (keystates[SDL_SCANCODE_W]) g_CarSpeed += 1500.0f * dt;
            if (keystates[SDL_SCANCODE_S]) g_CarSpeed -= 1500.0f * dt;
            
            if (g_CarSpeed > 10 || g_CarSpeed < -10) {
                float steer = (g_CarSpeed > 0) ? 1.0f : -1.0f;
                if (keystates[SDL_SCANCODE_A]) g_CarAngle -= 120.0f * steer * dt;
                if (keystates[SDL_SCANCODE_D]) g_CarAngle += 120.0f * steer * dt;
            }
            
            g_CarSpeed *= 0.95f; 
            if (g_CarSpeed > 2000.0f) g_CarSpeed = 2000.0f;
            if (g_CarSpeed < -800.0f) g_CarSpeed = -800.0f;
            
            player.posX += cos(g_CarAngle * M_PI / 180.0) * g_CarSpeed * dt;
            player.posY += sin(g_CarAngle * M_PI / 180.0) * g_CarSpeed * dt;
            player.objectangle = g_CarAngle;
            player.dstRect.x = (int)player.posX;
            player.dstRect.y = (int)player.posY;
        } else {
            int currentSpeed = g_CheatSpeedBoost ? player.objectspeed * 3 : player.objectspeed;
            handleMovement(player, currentSpeed, dt);
            angle_otom(player,camera);
        }
    }
    player.meleeattack();
    if(player.isattacking)
    {
        player.calculate_melee_hitbox();
        check_attack_and_calculate_damage(player,npcs, dt);
        check_attack_and_calculate_damage(player,other_objects, dt);
        if((player.shortrange_weapon.weapon_type == 1 || g_CheatMultiShot) && !player.played_attack_sound)
        {
            if (g_CheatMultiShot) {
                for(int a = -30; a <= 30; a += 15) {
                    double rad = (player.objectangle + a) * M_PI / 180.0;
                    bullet *bullet1 = new bullet(renderer,5000,30,"bullets/bullet1.png",(player.dstRect.x+player.dstRect.w/2)+40*cos(rad),(player.dstRect.y+player.dstRect.h/2)+40*sin(rad),16,6,player.objectangle + a);
                    bullets.push_back(bullet1);
                }
            } else {
                double rad = player.objectangle * M_PI / 180.0;
                bullet *bullet1 = new bullet(renderer,5000,30,"bullets/bullet1.png",(player.dstRect.x+player.dstRect.w/2)+40*cos(rad),(player.dstRect.y+player.dstRect.h/2)+40*sin(rad),16,6,player.objectangle);
                bullets.push_back(bullet1);
            }
        }
    }
    
    if (g_CheatAura) {
        SDL_Rect auraRect = { player.dstRect.x - 150, player.dstRect.y - 150, player.dstRect.w + 300, player.dstRect.h + 300 };
        for (auto& n : npcs) {
            if (!n->isdead && checkcollision(auraRect, n->dstRect)) {
                n->objecthp -= 1000.0f * dt;
            }
        }
        for (auto& o : other_objects) {
            if (!o->isdead && checkcollision(auraRect, o->dstRect)) {
                o->objecthp -= 1000.0f * dt;
            }
        }
    }
    check_if_moved(player,tempplayerdstrect);
    if(check_if_collided(player,other_objects))
    {
        if (g_PlayerIsDriving) {
            for (auto t : other_objects) {
                if (t != g_CurrentCar && checkcollision(player.dstRect, t->dstRect)) {
                    t->objecthp -= 5000 * dt; // Car rams and destroys obstacles!
                }
            }
        } else {
            player.dstRect = tempplayerdstrect;
            player.posX = tempplayerdstrect.x;
            player.posY = tempplayerdstrect.y;
        }
    }
    
    if (g_PlayerIsDriving) {
        for (auto npc : npcs) {
            if (npc->object_type == 0 && !npc->isdead && checkcollision(player.dstRect, npc->dstRect)) {
                if (std::abs(g_CarSpeed) > 100) {
                    npc->velX = cos(g_CarAngle * M_PI / 180.0) * g_CarSpeed * 1.5f;
                    npc->velY = sin(g_CarAngle * M_PI / 180.0) * g_CarSpeed * 1.5f;
                    npc->objecthp -= std::abs(g_CarSpeed) * 10.0f * dt;
                }
            }
        }
    }

    check_if_moved(player,tempplayerdstrect);
}

void the_ultimate_animation_handler(GameObject& moving_things, int frame_duration_for_walking_texture)
{
    if(moving_things.object_type == 1)
    {
        int new_texture_index = moving_things.isattacking ? 1 : 0;
        if(moving_things.texture_index != new_texture_index)
        {
            moving_things.texture_index = new_texture_index;
            moving_things.anothertexture();
        }
        if(moving_things.isdead)
        {
            if(!moving_things.isattacking)
            {
            moving_things.texture_index = 2;
            moving_things.anothertexture();
            moving_things.setSourceRect(0,0,1000,1000);
            return;
            }
        }
        if(moving_things.isattacking)
        {
            int width, height;
            SDL_QueryTexture(moving_things.texture, nullptr, nullptr, &width, &height);
            int frame_width = width / moving_things.numofframe;
            Uint32 ct = SDL_GetTicks();
            int frame_duration_for_attacking = moving_things.shortrange_weapon.attack_duration / moving_things.numofframe;
            if (ct - moving_things.lastattacktime >= frame_duration_for_attacking * (moving_things.currentframe + 1))
            {
            moving_things.currentframe = (moving_things.currentframe + 1) % moving_things.numofframe;
            moving_things.setSourceRect(frame_width * moving_things.currentframe, 0, frame_width, height);
            }
            return;
        }
    }

    if(moving_things.object_type == 0 || moving_things.object_type == 2)
    {
    if(moving_things.isdead)
    {
        moving_things.texture_index = 2;
        moving_things.anothertexture();
        moving_things.setSourceRect(0,0,1000,1000);
        moving_things.setDestinationRect(moving_things.dstRect.x,moving_things.dstRect.y,120,110);
        return;
    }
    int new_texture_index = moving_things.isattacking ? 1 : 0;
    if (moving_things.texture_index != new_texture_index) {
        moving_things.texture_index = new_texture_index;
        moving_things.anothertexture();
    }

    int width, height;
    SDL_QueryTexture(moving_things.texture, nullptr, nullptr, &width, &height);
    int frame_width = width / moving_things.numofframe;
    Uint32 ct = SDL_GetTicks();
    int frame_duration_for_attacking = moving_things.shortrange_weapon.attack_duration / moving_things.numofframe;

    if (moving_things.isattacking) {
        if (ct - moving_things.lastattacktime >= frame_duration_for_attacking * (moving_things.currentframe + 1)) {
            moving_things.currentframe = (moving_things.currentframe + 1) % moving_things.numofframe;
            moving_things.setSourceRect(frame_width * moving_things.currentframe, 0, frame_width, height);
        }
        return;
    }


    if (moving_things.isrunning) {
        if (ct - moving_things.lastframetime >= frame_duration_for_walking_texture) {
            moving_things.currentframe = (moving_things.currentframe + 1) % moving_things.numofframe;
            moving_things.setSourceRect(frame_width * moving_things.currentframe, 0, frame_width, height);
            moving_things.lastframetime = ct;
        }
    }
    else { // Nếu đứng yên
        moving_things.setSourceRect(0, 0, frame_width, height);
        moving_things.currentframe = 0;
    }
    }
}



void the_more_ultimate_animation_handler(std::vector<GameObject*>moving_things)
{
    for(auto it = moving_things.begin(); it != moving_things.end(); it++)
    {
            the_ultimate_animation_handler(*(*it),(*it)->walking_duration);
    }
}

void the_ultimate_sound_effects_handler(std::vector<GameObject*>moving_things)
{
     for (auto it : moving_things)
    {
        if(it->isdead && !it->played_dead_sound)
        {
            if (g_SoundOn) Mix_PlayChannel(-1, it->dead_sound, 0);
            it->played_dead_sound = true;
        }
        if(it->object_type == 0 || it->object_type == 2)
        {
            if(it->isreadyforattack) it->played_attack_sound = false;
            if(it->isattacking && !it->played_attack_sound)
            {
            if (g_SoundOn) Mix_PlayChannel(-1,(*it).shortrange_weapon.attack_sound,0);
            it->played_attack_sound = true;
            }
        }
    }
}

void the_ultimate_sound_effects_handler_for_single_object(GameObject& moving_thing)
{
    if(moving_thing.isdead && !moving_thing.played_dead_sound)
    {
        if (g_SoundOn) Mix_PlayChannel(-1,moving_thing.dead_sound,0);
        moving_thing.played_dead_sound = true;
    }
    if(moving_thing.isreadyforattack) moving_thing.played_attack_sound = false;
    if(moving_thing.isattacking && !moving_thing.played_attack_sound)
    {
        if (g_SoundOn) Mix_PlayChannel(-1,moving_thing.shortrange_weapon.attack_sound,0);
        moving_thing.played_attack_sound = true;
    }
}


void spawn_enemies(std::vector<GameObject*>& moving_things, SDL_Renderer* renderer, int method, GameObject& player)
{
    float angle = (rand() % 360) * M_PI / 180.0f;
    float dist = 800.0f + (rand() % 800);
    int x = player.posX + cos(angle) * dist;
    int y = player.posY + sin(angle) * dist;

    if(method == 0)
    {

        GameObject *npc1 = new GameObject(renderer,"npcs/npc1.png","sounds/dead.wav",x,y,74,74,10,300,0,8,20,0);
        npc1->shortrange_weapon = {70,70,1000,30,320,500,"sounds/metal_pipe.wav",0};
        npc1->emplace_backtexture(renderer,"npcs/npc1-a.png",7);
        npc1->emplace_backtexture(renderer,"npcs/npc1-d.png",1);
        npc1->setSourceRect(0,0,32,32);
        moving_things.push_back(npc1);
    }
    if(method == 1)
    {
        GameObject *npc2 = new GameObject(renderer,"npcs/npc2.png","sounds/dead.wav",x,y,80,80,6,300,0,8,16,0);
        npc2->shortrange_weapon = {100,100,0,0,100,600,"sounds/gun_shot.wav",1};
        npc2->emplace_backtexture(renderer,"npcs/npc2-a.png",3);
        npc2->emplace_backtexture(renderer,"npcs/npc2-d.png",1);
        npc2->setSourceRect(0,0,32,32);
        moving_things.push_back(npc2);
    }
    if(method == 2)
    {
        GameObject *npc3 = new GameObject(renderer,"npcs/npc3.png","sounds/dead.wav",x,y,93,93,8,300,0,8,16,0);
        npc3->shortrange_weapon = {70,70,1100,30,300,850,"sounds/metal_pipe.wav",0};
        npc3->emplace_backtexture(renderer,"npcs/npc3-a.png",7);
        npc3->emplace_backtexture(renderer,"npcs/npc3-d.png",1);
        npc3->setSourceRect(0,0,44,44);
        moving_things.push_back(npc3);
    }
}

int count_dead_things(const std::vector<GameObject*>& moving_things)
{
    int sum = 0;
    for(auto x : moving_things)
    {
        if(x->isdead) sum++;
    }
    return sum;
}

void handle_movement_of_bullet(std::vector<bullet*>& bullets, float dt)
{
    for(auto z : bullets)
    {
        double rad = z->bullet_angle * M_PI / 180.0;
        z->posX += cos(rad) * z->speed * 60.0f * dt;
        z->posY += sin(rad) * z->speed * 60.0f * dt;
        z->dstRect.x = (int)z->posX;
        z->dstRect.y = (int)z->posY;
    }
}

void handle_collision_of_bullet(std::vector<bullet*>& bullets,std::vector<GameObject*> things, bool is_player)
{
    for(auto z : bullets)
    {
        for(auto m : things)
        {
            if(checkcollision(z->dstRect,m->dstRect))
            {
                if(m->isdead) continue;
                if(is_player) {
                    if (g_CheatAutoDodge) {
                        g_IsDodging = true;
                        g_DodgeTimeLeft = 1.5f;
                        g_AfterImages.push_back({m->texture, m->srcRect, m->dstRect, m->objectangle, 255.0f});
                        g_DodgeVelocityX = -cos(z->bullet_angle * M_PI / 180.0) * 800.0f;
                        g_DodgeVelocityY = -sin(z->bullet_angle * M_PI / 180.0) * 800.0f;
                        continue;
                    }
                    if (g_CheatInvincible || g_PlayerIsDriving) continue;
                }
                m->objecthp -= z->damage;
                z->isdead = true;
            }
        }
    }
}

void handle_status_and_delete_dead_bullets(std::vector<bullet*>& bullets, int map_x, int map_y)
{
    for (auto it = bullets.begin(); it != bullets.end(); )
    {
        if ((*it)->dstRect.x < 0 || (*it)->dstRect.x > (map_x + 100) || (*it)->dstRect.y < 0 || (*it)->dstRect.y > (map_y + 100) || (*it)->isdead)
        {
            SDL_DestroyTexture((*it)->texture);
            memset(*it, 0, sizeof(**it));
            delete (*it);
            it = bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }
    bullets.shrink_to_fit();  // Giải phóng bộ nhớ dư thừa
}



//celllllllll










cell::cell()
{
    x = 0;
    y = 0;
    cost = -2;
    flowVector ={0,0};
}

bool isFull(std::vector<std::vector<cell>>& matrix,int default_value){
    int size_x = matrix.size();
    int size_y = matrix[0].size();
    if (matrix.empty() || matrix[0].empty()) return false;
    for(int x = 0;x < size_x;x++)
    {
        for(int y = 0;y < size_y;y++)
        {
            if(matrix[x][y].cost == default_value)
            {
                return false;
            }
        }
    }
    return true;
}

void computeCostField(std::vector<std::vector<cell>>& grid) {
    if (grid.empty() || grid[0].empty()) return;

    int size_x = grid.size();
    int size_y = grid[0].size();
    std::queue<std::pair<int, int>> q;

    for (int x = 0; x < size_x; x++) {
        for (int y = 0; y < size_y; y++) {
            if (grid[x][y].cost == 0) {
                q.push({x, y});
            }
        }
    }

    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    while (!q.empty()) {
        int cx = q.front().first;
        int cy = q.front().second;
        q.pop();

        int current_cost = grid[cx][cy].cost;

        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (nx >= 0 && nx < size_x && ny >= 0 && ny < size_y) {
                if (grid[nx][ny].cost == -2) {
                    grid[nx][ny].cost = current_cost + 1;
                    q.push({nx, ny});
                }
            }
        }
    }
}




void drawVector(std::vector<std::vector<cell>>& arr) {
    int sizex = arr.size();
    int sizey = arr[0].size();
    for (int x = 0; x < sizex; x++) {
        for (int y = 0; y < sizey; y++) {

            int minCost = arr[x][y].cost;
            SDL_Point bestDir = {0, 0};

            if (x + 1 < sizex && arr[x + 1][y].cost < minCost) {
                minCost = arr[x + 1][y].cost;
                bestDir = {1, 0};
            }
            if (x - 1 >= 0 && arr[x - 1][y].cost < minCost) {
                minCost = arr[x - 1][y].cost;
                bestDir = {-1, 0};
            }
            if (y + 1 < sizey && arr[x][y + 1].cost < minCost) {
                minCost = arr[x][y + 1].cost;
                bestDir = {0, 1};
            }
            if (y - 1 >= 0 && arr[x][y - 1].cost < minCost) {
                minCost = arr[x][y - 1].cost;
                bestDir = {0, -1};
            }

            arr[x][y].flowVector = bestDir;
        }
    }
}

void initialcostfield(std::vector<std::vector<cell>>& grid,const std::vector<GameObject*>other_objects,int cellsize)
{
      int gridRows = grid.size();
      if (gridRows == 0) return;
      int gridCols = grid[0].size();
      for(auto x : other_objects)
      {
          int widthcell = ceil((double)x->dstRect.w / (double)cellsize);
          int heightcell = ceil((double)x->dstRect.h / (double)cellsize);
          int xcell = x->dstRect.x / cellsize;
          int ycell = x->dstRect.y/cellsize;
          for(int i = xcell;i < xcell+widthcell;i++)
          {
              for(int y = ycell;y < ycell+heightcell;y++)
              {
                  if (y >= 0 && y < gridRows && i >= 0 && i < gridCols) {
                      grid[y][i].cost = 1000;
                  }
              }
          }
      }
}

bool isValid(int x, int y, int sizex, int sizey, const std::vector<std::vector<cell>>& arr) {
    return x >= 0 && x < sizex && y >= 0 && y < sizey && arr[x][y].cost != -1;
}

void drawbetterVector(std::vector<std::vector<cell>>& arr) {
    if (arr.empty() || arr[0].empty()) return;

    int sizex = arr.size();
    int sizey = arr[0].size();

    const std::vector<SDL_Point> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, // 4 hướng chính
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // 4 hướng chéo
    };

    for (int x = 0; x < sizex; x++) {
        for (int y = 0; y < sizey; y++) {

            int minCost = arr[x][y].cost;
            SDL_Point bestDir = {0, 0};

            for (const auto& dir : directions) {
                int nx = x + dir.x, ny = y + dir.y;
                if (isValid(nx, ny, sizex, sizey, arr) && arr[nx][ny].cost < minCost) {
                    minCost = arr[nx][ny].cost;
                    bestDir = dir;
                }
            }

            arr[x][y].flowVector = bestDir;
        }
    }
}

void follow_the_vector(GameObject& npc,const std::vector<std::vector<cell>>& grid, int cellsize, float dt) {
    int gridX = npc.dstRect.y / cellsize;
    int gridY = npc.dstRect.x / cellsize;

    if (gridX < 0 || gridX >= grid.size() || gridY < 0 || gridY >= grid[0].size()) return;

    SDL_Point direction = grid[gridX][gridY].flowVector;

    npc.posX += direction.y * npc.objectspeed * 60.0f * dt;
    npc.posY += direction.x * npc.objectspeed * 60.0f * dt;
    npc.dstRect.x = (int)npc.posX;
    npc.dstRect.y = (int)npc.posY;
}

bool checkClick(SDL_Rect button, int x, int y) {
    return (x > button.x && x < button.x + button.w &&
            y > button.y && y < button.y + button.h);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_Rect rect) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect = { rect.x + (rect.w - surface->w)/2, rect.y + (rect.h - surface->h)/2, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

void game()
{
    srand(time(0));
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    Initializer(window,renderer,1280,720);
    Mix_AllocateChannels(50);

    SDL_Rect startButton = {100, 100, 300, 50};
    SDL_Rect settingsButton = {100, 200, 300, 50};
    SDL_Rect quitButton = {100, 300, 300, 50};

    SDL_Texture* start = IMG_LoadTexture(renderer,"menu/start.png");
    SDL_Texture* background1 = IMG_LoadTexture(renderer,"menu/background.png");
    SDL_Texture* quit = IMG_LoadTexture(renderer,"menu/quit.png");

    Mix_Music* background_music = Mix_LoadMUS("background.mp3");
    if (g_SoundOn) Mix_PlayMusic(background_music, -1);

    TTF_Font* font = TTF_OpenFont("arial.ttf", 28);

    SDL_Event event;
    bool running1 = true;
    bool running = true;
    while(running1)
    {
        SDL_RenderCopy(renderer,background1,nullptr,nullptr);
        SDL_RenderCopy(renderer,start,nullptr,&startButton);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &settingsButton);
        if (font) renderText(renderer, font, "Settings", {255,255,255,255}, settingsButton);
        
        SDL_RenderCopy(renderer,quit,nullptr,&quitButton);
        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
            {
                running1 = false;
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = event.button.x, y = event.button.y;
                if (checkClick(settingsButton, x, y))
                {
                    bool inSettings = true;
                    SDL_Rect soundButton = {100, 50, 300, 50};
                    SDL_Rect fpsButton = {100, 150, 300, 50};
                    SDL_Rect cheatMenuBtn = {100, 250, 300, 50};
                    SDL_Rect backButton = {100, 350, 300, 50};
                    while(inSettings) {
                        SDL_Event setEvent;
                        while(SDL_PollEvent(&setEvent)) {
                            if (setEvent.type == SDL_QUIT) {
                                inSettings = false; running1 = false; running = false;
                            }
                            if (setEvent.type == SDL_MOUSEBUTTONDOWN) {
                                int sx = setEvent.button.x, sy = setEvent.button.y;
                                if (checkClick(soundButton, sx, sy)) {
                                    g_SoundOn = !g_SoundOn;
                                    if(g_SoundOn) Mix_ResumeMusic(); else Mix_PauseMusic();
                                }
                                if (checkClick(fpsButton, sx, sy)) {
                                    if(g_TargetFPS == 30) g_TargetFPS = 60;
                                    else if(g_TargetFPS == 60) g_TargetFPS = 144;
                                    else if(g_TargetFPS == 144) g_TargetFPS = 0;
                                    else g_TargetFPS = 30;
                                }
                                if (checkClick(cheatMenuBtn, sx, sy)) {
                                    bool inCheats = true;
                                    SDL_Rect speedBtn = {100, 50, 300, 50};
                                    SDL_Rect invBtn = {100, 150, 300, 50};
                                    SDL_Rect multiBtn = {100, 250, 300, 50};
                                    SDL_Rect auraBtn = {100, 350, 300, 50};
                                    SDL_Rect backCheatBtn = {100, 450, 300, 50};
                                    while(inCheats) {
                                        SDL_Event cEvent;
                                        while(SDL_PollEvent(&cEvent)) {
                                            if (cEvent.type == SDL_QUIT) {
                                                inCheats = false; inSettings = false; running1 = false; running = false;
                                            }
                                            if (cEvent.type == SDL_MOUSEBUTTONDOWN) {
                                                int cx = cEvent.button.x, cy = cEvent.button.y;
                                                if (checkClick(speedBtn, cx, cy)) g_CheatSpeedBoost = !g_CheatSpeedBoost;
                                                if (checkClick(invBtn, cx, cy)) g_CheatInvincible = !g_CheatInvincible;
                                                if (checkClick(multiBtn, cx, cy)) g_CheatMultiShot = !g_CheatMultiShot;
                                                if (checkClick(auraBtn, cx, cy)) g_CheatAura = !g_CheatAura;
                                                if (checkClick(backCheatBtn, cx, cy)) inCheats = false;
                                            }
                                        }
                                        SDL_RenderCopy(renderer,background1,nullptr,nullptr);
                                        
                                        SDL_SetRenderDrawColor(renderer, g_CheatSpeedBoost?0:255, g_CheatSpeedBoost?255:0, 0, 255);
                                        SDL_RenderFillRect(renderer, &speedBtn);
                                        if(font) renderText(renderer, font, g_CheatSpeedBoost?"Speed: x3":"Speed: x1", {255,255,255,255}, speedBtn);
                                        
                                        SDL_SetRenderDrawColor(renderer, g_CheatInvincible?0:255, g_CheatInvincible?255:0, 0, 255);
                                        SDL_RenderFillRect(renderer, &invBtn);
                                        if(font) renderText(renderer, font, g_CheatInvincible?"God Mode: ON":"God Mode: OFF", {255,255,255,255}, invBtn);
                                        
                                        SDL_SetRenderDrawColor(renderer, g_CheatMultiShot?0:255, g_CheatMultiShot?255:0, 0, 255);
                                        SDL_RenderFillRect(renderer, &multiBtn);
                                        if(font) renderText(renderer, font, g_CheatMultiShot?"MultiShot: ON":"MultiShot: OFF", {255,255,255,255}, multiBtn);
                                        
                                        SDL_SetRenderDrawColor(renderer, g_CheatAura?0:255, g_CheatAura?255:0, 0, 255);
                                        SDL_RenderFillRect(renderer, &auraBtn);
                                        if(font) renderText(renderer, font, g_CheatAura?"Aura: ON":"Aura: OFF", {255,255,255,255}, auraBtn);
                                        
                                        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                                        SDL_RenderFillRect(renderer, &backCheatBtn);
                                        if(font) renderText(renderer, font, "Back", {255,255,255,255}, backCheatBtn);
                                        
                                        SDL_RenderPresent(renderer);
                                    }
                                }
                                if (checkClick(backButton, sx, sy)) inSettings = false;
                            }
                        }
                        SDL_RenderCopy(renderer,background1,nullptr,nullptr);
                        
                        if(g_SoundOn) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                        else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        SDL_RenderFillRect(renderer, &soundButton);
                        if (font) renderText(renderer, font, g_SoundOn ? "Sound: ON" : "Sound: OFF", {255,255,255,255}, soundButton);
                        
                        if(g_TargetFPS == 30) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                        else if(g_TargetFPS == 60) SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                        else if(g_TargetFPS == 144) SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                        else SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                        SDL_RenderFillRect(renderer, &fpsButton);
                        
                        const char* fpsText = "FPS: 60";
                        if (g_TargetFPS == 30) fpsText = "FPS: 30";
                        else if (g_TargetFPS == 144) fpsText = "FPS: 144";
                        else if (g_TargetFPS == 0) fpsText = "FPS: Unlimited";
                        if (font) renderText(renderer, font, fpsText, {0,0,0,255}, fpsButton);
                        
                        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                        SDL_RenderFillRect(renderer, &cheatMenuBtn);
                        if (font) renderText(renderer, font, "Cheats", {255,255,255,255}, cheatMenuBtn);
                        
                        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                        SDL_RenderFillRect(renderer, &backButton);
                        if (font) renderText(renderer, font, "Back", {255,255,255,255}, backButton);
                        
                        SDL_RenderPresent(renderer);
                    }
                }
                if (checkClick(startButton, x, y))
                {
                SDL_Rect camera = {0,0,1280,720};
                GameObject player(renderer,"player/player1.png","sounds/dead-p.wav",0,0,70,70,11,10000,0,8,18,0);
                player.shortrange_weapon = {75,75,300,30,320,420,"sounds/knife1.wav",0};
                player.emplace_backtexture(renderer,"player/player2.png",8);
                player.emplace_backtexture(renderer,"player/player3.png",1);



                GameObject* bosscar = new GameObject(renderer,"things/bosscar.png","sounds/explosion.wav",0,0,103,53,0,30000,0,1,20,1);
                bosscar->setDestinationRect(100,100,200,100);
                bosscar->shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                bosscar->emplace_backtexture(renderer,"things/bosscar-a.png",7);
                bosscar->emplace_backtexture(renderer,"things/bosscar-d.png",1);

                GameObject* bosscar2 = new GameObject(renderer,"things/bosscar.png","sounds/explosion.wav",0,0,108,53,0,30000,0,1,20,1);
                bosscar2->setDestinationRect(100,300,200,100);
                bosscar2->shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                bosscar2->emplace_backtexture(renderer,"things/bosscar-a.png",7);
                bosscar2->emplace_backtexture(renderer,"things/bosscar-d.png",1);

                GameObject* bosscar3 = new GameObject(renderer,"things/bosscar.png","sounds/explosion.wav",0,0,108,53,0,30000,0,1,20,1);
                bosscar3->setDestinationRect(100,500,200,100);
                bosscar3->shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                bosscar3->emplace_backtexture(renderer,"things/bosscar-a.png",7);
                bosscar3->emplace_backtexture(renderer,"things/bosscar-d.png",1);

                GameObject* wall_1 = new GameObject(renderer,"things/wall1.png","sounds/explosion.wav",0,0,50,850,0,100000,0,1,20,1);
                wall_1->setDestinationRect(400,0,50,850);
                wall_1->shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_1->emplace_backtexture(renderer,"things/wall1-a.png",1);
                wall_1->emplace_backtexture(renderer,"things/wall1-d.png",1);

                GameObject* wall_2 = new GameObject(renderer,"things/wall1.png","sounds/explosion.wav",0,0,50,850,0,100000,0,1,20,1);
                wall_2->setDestinationRect(400,1150,50,850);
                wall_2->shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_2->emplace_backtexture(renderer,"things/wall1-a.png",1);
                wall_2->emplace_backtexture(renderer,"things/wall1-d.png",1);

                GameObject* wall_3 = new GameObject(renderer,"things/wall2.png","sounds/explosion.wav",0,0,850,50,0,100000,0,1,20,1);
                wall_3->setDestinationRect(450,0,1550,50);
                wall_3->shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_3->emplace_backtexture(renderer,"things/wall2-a.png",1);
                wall_3->emplace_backtexture(renderer,"things/wall2-d.png",1);

                GameObject* wall_4 = new GameObject(renderer,"things/wall2.png","sounds/explosion.wav",0,0,850,50,0,100000,0,1,20,1);
                wall_4->setDestinationRect(450,1950,1550,50);
                wall_4->shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_4->emplace_backtexture(renderer,"things/wall2-a.png",1);
                wall_4->emplace_backtexture(renderer,"things/wall2-d.png",1);

                GameObject* wall_5 = new GameObject(renderer,"things/wall1.png","sounds/explosion.wav",0,0,50,850,0,100000,0,1,20,1);
                wall_5->setDestinationRect(1950,50,50,1900);
                wall_5->shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_5->emplace_backtexture(renderer,"things/wall1-a.png",1);
                wall_5->emplace_backtexture(renderer,"things/wall1-d.png",1);

                player.setSourceRect(0,0,32,32);
                std::vector<GameObject*> players = {&player};
                std::vector<GameObject*> npcs;
                std::vector<GameObject*> things = {bosscar,bosscar2,bosscar3,wall_1,wall_2,wall_3,wall_4,wall_5};
                std::vector<bullet*> bullets;

                SDL_Surface* tempSurface = IMG_Load("maps/map.png");
                SDL_Texture* background = SDL_CreateTextureFromSurface(renderer, tempSurface);
                SDL_FreeSurface(tempSurface);
                int bgWidth, bgHeight;
                SDL_QueryTexture(background, NULL, NULL, &bgWidth, &bgHeight);
                running = true;
                bool inGameCheatMenu = false;
                Uint32 lastTime = SDL_GetTicks();
                float mapSpawnTimer = 0.0f;
                
                    while(running)
                    {
                    Uint32 currentTime = SDL_GetTicks();
                    float dt = (currentTime - lastTime) / 1000.0f;
                    
                    mapSpawnTimer += dt;
                    if (mapSpawnTimer > 0.5f) {
                        mapSpawnTimer = 0.0f;
                        if (rand() % 100 < 15) { // 15% chance every 0.5s
                            float angle = (rand() % 360) * M_PI / 180.0f;
                            float dist = 1500.0f + (rand() % 1000);
                            float spawnX = player.posX + cos(angle) * dist;
                            float spawnY = player.posY + sin(angle) * dist;
                            
                            int type = rand() % 3;
                            if (type == 0) { // bosscar
                                GameObject* newcar = new GameObject(renderer,"things/bosscar.png","sounds/explosion.wav",spawnX,spawnY,103,53,0,30000,0,1,20,1);
                                newcar->setDestinationRect(spawnX, spawnY, 200, 100);
                                newcar->shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                                newcar->emplace_backtexture(renderer,"things/bosscar-a.png",7);
                                newcar->emplace_backtexture(renderer,"things/bosscar-d.png",1);
                                things.push_back(newcar);
                            } else if (type == 1) { // wall1
                                GameObject* newwall = new GameObject(renderer,"things/wall1.png","sounds/explosion.wav",spawnX,spawnY,50,850,0,100000,0,1,20,1);
                                newwall->setDestinationRect(spawnX, spawnY, 50, 400 + rand() % 400);
                                newwall->emplace_backtexture(renderer,"things/wall1-a.png",1);
                                newwall->emplace_backtexture(renderer,"things/wall1-d.png",1);
                                things.push_back(newwall);
                            } else { // wall2
                                GameObject* newwall = new GameObject(renderer,"things/wall2.png","sounds/explosion.wav",spawnX,spawnY,850,50,0,100000,0,1,20,1);
                                newwall->setDestinationRect(spawnX, spawnY, 400 + rand() % 400, 50);
                                newwall->emplace_backtexture(renderer,"things/wall2-a.png",1);
                                newwall->emplace_backtexture(renderer,"things/wall2-d.png",1);
                                things.push_back(newwall);
                            }
                        }
                    }
                    
                    if (g_IsDodging) {
                        g_DodgeTimeLeft -= dt;
                        if (g_DodgeTimeLeft <= 0) g_IsDodging = false;
                        g_AfterImageSpawnTimer -= dt;
                        if (g_AfterImageSpawnTimer <= 0) {
                            g_AfterImages.push_back({player.texture, player.srcRect, player.dstRect, player.objectangle, 200.0f});
                            g_AfterImageSpawnTimer = 0.03f;
                        }
                    }
                    float effective_time_scale = g_IsDodging ? 0.05f : g_TimeScale;
                    float game_dt = dt * effective_time_scale;
                    
                    lastTime = currentTime;
                    while(SDL_PollEvent(&event))
                    {
                        if (event.type == SDL_QUIT) running = false;
                        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                            if (event.key.keysym.sym == SDLK_F1) inGameCheatMenu = !inGameCheatMenu;
                            if (event.key.keysym.sym == SDLK_f) {
                                if (g_PlayerIsDriving) {
                                    g_CurrentCar->posX = player.posX;
                                    g_CurrentCar->posY = player.posY;
                                    g_CurrentCar->dstRect.x = player.dstRect.x;
                                    g_CurrentCar->dstRect.y = player.dstRect.y;
                                    g_CurrentCar->isdead = false; // Show it again
                                    g_CurrentCar->go_through_able = false;
                                    
                                    player.texture = player.textures[0].texture;
                                    player.dstRect.w = 70;
                                    player.dstRect.h = 70;
                                    player.setSourceRect(0,0,32,32);
                                    
                                    g_PlayerIsDriving = false;
                                    g_CurrentCar = nullptr;
                                } else {
                                    for (auto t : things) {
                                        if (t->object_type == 1 && !t->isdead && t->dstRect.w == 200) {
                                            if (calculate_distance(player, *t) < 150) {
                                                g_CurrentCar = t;
                                                g_CurrentCar->isdead = true;
                                                g_CurrentCar->go_through_able = true;
                                                
                                                player.texture = g_CurrentCar->texture;
                                                player.posX = g_CurrentCar->posX;
                                                player.posY = g_CurrentCar->posY;
                                                player.dstRect.w = 200;
                                                player.dstRect.h = 100;
                                                player.setSourceRect(0,0,103,53);
                                                
                                                g_PlayerIsDriving = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            if (event.key.keysym.sym == SDLK_1) {
                                int mouseX, mouseY;
                                SDL_GetMouseState(&mouseX, &mouseY);
                                g_GojoOrbs.push_back({0, mouseX + camera.x * 1.0f, mouseY + camera.y * 1.0f, 0, 0, 50.0f, 3.0f});
                            }
                            if (event.key.keysym.sym == SDLK_2) {
                                g_GojoOrbs.push_back({1, player.posX + player.dstRect.w/2, player.posY + player.dstRect.h/2, 0, 0, 10.0f, 1.0f});
                            }
                            if (event.key.keysym.sym == SDLK_3) {
                                int mouseX, mouseY;
                                SDL_GetMouseState(&mouseX, &mouseY);
                                float dx = (mouseX + camera.x) - (player.posX + player.dstRect.w/2);
                                float dy = (mouseY + camera.y) - (player.posY + player.dstRect.h/2);
                                float len = sqrt(dx*dx + dy*dy);
                                g_GojoOrbs.push_back({2, player.posX + player.dstRect.w/2, player.posY + player.dstRect.h/2, dx/len, dy/len, 100.0f, 5.0f});
                            }
                        }
                    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                    {
                        if (!inGameCheatMenu) player.isattacking = true;
                    }
                    if (event.type == SDL_MOUSEBUTTONDOWN && inGameCheatMenu)
                    {
                        int cx = event.button.x, cy = event.button.y;
                        SDL_Rect speedBtn = {100, 50, 300, 50};
                        SDL_Rect invBtn = {100, 150, 300, 50};
                        SDL_Rect multiBtn = {100, 250, 300, 50};
                        SDL_Rect auraBtn = {100, 350, 300, 50};
                        SDL_Rect dodgeBtn = {100, 450, 300, 50};
                        SDL_Rect summonBtn = {100, 550, 300, 50};
                        SDL_Rect stopTimeBtn = {450, 50, 200, 50};
                        SDL_Rect slowTimeBtn = {450, 150, 200, 50};
                        SDL_Rect normTimeBtn = {450, 250, 200, 50};
                        SDL_Rect fastTimeBtn = {450, 350, 200, 50};
                        if (checkClick(speedBtn, cx, cy)) g_CheatSpeedBoost = !g_CheatSpeedBoost;
                        if (checkClick(invBtn, cx, cy)) g_CheatInvincible = !g_CheatInvincible;
                        if (checkClick(multiBtn, cx, cy)) g_CheatMultiShot = !g_CheatMultiShot;
                        if (checkClick(auraBtn, cx, cy)) g_CheatAura = !g_CheatAura;
                        if (checkClick(dodgeBtn, cx, cy)) g_CheatAutoDodge = !g_CheatAutoDodge;
                        if (checkClick(summonBtn, cx, cy)) g_CheatSummonMinions = !g_CheatSummonMinions;
                        if (checkClick(stopTimeBtn, cx, cy)) g_TimeScale = 0.0f;
                        if (checkClick(slowTimeBtn, cx, cy)) g_TimeScale = 0.2f;
                        if (checkClick(normTimeBtn, cx, cy)) g_TimeScale = 1.0f;
                        if (checkClick(fastTimeBtn, cx, cy)) g_TimeScale = 3.0f;
                    }
                    }

                    if(player.objecthp < 9990) player.objecthp += 10;

                    SDL_RenderClear(renderer);

                    // Tiling background
                    int startX = -(camera.x % bgWidth);
                    int startY = -(camera.y % bgHeight);
                    if (startX > 0) startX -= bgWidth;
                    if (startY > 0) startY -= bgHeight;
                    
                    for (int x = startX; x < camera.w; x += bgWidth) {
                        for (int y = startY; y < camera.h; y += bgHeight) {
                            SDL_Rect bgRect = {x, y, bgWidth, bgHeight};
                            SDL_RenderCopy(renderer, background, NULL, &bgRect);
                        }
                    }

                    std::vector<std::vector<cell>> mapgrid(40,std::vector<cell>(40));
                    initialcostfield(mapgrid,things,50);
                    mapgrid[clamp(player.dstRect.y/50,0,39)][clamp(player.dstRect.x/50,0,39)].cost = 0;
                    computeCostField(mapgrid);
                    drawbetterVector(mapgrid);

                    if(npcs.size() < 10) {
                        if(rand() % 100 < 5) spawn_enemies(npcs,renderer,0,player);
                        if(rand() % 100 < 3) spawn_enemies(npcs,renderer,1,player);
                    }
                    if (g_CheatSummonMinions) {
                        int minion_count = 0;
                        for(auto m : npcs) {
                            if (m->object_type == 2 && !m->isdead) minion_count++;
                        }
                        if (minion_count < 5) {
                            int r = rand() % 2;
                            GameObject *minion = nullptr;
                            if(r == 0) {
                                minion = new GameObject(renderer,"npcs/npc1.png","sounds/dead.wav",player.dstRect.x,player.dstRect.y,74,74,20,300,0,8,20,2);
                                minion->shortrange_weapon = {70,70,1000,30,320,500,"sounds/metal_pipe.wav",0};
                                minion->emplace_backtexture(renderer,"npcs/npc1-a.png",7);
                                minion->emplace_backtexture(renderer,"npcs/npc1-d.png",1);
                                minion->setSourceRect(0,0,32,32);
                            } else {
                                minion = new GameObject(renderer,"npcs/npc2.png","sounds/dead.wav",player.dstRect.x,player.dstRect.y,80,80,20,300,0,8,16,2);
                                minion->shortrange_weapon = {100,100,0,0,100,600,"sounds/gun_shot.wav",1};
                                minion->emplace_backtexture(renderer,"npcs/npc2-a.png",3);
                                minion->emplace_backtexture(renderer,"npcs/npc2-d.png",1);
                                minion->setSourceRect(0,0,32,32);
                            }
                            npcs.push_back(minion);
                        }
                    }

                    for(auto it = g_GojoOrbs.begin(); it != g_GojoOrbs.end(); ) {
                        it->time_left -= dt;
                        if (it->type == 0) { // Blue
                            it->radius += 20 * dt;
                            for (auto e : npcs) {
                                if (e->object_type == 0 && !e->isdead) {
                                    float dx = it->posX - (e->posX + e->dstRect.w/2);
                                    float dy = it->posY - (e->posY + e->dstRect.h/2);
                                    float dist = sqrt(dx*dx + dy*dy);
                                    if (dist > 0 && dist < 1000) {
                                        e->posX += (dx/dist) * 300 * dt;
                                        e->posY += (dy/dist) * 300 * dt;
                                        e->dstRect.x = (int)e->posX;
                                        e->dstRect.y = (int)e->posY;
                                        if (dist < it->radius) e->objecthp -= 5000 * dt;
                                    }
                                }
                            }
                        } else if (it->type == 1) { // Red
                            it->radius += 1500 * dt;
                            for (auto e : npcs) {
                                if (e->object_type == 0 && !e->isdead) {
                                    float dx = (e->posX + e->dstRect.w/2) - it->posX;
                                    float dy = (e->posY + e->dstRect.h/2) - it->posY;
                                    float dist = sqrt(dx*dx + dy*dy);
                                    if (dist > 0 && dist < it->radius && dist > it->radius - 300) {
                                        e->posX += (dx/dist) * 2000 * dt;
                                        e->posY += (dy/dist) * 2000 * dt;
                                        e->dstRect.x = (int)e->posX;
                                        e->dstRect.y = (int)e->posY;
                                        e->objecthp -= 5000 * dt;
                                    }
                                }
                            }
                        } else if (it->type == 2) { // Purple
                            it->posX += it->dirX * 800 * dt;
                            it->posY += it->dirY * 800 * dt;
                            for (auto e : npcs) {
                                if (e->object_type == 0 && !e->isdead) {
                                    float dx = (e->posX + e->dstRect.w/2) - it->posX;
                                    float dy = (e->posY + e->dstRect.h/2) - it->posY;
                                    if (sqrt(dx*dx + dy*dy) < it->radius) e->objecthp -= 100000 * dt;
                                }
                            }
                            for (auto t : things) {
                                if (!t->isdead && t != g_CurrentCar) {
                                    float dx = (t->posX + t->dstRect.w/2) - it->posX;
                                    float dy = (t->posY + t->dstRect.h/2) - it->posY;
                                    if (sqrt(dx*dx + dy*dy) < it->radius) t->objecthp -= 100000 * dt;
                                }
                            }
                            for (auto b : bullets) {
                                if (!b->isdead) {
                                    float dx = b->posX - it->posX;
                                    float dy = b->posY - it->posY;
                                    if (sqrt(dx*dx + dy*dy) < it->radius) b->isdead = true;
                                }
                            }
                        }
                        if (it->time_left <= 0) it = g_GojoOrbs.erase(it);
                        else ++it;
                    }

                    the_ultimate_movement_and_status_handler_for_player(renderer,player,camera,things,npcs,bullets, dt);
                    greater_check_if_died(npcs,player);
                    the_ultimate_movement_and_status_handler(renderer,npcs,mapgrid,50,things,player,bullets, game_dt);
                    the_ultimate_status_handler_for_things(things,npcs,player, game_dt);

                    handle_movement_of_bullet(bullets, game_dt);
                    handle_status_and_delete_dead_bullets(bullets,2000,2000);
                    handle_collision_of_bullet(bullets,npcs);
                    handle_collision_of_bullet(bullets,things);
                    handle_collision_of_bullet(bullets,players, true);

                    if (!g_PlayerIsDriving) the_ultimate_animation_handler(player,20);
                    if (!g_PlayerIsDriving) the_more_ultimate_animation_handler(players);
                    the_more_ultimate_animation_handler(npcs);
                    the_more_ultimate_animation_handler(things);


                    the_ultimate_sound_effects_handler_for_single_object(player);
                    the_ultimate_sound_effects_handler(npcs);
                    the_ultimate_sound_effects_handler(things);



                    rendercopytocamera(renderer,camera,things);
                    rendercopytocamera_for_dead(renderer,camera,players);
                    rendercopytocamera_for_dead(renderer,camera,npcs);
                    rendercopytocamera_for_bullet(renderer,camera,bullets);
                    
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    for(auto it = g_AfterImages.begin(); it != g_AfterImages.end(); ) {
                        it->alpha -= 500 * dt;
                        if(it->alpha <= 0) {
                            it = g_AfterImages.erase(it);
                        } else {
                            SDL_Rect camRect = {it->dst.x - camera.x, it->dst.y - camera.y, it->dst.w, it->dst.h};
                            SDL_SetTextureColorMod(it->tex, 0, 255, 255);
                            SDL_SetTextureAlphaMod(it->tex, (int)it->alpha);
                            SDL_RenderCopyEx(renderer, it->tex, &it->src, &camRect, it->angle, nullptr, SDL_FLIP_NONE);
                            SDL_SetTextureAlphaMod(it->tex, 255);
                            SDL_SetTextureColorMod(it->tex, 255, 255, 255);
                            ++it;
                        }
                    }

                    for (auto& orb : g_GojoOrbs) {
                        int cx = orb.posX - camera.x;
                        int cy = orb.posY - camera.y;
                        if (orb.type == 0) { // Blue
                            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
                            for (int r = 1; r < orb.radius; r += 2) drawCircle(renderer, cx, cy, r);
                            SDL_SetRenderDrawColor(renderer, 0, 100, 255, 200);
                            for (int r = orb.radius-5; r <= orb.radius; r++) drawCircle(renderer, cx, cy, r);
                        } else if (orb.type == 1) { // Red
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);
                            for (int r = orb.radius; r > std::max(1.0f, orb.radius - 20); r -= 4) drawCircle(renderer, cx, cy, r);
                        } else if (orb.type == 2) { // Purple
                            SDL_SetRenderDrawColor(renderer, 128, 0, 128, 150);
                            for (int r = 1; r < orb.radius; r += 2) drawCircle(renderer, cx, cy, r);
                            SDL_SetRenderDrawColor(renderer, 200, 0, 255, 255);
                            drawCircle(renderer, cx, cy, orb.radius);
                        }
                    }
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

                    rendercopytocamera_for_alive(renderer,camera,players);
                    rendercopytocamera_for_alive(renderer,camera,npcs);
                    
                    if (g_CheatAura) {
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
                        int cx = player.dstRect.x + player.dstRect.w/2 - camera.x;
                        int cy = player.dstRect.y + player.dstRect.h/2 - camera.y;
                        for (int r = 140; r <= 150; r++) drawCircle(renderer, cx, cy, r);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                    }
                    
                    updateCamera(camera,player,1280,720,bgWidth,bgHeight);

                    if (inGameCheatMenu) {
                        SDL_Rect overlay = {0,0,1280,720};
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
                        SDL_RenderFillRect(renderer, &overlay);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                        
                        SDL_Rect speedBtn = {100, 50, 300, 50};
                        SDL_Rect invBtn = {100, 150, 300, 50};
                        SDL_Rect multiBtn = {100, 250, 300, 50};
                        SDL_Rect auraBtn = {100, 350, 300, 50};
                        SDL_Rect dodgeBtn = {100, 450, 300, 50};
                        SDL_Rect summonBtn = {100, 550, 300, 50};
                        SDL_Rect stopTimeBtn = {450, 50, 200, 50};
                        SDL_Rect slowTimeBtn = {450, 150, 200, 50};
                        SDL_Rect normTimeBtn = {450, 250, 200, 50};
                        SDL_Rect fastTimeBtn = {450, 350, 200, 50};
                        
                        SDL_SetRenderDrawColor(renderer, g_CheatSpeedBoost?0:255, g_CheatSpeedBoost?255:0, 0, 255);
                        SDL_RenderFillRect(renderer, &speedBtn);
                        if(font) renderText(renderer, font, g_CheatSpeedBoost?"Speed: x3":"Speed: x1", {255,255,255,255}, speedBtn);
                        
                        SDL_SetRenderDrawColor(renderer, g_CheatInvincible?0:255, g_CheatInvincible?255:0, 0, 255);
                        SDL_RenderFillRect(renderer, &invBtn);
                        if(font) renderText(renderer, font, g_CheatInvincible?"God Mode: ON":"God Mode: OFF", {255,255,255,255}, invBtn);
                        
                        SDL_SetRenderDrawColor(renderer, g_CheatMultiShot?0:255, g_CheatMultiShot?255:0, 0, 255);
                        SDL_RenderFillRect(renderer, &multiBtn);
                        if(font) renderText(renderer, font, g_CheatMultiShot?"MultiShot: ON":"MultiShot: OFF", {255,255,255,255}, multiBtn);
                        
                        SDL_SetRenderDrawColor(renderer, g_CheatAura?0:255, g_CheatAura?255:0, 0, 255);
                        SDL_RenderFillRect(renderer, &auraBtn);
                        if(font) renderText(renderer, font, g_CheatAura?"Aura: ON":"Aura: OFF", {255,255,255,255}, auraBtn);

                        SDL_SetRenderDrawColor(renderer, g_CheatAutoDodge?0:255, g_CheatAutoDodge?255:0, 0, 255);
                        SDL_RenderFillRect(renderer, &dodgeBtn);
                        if(font) renderText(renderer, font, g_CheatAutoDodge?"Auto Dodge: ON":"Auto Dodge: OFF", {255,255,255,255}, dodgeBtn);

                        SDL_SetRenderDrawColor(renderer, g_CheatSummonMinions?0:255, g_CheatSummonMinions?255:0, 0, 255);
                        SDL_RenderFillRect(renderer, &summonBtn);
                        if(font) renderText(renderer, font, g_CheatSummonMinions?"Summon Minions: ON":"Summon Minions: OFF", {255,255,255,255}, summonBtn);

                        SDL_SetRenderDrawColor(renderer, g_TimeScale==0.0f?0:100, g_TimeScale==0.0f?255:100, 100, 255);
                        SDL_RenderFillRect(renderer, &stopTimeBtn);
                        if(font) renderText(renderer, font, "Stop Time", {255,255,255,255}, stopTimeBtn);
                        
                        SDL_SetRenderDrawColor(renderer, g_TimeScale==0.2f?0:100, g_TimeScale==0.2f?255:100, 100, 255);
                        SDL_RenderFillRect(renderer, &slowTimeBtn);
                        if(font) renderText(renderer, font, "Slow Time", {255,255,255,255}, slowTimeBtn);
                        
                        SDL_SetRenderDrawColor(renderer, g_TimeScale==1.0f?0:100, g_TimeScale==1.0f?255:100, 100, 255);
                        SDL_RenderFillRect(renderer, &normTimeBtn);
                        if(font) renderText(renderer, font, "Normal Time", {255,255,255,255}, normTimeBtn);
                        
                        SDL_SetRenderDrawColor(renderer, g_TimeScale==3.0f?0:100, g_TimeScale==3.0f?255:100, 100, 255);
                        SDL_RenderFillRect(renderer, &fastTimeBtn);
                        if(font) renderText(renderer, font, "Fast Time", {255,255,255,255}, fastTimeBtn);
                    }

                    SDL_RenderPresent(renderer);








                    greater_check_if_died(players,player);
                    greater_check_if_died(things,player);
                    if(count_dead_things(npcs) >= 6) ObjectKiller(npcs);
                    if (g_TargetFPS > 0) {
                        Uint32 targetTime = 1000 / g_TargetFPS;
                        Uint32 frameTime = SDL_GetTicks() - currentTime;
                        if (frameTime < targetTime) {
                            SDL_Delay(targetTime - frameTime);
                        }
                    }
                    }
                    greater_check_if_died(npcs,player);
                    ObjectKiller(things);
                    ObjectKiller(npcs);

                }

                if (checkClick(quitButton, x, y)) running1 = false;
            }
        }
    }
    if (font) TTF_CloseFont(font);
    EndEverything(window,renderer);

}






//#include "raylib.h"

#define arrsize(a) (sizeof(a)/sizeof(a[0]))
#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))
#define toggle(a) (!(a)? (1) : (0))

#define MIN_WIDTH 320
#define MIN_HEIGHT 240

#define MAX_LANG 2

#define SPLASH_FADE_OUT_SPEED 2

//#define MAX_MAPS 6
#define MAX_WALLS 35
#define MAX_DOORS 5
#define PLAYER_SPEED 200

#define MENU_POS_X 50
#define MENU_POS_Y 110
#define MENU_GAP_X 350
const Rectangle MENU_INFO_REC = (Rectangle){50,400,750,200};

// save game
typedef enum {
    STORAGE_POSITION_PLAYED             = 0,
    STORAGE_POSITION_KEY1               = 1,
    STORAGE_POSITION_KEY2               = 2,
    STORAGE_POSITION_KEY3               = 3,
    STORAGE_POSITION_DOOR1              = 4,
    STORAGE_POSITION_DOOR2              = 5,
    STORAGE_POSITION_DOOR3              = 6,
    STORAGE_POSITION_CAPTURED_PIGS      = 7,
    STORAGE_POSITION_THE_END            = 8,
    STORAGE_POSITION_KILLED_SPIDERS     = 9,
    STORAGE_POSITION_DELIVERED_PIE      = 10
} StorageData;

// Global delta, cause I like it better this way (and some other global vars)
float delta;
Font defaultFont;
Vector2 mouse = { 0 };
const Color MORFYGREEN = (Color){0,255,75,255};
const Color GUI_BACKGROUND_COLOR = (Color){150,150,150,255};
const Color GUI_LINE_COLOR = BLACK;
const Color GUI_HOVER_COLOR = (Color){200,200,200,255};
const Color GUI_DOWN_COLOR = (Color){125,125,125,255};
const Color GUI_ACTIVE_COLOR = MORFYGREEN;

// enums
enum enum_language {L_ENGLISH=0, L_GERMAN};
enum enum_scale_type {ST_STRETCHING=0, ST_KEEPING};
enum enum_project_state {PS_SPLASHSCREEN=0, PS_GAME, PS_MENU, PS_TRANSITION};
enum enum_menu_state {MS_MAIN=0, MS_SETTINGS, MS_CONTROLS, MS_COUNT};
typedef enum {ROOM_FOREST=0, ROOM_CAMP, ROOM_BUSHES, ROOM_GRAVEYARD, ROOM_DUNGEON, ROOM_CANYONS} EnumRoom;

typedef enum {TEXTURE_RAYLIB=0, TEXTURE_TILES, TEXTURE_PLAYER, TEXTURE_DITTO_PANTS, TEXTURE_NPC1, TEXTURE_NPC2, TEXTURE_NPC3, TEXTURE_MISC, MAX_TEXTURE_COUNT} EnumTexture;
typedef enum {MUSIC_WURFELSPIEL=0, MUSIC_FOREST, MUSIC_DUNGEON, MUSIC_GRAVEYARD, MUSIC_BABY_CRYING, MUSIC_BABY_LAUGHING, MAX_MUSIC_COUNT} EnumMusic;
typedef enum {SOUND_SELECT=0, SOUND_ERROR, SOUND_PUNCH1, SOUND_PUNCH2, SOUND_PUNCH3, SOUND_SAVE, SOUND_KEY, SOUND_DOOR, SOUND_WHIP, SOUND_EXPLOSION, SOUND_MONSTER_LAUGH, SOUND_MONSTER_OUCH, SOUND_PIG, SOUND_CHEW,
MAX_SOUND_COUNT} EnumSound;

const Rectangle WALL_PINETREES = (Rectangle){0,0,100,100};
const Rectangle WALL_STATUE = (Rectangle){100,0,100,100};
const Rectangle WALL_APPLETREES = (Rectangle){200,0,100,100};
const Rectangle WALL_DEADTREES = (Rectangle){300,0,100,100};
const Rectangle WALL_GRAVE = (Rectangle){400,0,100,100};
const Rectangle WALL_DUNGEON = (Rectangle){500,0,100,100};
const Rectangle WALL_FENCE_TOPLEFT = (Rectangle){0,100,100,100};
const Rectangle WALL_FENCE_TOPRIGHT = (Rectangle){100,100,100,100};
const Rectangle WALL_FENCE_BOTTOMLEFT = (Rectangle){0,200,100,100};
const Rectangle WALL_FENCE_BOTTOMRIGHT = (Rectangle){100,200,100,100};
const Rectangle WALL_FENCE_HORZ = (Rectangle){200,100,100,100};
const Rectangle WALL_FENCE_VERT = (Rectangle){200,200,100,100};
const Rectangle WALL_EMPTY = (Rectangle){300,100,100,100};

const Rectangle WHIP_RECT = (Rectangle){0,0,100,100};

int keyUp = KEY_UP;
int keyDown = KEY_DOWN;
int keyRight = KEY_RIGHT;
int keyLeft = KEY_LEFT;
int keyPrimary = KEY_E;
int keySecondary = KEY_W;
int padUp = GAMEPAD_BUTTON_LEFT_FACE_UP;
int padDown = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
int padRight = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
int padLeft = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
int padPrimary = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
int padSecondary = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
int keyEscape = KEY_ESCAPE;
int padEscape = GAMEPAD_BUTTON_MIDDLE_LEFT;
int IsAnyKeyPressed() {
    if (GetKeyPressed()>=1 || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 1;
    else return 0;
}
int InputMoveRight() {
    if (IsKeyDown(keyRight)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonDown(GAMEPAD_PLAYER1, padRight)) return 1;
        else return 0;
    } else return 0;
}int InputMoveLeft() {
    if (IsKeyDown(keyLeft)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonDown(GAMEPAD_PLAYER1, padLeft)) return 1;
        else return 0;
    } else return 0;
}int InputMoveUp() {
    if (IsKeyDown(keyUp)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonDown(GAMEPAD_PLAYER1, padUp)) return 1;
        else return 0;
    } else return 0;
}int InputMoveDown() {
    if (IsKeyDown(keyDown)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonDown(GAMEPAD_PLAYER1, padDown)) return 1;
        else return 0;
    } else return 0;
}
int InputMoveRightPressed() {
    if (IsKeyPressed(keyRight)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padRight)) return 1;
        else return 0;
    } else return 0;
}int InputMoveLeftPressed() {
    if (IsKeyPressed(keyLeft)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padLeft)) return 1;
        else return 0;
    } else return 0;
}int InputMoveUpPressed() {
    if (IsKeyPressed(keyUp)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padUp)) return 1;
        else return 0;
    } else return 0;
}int InputMoveDownPressed() {
    if (IsKeyPressed(keyDown)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padDown)) return 1;
        else return 0;
    } else return 0;
}
int InputPrimary() {
    if (IsKeyPressed(keyPrimary)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padPrimary)) return 1;
        else return 0;
    } else return 0;
}int InputSecondary() {
    if (IsKeyPressed(keySecondary)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padSecondary)) return 1;
        else return 0;
    } else return 0;
}int InputEscape() {
    if (IsKeyPressed(keyEscape)) return 1;
    else if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, padEscape)) return 1;
        else return 0;
    } else return 0;
}

const char *CodepointToString(int key, enum enum_language lang, int *byteLength) {
    if (key >= 33 && key <= 126) return CodepointToUtf8(key, byteLength);
    else if (key == 32) {
        if (lang == L_GERMAN) return "Leertaste";
        else return "Space";
    } else if (key == 262) {
        if (lang == L_GERMAN) return "Pfeil-rechts";
        else return "Right arrow";
    } else if (key == 263) {
        if (lang == L_GERMAN) return "Pfeil-links";
        else return "Left arrow";
    } else if (key == 264) {
        if (lang == L_GERMAN) return "Pfeil-unten";
        else return "Down arrow";
    } else if (key == 265) {
        if (lang == L_GERMAN) return "Pfeil-oben";
        else return "Up arrow";
    } else return "N/A";
}
int CodepointIsUseable(int key) {
    if (key == 32) return 1;
    else if (key >= 65 && key <= 90) return 1;
    else if (key >= 97 && key <= 122) return 1;
    else if (key >= 262 && key <= 265) return 1;
    else if (key == KEY_RIGHT) return 1;
    else return 0;
}

int CharToInteger(char c) {
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1;
    }
}
float ClampFloat(float value, float min, float max) {
    if (value > max) value = max;
    else if (value < min) value = min;
    return value;
}
int ClampInteger(int value, int min, int max) {
    if (value > max) value = max;
    else if (value < min) value = min;
    return value;
}
void DrawTextShadow(const char* text, int posX, int posY, int shadowDistanceX, int shadowDistanceY, int fontSize, Color color, Color shadowColor) {
    DrawTextEx(defaultFont, text, (Vector2){posX+shadowDistanceX, posY+shadowDistanceY}, fontSize, 0, shadowColor);
    DrawTextEx(defaultFont, text, (Vector2){posX, posY}, fontSize, 0, color);
}
void DrawTextShadowEx(Font font, const char* text, Vector2 pos, Vector2 shadowDistance, int fontSize, int spacing, Color color, Color shadowColor) {
    DrawTextEx(font, text, (Vector2){pos.x + shadowDistance.x, pos.y + shadowDistance.y}, fontSize, spacing, shadowColor);
    DrawTextEx(font, text, pos, fontSize, spacing, color);
}
float VecDistance(Vector2 a, Vector2 b) {
    return sqrt( pow(b.x-a.x,2) + pow(b.y-a.y,2) ) ;
}
const char* BoolToString(int boolean, enum enum_language lang) {
    if (boolean) {
        switch (lang) {
            case L_ENGLISH: return "ON";
            case L_GERMAN: return "AN";
            default: return "ON";
        }
    } else {
        switch (lang) {
            case L_ENGLISH: return "OFF";
            case L_GERMAN: return "AUS";
            default: return "OFF";
        }
    }
}
void ProcessCamera(Camera2D * cam, Vector2 pos, Rectangle boundary) {
    cam->target.x = ClampFloat(pos.x, boundary.x+cam->offset.x, boundary.x+boundary.width-cam->offset.x);
    cam->target.y = ClampFloat(pos.y, boundary.y+cam->offset.y, boundary.y+boundary.height-cam->offset.y);
}
Rectangle PointToSquare(Vector2 position, float size) {
    return (Rectangle){position.x - size*0.5, position.y - size*0.5, size, size};
}




typedef enum {DRAWTYPE_TILED=0, DRAWTYPE_EDGES} EnumDrawType;
void DrawWall(Texture2D texture, Rectangle wall, Rectangle sourceRec, int drawType, Camera2D cam, int gameX, int gameY) {
    switch (drawType) {
        case DRAWTYPE_TILED:
            {
            float restX = (wall.width - (int)(wall.width/sourceRec.width)*sourceRec.width)/((int)(wall.width/sourceRec.width));
            float restY = (wall.height - (int)(wall.height/sourceRec.height)*sourceRec.height)/((int)(wall.height/sourceRec.height));
            Rectangle camRect = (Rectangle){ cam.target.x-cam.offset.x, cam.target.y-cam.offset.y, gameX, gameY };
            for ( int x=0; x < (int)(wall.width/sourceRec.width); x++ ) {
                for ( int y=0; y < (int)(wall.height/sourceRec.height); y++ ) {
                    Rectangle rect = (Rectangle){wall.x + x*sourceRec.width + x*restX, wall.y + y*sourceRec.height + y*restY, sourceRec.width+restX, sourceRec.height+restY};
                    if (CheckCollisionRecs(camRect, rect )) {
                        DrawTexturePro(texture, sourceRec, rect, (Vector2){0,0}, 0, WHITE);
                    }
                }
            }
            }
        default: break;
    }
}
#define MAX_ENEMIES 20
struct Room {
    int id;
    Color background;
    Color fog;
    Rectangle boundary;
    int wallCount;
    Rectangle walls[MAX_WALLS];
    Rectangle wallType[MAX_WALLS];
    int doorCount;
    Rectangle doors[MAX_DOORS];
    int doorToRoom[MAX_DOORS];
    Vector2 doorToPos[MAX_DOORS];
    int enemyCount;
    float enemySize;
    Vector2 enemyPos[MAX_ENEMIES];
    int enemyType[MAX_ENEMIES];
    float enemyFloat;
    Vector2 enemyVel[MAX_ENEMIES];
    float enemyStun[MAX_ENEMIES];
    int pigChecker[4];
    int capturedPigs;
    int hasQuest;
    Vector2 questPos;
    float questSize;
    int quest;
    Rectangle questBox;
    float questFloat;
    int localKey;
    Vector2 keyPos;
    int hasKey[3];
    int openedDoor[3];
    int monsterHealth;
    int spiderAlive[10];
    int killedSpiders;
    int music;
    int delivered;
};
void MoveAndSlide(Vector2 * pos, float size, Vector2 vel, struct Room r, int withWalls, int withDoors, int withEnemies, int withQuest) {
    pos->x += vel.x*delta;
    pos->y += vel.y*delta;
    if (withWalls) {
        for (int i=0; i<r.wallCount; i++) {
            if (CheckCollisionRecs(r.walls[i], PointToSquare(*pos, size))) {
                Rectangle rect = GetCollisionRec(r.walls[i], PointToSquare(*pos, size));
                if (rect.width > rect.height) {
                    if (rect.y > pos->y) pos->y = rect.y - (size*0.5);
                    else pos->y = rect.y + rect.height + (size*0.5);
                } else {
                    if (rect.x > pos->x) pos->x = rect.x - (size*0.5);
                    else pos->x = rect.x + rect.width + (size*0.5);
                }
            }
        }
    }
    if (withDoors) {
        for (int i=0; i<r.doorCount; i++) {
            if (CheckCollisionRecs(r.doors[i], PointToSquare(*pos, size))) {
                Rectangle rect = GetCollisionRec(r.doors[i], PointToSquare(*pos, size));
                if (rect.width > rect.height) {
                    if (rect.y > pos->y) pos->y = rect.y - (size*0.5);
                    else pos->y = rect.y + rect.height + (size*0.5);
                } else {
                    if (rect.x > pos->x) pos->x = rect.x - (size*0.5);
                    else pos->x = rect.x + rect.width + (size*0.5);
                }
            }
        }
    }
    if (withEnemies) {
        for (int i=0; i<r.enemyCount; i++) {
            if (i != withEnemies) {
                if (CheckCollisionRecs(PointToSquare(r.enemyPos[i],r.enemySize), PointToSquare(*pos, size))) {
                    Rectangle rect = GetCollisionRec(PointToSquare(r.enemyPos[i],r.enemySize), PointToSquare(*pos, size));
                    if (rect.width > rect.height) {
                        if (rect.y > pos->y) pos->y = rect.y - (size*0.5);
                        else pos->y = rect.y + rect.height + (size*0.5);
                    } else {
                        if (rect.x > pos->x) pos->x = rect.x - (size*0.5);
                        else pos->x = rect.x + rect.width + (size*0.5);
                    }
                }
            }
        }
    }
    if (withQuest) {
        if (r.hasQuest) {
            if (CheckCollisionRecs(PointToSquare(r.questPos,r.questSize), PointToSquare(*pos, size))) {
                Rectangle rect = GetCollisionRec(PointToSquare(r.questPos,r.questSize), PointToSquare(*pos, size));
                if (rect.width > rect.height) {
                    if (rect.y > pos->y) pos->y = rect.y - (size*0.5);
                    else pos->y = rect.y + rect.height + (size*0.5);
                } else {
                    if (rect.x > pos->x) pos->x = rect.x - (size*0.5);
                    else pos->x = rect.x + rect.width + (size*0.5);
                }
            }
        }
    }
}
#define PUNCH_SIZE 40
#define PUNCH_DURATION 0.5
#define PUNCH_COLLISION_DURATION 0.1
#define PUNCH_SPEED 500
#define PUNCH_DECEL 800
#define WHIP_SPEED 800
void UpdateEnemies(struct Room * r, Vector2 playerPos, Vector2 punch, int punching, Rectangle whip, int whipping, Camera2D cam, int gameX, int gameY) {
    r->enemyFloat -= delta;
    for (int i=0; i < r->enemyCount; i++) {
        switch (r->enemyType[i]) {
            case 0:
                if (i > 1 ) {
                    //ectangle camRect = ;
                    if (CheckCollisionRecs(PointToSquare(r->enemyPos[i], r->enemySize), (Rectangle){ cam.target.x-cam.offset.x, cam.target.y-cam.offset.y, gameX, gameY })) {
                        if (CheckCollisionPointRec(r->enemyPos[i],(Rectangle){1600,200,600,400})) r->pigChecker[i-2] = 1;
                        else r->pigChecker[i-2] = 0;
                        if (r->enemyStun[i]>0) {
                            r->enemyStun[i] -= delta;
                            if (r->enemyVel[i].x > 10) r->enemyVel[i].x -= delta * PUNCH_DECEL;
                            else if (r->enemyVel[i].x < -10) r->enemyVel[i].x += delta * PUNCH_DECEL;
                            else r->enemyVel[i].x = 0;
                            if (r->enemyVel[i].y > 10) r->enemyVel[i].y -= delta * PUNCH_DECEL;
                            else if (r->enemyVel[i].y < -10) r->enemyVel[i].y += delta * PUNCH_DECEL;
                            else r->enemyVel[i].y = 0;
                        } else {
                            if (punching && CheckCollisionCircleRec(punch,PUNCH_SIZE, PointToSquare(r->enemyPos[i], r->enemySize))) {
                                /*if (IsKeyPressed(keyPrimary)) */
                                r->enemyStun[i] = 0.5;
                                Vector2 diff = (Vector2){sqrt(pow(playerPos.x-r->enemyPos[i].x,2)), sqrt(pow(playerPos.y-r->enemyPos[i].y,2))};
                                if (diff.x > diff.y) {
                                    if (r->enemyPos[i].x < playerPos.x) r->enemyVel[i].x = -PUNCH_SPEED;
                                    else r->enemyVel[i].x = PUNCH_SPEED;
                                } else {
                                    if (r->enemyPos[i].y < playerPos.y) r->enemyVel[i].y = -PUNCH_SPEED;
                                    else r->enemyVel[i].y = PUNCH_SPEED;
                                }
                            } else if (whipping && CheckCollisionRecs(whip,PointToSquare(r->enemyPos[i], r->enemySize))) {
                                r->enemyStun[i] = 0.5;
                                Vector2 diff = (Vector2){sqrt(pow(playerPos.x-r->enemyPos[i].x,2)), sqrt(pow(playerPos.y-r->enemyPos[i].y,2))};
                                if (diff.x > diff.y) {
                                    if (r->enemyPos[i].x < playerPos.x) r->enemyVel[i].x = -WHIP_SPEED;
                                    else r->enemyVel[i].x = WHIP_SPEED;
                                } else {
                                    if (r->enemyPos[i].y < playerPos.y) r->enemyVel[i].y = -WHIP_SPEED;
                                    else r->enemyVel[i].y = WHIP_SPEED;
                                }
                            }
                            if (r->enemyFloat <= 0) {
                                if (GetRandomValue(0,1)) {
                                    r->enemyVel[i].x = (float)GetRandomValue(-1,1)*128;
                                    r->enemyVel[i].y = 0;
                                } else {
                                    r->enemyVel[i].x = 0;
                                    r->enemyVel[i].y = (float)GetRandomValue(-1,1)*128;
                                }
                                //if () r->enemyVel[i].y = 0;
                            }
                        }
                        MoveAndSlide( &(r->enemyPos[i]), r->enemySize, r->enemyVel[i], *r, 1, 1, i, 1);
                    }
                }
                break;
            case 1:
                if (i > 1) {
                    //ectangle camRect = ;
                    if (r->monsterHealth  > 0) {
                        if (CheckCollisionRecs(PointToSquare(r->enemyPos[i], r->enemySize), (Rectangle){ cam.target.x-cam.offset.x, cam.target.y-cam.offset.y, gameX, gameY })) {
                            //if (r->enemyStun[i] <= 0.0) {
                            //}
                            if (r->enemyStun[i]>0.0) {
                                r->enemyStun[i] -= delta;
                                if (r->enemyVel[i].x > 10) r->enemyVel[i].x -= delta * PUNCH_DECEL;
                                else if (r->enemyVel[i].x < -10) r->enemyVel[i].x += delta * PUNCH_DECEL;
                                else r->enemyVel[i].x = 0;
                                if (r->enemyVel[i].y > 10) r->enemyVel[i].y -= delta * PUNCH_DECEL;
                                else if (r->enemyVel[i].y < -10) r->enemyVel[i].y += delta * PUNCH_DECEL;
                                else r->enemyVel[i].y = 0;
                            } else {
                                if (whipping && CheckCollisionRecs(whip,PointToSquare(r->enemyPos[i], r->enemySize))) {
                                    r->enemyStun[i] = 0.25;
                                    Vector2 diff = (Vector2){sqrt(pow(playerPos.x-r->enemyPos[i].x,2)), sqrt(pow(playerPos.y-r->enemyPos[i].y,2))};
                                    if (diff.x > diff.y) {
                                        if (r->enemyPos[i].x < playerPos.x) r->enemyVel[i].x = -1000;
                                        else r->enemyVel[i].x = 1000;
                                    } else {
                                        if (r->enemyPos[i].y < playerPos.y) r->enemyVel[i].y = -1000;
                                        else r->enemyVel[i].y = 1000;
                                    }
                                    r->monsterHealth--;
                                    if (r->monsterHealth <= 0) {
                                        r->keyPos = r->enemyPos[i];
                                        if (r->enemyPos[i].x >= 0) r->enemyPos[i].x = -100;
                                        if (r->enemyPos[i].y >= 0) r->enemyPos[i].y = -100;
                                    }
                                }
                                if (r->enemyFloat <= 0) {
                                    if (GetRandomValue(0,1)) {
                                        r->enemyVel[i].x = 300;
                                        if (GetRandomValue(0,1)) {
                                            r->enemyVel[i].x *= -1;
                                        }
                                        r->enemyVel[i].y = 0;
                                    } else {
                                        r->enemyVel[i].x = 0;
                                        r->enemyVel[i].y = 300;
                                        if (GetRandomValue(0,1)) {
                                            r->enemyVel[i].y *= -1;
                                        }
                                    }
                                    /*if (r->enemyVel[i].x >= -1 && r->enemyVel[i].x <= 1)  {
                                        r->enemyVel[i].x = 512;
                                        if (GetRandomValue(0,1)) r->enemyVel[i].x *= -1;
                                    } else r->enemyVel[i].x = 0;
                                    if (r->enemyVel[i].y >= -1 && r->enemyVel[i].y <= 1)  {
                                        r->enemyVel[i].y = 512;
                                        if (GetRandomValue(0,1)) r->enemyVel[i].y *= -1;
                                    } else r->enemyVel[i].y = 0;*/
                                }
                            }
                            MoveAndSlide( &(r->enemyPos[i]), r->enemySize, r->enemyVel[i], *r, 1, 1, i, 1);
                        }
                    }/* else {
                        if (r->enemyPos[i].x >= 0) r->enemyPos[i].x = -100;
                        if (r->enemyPos[i].y >= 0) r->enemyPos[i].y = -100;
                    }*/
                }
                break;
            case 2:
                if (i > 1 ) {
                    //ectangle camRect = ;
                    if (r->spiderAlive[i-2]) {
                        if (CheckCollisionRecs(PointToSquare(r->enemyPos[i], r->enemySize), (Rectangle){ cam.target.x-cam.offset.x, cam.target.y-cam.offset.y, gameX, gameY })) {
                            if (r->enemyStun[i] <= 0.0) {
                                if (punching && CheckCollisionCircleRec(punch,PUNCH_SIZE, PointToSquare(r->enemyPos[i], r->enemySize))) {
                                    /*if (IsKeyPressed(keyPrimary)) */
                                    r->enemyStun[i] = 0.5;
                                    Vector2 diff = (Vector2){sqrt(pow(playerPos.x-r->enemyPos[i].x,2)), sqrt(pow(playerPos.y-r->enemyPos[i].y,2))};
                                    if (diff.x > diff.y) {
                                        if (r->enemyPos[i].x < playerPos.x) r->enemyVel[i].x = -PUNCH_SPEED;
                                        else r->enemyVel[i].x = PUNCH_SPEED;
                                    } else {
                                        if (r->enemyPos[i].y < playerPos.y) r->enemyVel[i].y = -PUNCH_SPEED;
                                        else r->enemyVel[i].y = PUNCH_SPEED;
                                    }
                                } else if (whipping && CheckCollisionRecs(whip,PointToSquare(r->enemyPos[i], r->enemySize))) {
                                    r->spiderAlive[i-2] = 0;
                                    r->killedSpiders++;
                                }
                            }
                            if (r->enemyStun[i]>=0) {
                                r->enemyStun[i] -= delta;
                                if (r->enemyVel[i].x > 10) r->enemyVel[i].x -= delta * PUNCH_DECEL;
                                else if (r->enemyVel[i].x < -10) r->enemyVel[i].x += delta * PUNCH_DECEL;
                                else r->enemyVel[i].x = 0;
                                if (r->enemyVel[i].y > 10) r->enemyVel[i].y -= delta * PUNCH_DECEL;
                                else if (r->enemyVel[i].y < -10) r->enemyVel[i].y += delta * PUNCH_DECEL;
                                else r->enemyVel[i].y = 0;
                            } else {
                                if (r->enemyFloat <= 0) {
                                    if (GetRandomValue(0,1)) {
                                        r->enemyVel[i].x = (float)GetRandomValue(-256,256);
                                        r->enemyVel[i].y = 0;
                                    } else {
                                        r->enemyVel[i].x = 0;
                                        r->enemyVel[i].y = (float)GetRandomValue(-256,256);
                                    }
                                    //if () r->enemyVel[i].y = 0;
                                }
                            }
                            MoveAndSlide( &(r->enemyPos[i]), r->enemySize, r->enemyVel[i], *r, 1, 1, i, 1);
                        }
                    }
                }
                break;
            default: break;
        }
    }
    if (r->enemyFloat <= 0) r->enemyFloat = 1.0;
    //}
}
const Color KEY_COLOR[3] = {(Color){100,100,200,255}, (Color){255,100,100,255}, (Color){255,255,0,255}};
const int DOOR_AT_WALL[3] = {28,8,29};
void ChangeRoom(struct Room * r, int toRoom) {
    r->id = toRoom;
    switch (toRoom) {
        case ROOM_FOREST:
            r->music = MUSIC_FOREST;
            r->background = (Color){80,160,64,255};
            r->fog = (Color){0,0,0,0};
            r->localKey = -1;
            r->doorCount = 4;
            r->doors[0] = (Rectangle){2400,600,100,300};
            r->doorToRoom[0] = ROOM_CAMP;
            r->doorToPos[0] = (Vector2){50,350};
            r->doors[1] = (Rectangle){1100,-100,300,100};
            r->doorToRoom[1] = ROOM_BUSHES;
            r->doorToPos[1] = (Vector2){450,550};
            r->doors[2] = (Rectangle){400,1200,300,100};
            r->doorToRoom[2] = ROOM_GRAVEYARD;
            r->doorToPos[2] = (Vector2){450,50};
            r->doors[3] = (Rectangle){1900,1200,300,100};
            r->doorToRoom[3] = ROOM_DUNGEON;
            r->doorToPos[3] = (Vector2){1250,50};
            r->wallCount = 31;
            r->walls[30] = (Rectangle){300,800,100,100};
            r->wallType[30] = WALL_DEADTREES;
            r->walls[0] = (Rectangle){0,0,1200,100};
            r->walls[1] = (Rectangle){1300,0,1700,100};
            r->walls[2] = (Rectangle){0,100,100,1100};
            r->walls[3] = (Rectangle){100,1100,400,100};
            r->walls[27] = (Rectangle){600,1100,1400,100};
            r->wallType[27] = WALL_PINETREES;
            if (r->openedDoor[0]) r->walls[DOOR_AT_WALL[0]] = (Rectangle){-100,-100,1,1};
            else r->walls[DOOR_AT_WALL[0]] = (Rectangle){2000,1100,100,100};
            r->wallType[DOOR_AT_WALL[0]] = WALL_EMPTY;
            if (r->openedDoor[2]) r->walls[DOOR_AT_WALL[2]] = (Rectangle){-100,-100,1,1};
            else r->walls[DOOR_AT_WALL[2]] = (Rectangle){1200,0,100,100};
            r->wallType[DOOR_AT_WALL[2]] = WALL_EMPTY;
            r->walls[4] = (Rectangle){2100,1100,200,100};
            r->walls[5] = (Rectangle){1950,850,100,100};
            r->walls[6] = (Rectangle){2300,800,100,400};
            r->walls[7] = (Rectangle){100,500,300,100};
            r->walls[8] = (Rectangle){500,500,700,100};
            r->walls[9] = (Rectangle){800,700,100,300};
            r->walls[10] = (Rectangle){700,100,100,300};
            r->walls[11] = (Rectangle){400,800,100,100};
            r->walls[12] = (Rectangle){500,600,100,300};
            r->walls[13] = (Rectangle){1000,800,400,100};
            r->walls[14] = (Rectangle){900,200,100,200};
            r->walls[15] = (Rectangle){1000,300,400,100};
            for (int i=0; i < 16; i++) r->wallType[i] = WALL_PINETREES;
            r->walls[16] = (Rectangle){1300,500,100,100};
            r->wallType[16] = WALL_STATUE;
            r->walls[17] = (Rectangle){1600,100,600,100};
            r->wallType[17] = WALL_FENCE_HORZ;
            r->walls[18] = (Rectangle){1500,100,100,100};
            r->wallType[18] = WALL_FENCE_TOPLEFT;
            r->walls[19] = (Rectangle){2200,100,100,100};
            r->wallType[19] = WALL_FENCE_TOPRIGHT;
            r->walls[20] = (Rectangle){1500,200,100,400};
            r->wallType[20] = WALL_FENCE_VERT;
            r->walls[21] = (Rectangle){2200,200,100,400};
            r->wallType[21] = WALL_FENCE_VERT;
            r->walls[22] = (Rectangle){1500,600,100,100};
            r->wallType[22] = WALL_FENCE_BOTTOMLEFT;
            r->walls[23] = (Rectangle){2200,600,100,100};
            r->wallType[23] = WALL_FENCE_BOTTOMRIGHT;
            r->walls[24] = (Rectangle){1600,600,100,100};
            r->wallType[24] = WALL_FENCE_HORZ;
            r->walls[25] = (Rectangle){1900,600,300,100};
            r->wallType[25] = WALL_FENCE_HORZ;
            r->walls[26] = (Rectangle){2300,100,100,600};
            r->wallType[26] = WALL_PINETREES;
            r->enemyCount = 6;
            r->enemyPos[0] = (Vector2){-100,-100};
            r->enemyPos[1] = (Vector2){-100,-100};
            r->enemyPos[2] = (Vector2){1200,700};
            r->enemyPos[3] = (Vector2){1900,900};
            r->enemyPos[4] = (Vector2){1200,250};
            r->enemyPos[5] = (Vector2){400,900};
            for (int i=0; i<r->enemyCount; i++) r->enemyType[i] = 0;
            for (int i=0; i<r->enemyCount; i++) r->enemyVel[i] = (Vector2){0,0};
            for (int i=0; i<r->enemyCount; i++) r->enemyStun[i] = 0.1;
            r->enemyFloat = 1.0;
            r->boundary = (Rectangle){0,0,2400,1200};
            r->enemySize = 40;
            r->hasQuest = 1;
            r->questPos = (Vector2){1850,250};
            r->questSize = 40;
            r->questBox = (Rectangle){1650,50,400,100};
            r->questFloat = 0;
            r->quest = 0;
            break;
        case ROOM_CAMP:
            r->music = MUSIC_FOREST;
            r->background = (Color){80,160,64,255};
            r->fog = (Color){0,0,0,0};
            r->localKey = 1;
            if (r->killedSpiders >= 10) {
                r->keyPos = (Vector2){400,300};
            } else r->keyPos = (Vector2){-100,-100};
            r->doorCount = 1;
            r->doors[0] = (Rectangle){-100,200,100,300};
            r->doorToRoom[0] = ROOM_FOREST;
            r->doorToPos[0] = (Vector2){2350,750};
            r->wallCount = 4;
            r->walls[0] = (Rectangle){0,0,800,200};
            r->walls[1] = (Rectangle){0,400,800,200};
            r->walls[2] = (Rectangle){600,200,200,200};
            r->walls[3] = (Rectangle){0,200,100,100};
            for (int i=0; i < 16; i++) r->wallType[i] = WALL_APPLETREES;
            r->enemyCount = 0;
            r->boundary = (Rectangle){0,0,800,600};
            r->hasQuest = 1;
            r->questPos = (Vector2){500,250};
            r->questSize = 40;
            r->questBox = (Rectangle){300,50,400,100};
            r->questFloat = 0;
            r->quest = 1;
            break;
        case ROOM_BUSHES:
            if (r->delivered) r->music = MUSIC_BABY_LAUGHING;
            else r->music = MUSIC_BABY_CRYING;
            r->background = (Color){80,160,64,255};
            r->fog = (Color){0,0,0,0};
            r->localKey = -1;
            r->doorCount = 1;
            r->doors[0] = (Rectangle){300, 600, 300, 100};
            r->doorToRoom[0] = ROOM_FOREST;
            r->doorToPos[0] = (Vector2){1250,50};
            r->wallCount = 5;
            r->walls[0] = (Rectangle){100,0,600,200};
            r->walls[1] = (Rectangle){0,0,100,500};
            r->walls[2] = (Rectangle){700,0,100,500};
            r->walls[3] = (Rectangle){0,500,400,100};
            r->walls[4] = (Rectangle){500,500,300,100};
            for (int i=0; i<5; i++) r->wallType[i] = WALL_APPLETREES;
            r->enemyCount = 0;
            r->boundary = (Rectangle){0,0,800,600};
            r->hasQuest = 1;
            r->questPos = (Vector2){400,250};
            r->questSize = 40;
            r->questBox = (Rectangle){200,50,400,100};
            r->questFloat = 0;
            r->quest = 2;
            break;
        case ROOM_GRAVEYARD:
            r->monsterHealth = 5;
            r->music = MUSIC_GRAVEYARD;
            r->background = (Color){140,120,90,255};
            r->fog = (Color){200,200,200,55};
            if (!(r->hasKey[0])) {
                r->localKey = 0;
                r->keyPos = (Vector2){-100,-100};
            } else r->localKey = -1;
            r->doorCount = 1;
            r->doors[0] = (Rectangle){300, -100, 300, 100};
            r->doorToRoom[0] = ROOM_FOREST;
            r->doorToPos[0] = (Vector2){550,1150};
            for (int i=0; i<5; i++) r->wallType[i] = WALL_DEADTREES;
            for (int i=5; i<7; i++) r->wallType[i] = WALL_GRAVE;
            r->wallCount = 7;
            r->walls[0] = (Rectangle){0,500,800,100};
            r->walls[1] = (Rectangle){0,100,100,400};
            r->walls[2] = (Rectangle){700,100,100,400};
            r->walls[3] = (Rectangle){0,0,400,100};
            r->walls[4] = (Rectangle){500,0,300,100};
            r->walls[5] = (Rectangle){200,200,100,200};
            r->walls[6] = (Rectangle){500,200,100,200};
            r->enemyCount = 3;
            r->enemyPos[0] = (Vector2){-100,100};
            r->enemyPos[1] = (Vector2){-100,100};
            r->enemyType[2] = 1; 
            r->enemyStun[2] = 0.1;
            r->enemyVel[2] = (Vector2){-512,0};
            r->enemyPos[2] = (Vector2){650,450};
            r->enemySize = 60;
            r->boundary = (Rectangle){0,0,800,600};
            r->hasQuest = 0;
            break;
        case ROOM_DUNGEON:
            r->music = MUSIC_DUNGEON;
            r->background = (Color){0,0,0,255};
            r->fog = (Color){0,0,0,125};
            r->localKey = 2;
            r->keyPos = (Vector2){600,1250};
            r->doorCount = 1;
            r->doors[0] = (Rectangle){1100,-100,300,100};
            r->doorToPos[0] = (Vector2){2050,1150};
            r->doorToRoom[0] = ROOM_FOREST;
            r->wallCount = 9;
            r->walls[0] = (Rectangle){0,0,1200,100};
            r->walls[1] = (Rectangle){1300,0,1100,100};
            r->walls[2] = (Rectangle){0,100,100,1300};
            r->walls[3] = (Rectangle){100,1100,200,100};
            r->walls[4] = (Rectangle){2300,100,100,400};
            r->walls[5] = (Rectangle){400,1100,300,100};
            r->walls[6] = (Rectangle){700,500,1700,900};
            r->walls[7] = (Rectangle){100,1300,600,100};
            if (r->openedDoor[1]) r->walls[8] = (Rectangle){-100,-100,1,1};
            else r->walls[8] = (Rectangle){300,1100,100,100};
            r->wallType[8] = WALL_EMPTY;
            for (int i=0; i<r->wallCount; i++) r->wallType[i] = WALL_DUNGEON;
            r->enemyCount = 12;
            r->enemyFloat = 0;
            r->enemySize = 30;
            r->enemyPos[0] = (Vector2){-100,-100};
            r->enemyPos[1] = (Vector2){-100,-100};
            r->enemyPos[2] = (Vector2){400,300};
            r->enemyPos[5] = (Vector2){200,300};
            r->enemyPos[6] = (Vector2){600,300};
            r->enemyPos[3] = (Vector2){1200,300};
            r->enemyPos[10] = (Vector2){1200,200};
            r->enemyPos[11] = (Vector2){1200,400};
            r->enemyPos[4] = (Vector2){2000,300};
            r->enemyPos[7] = (Vector2){400,900};
            r->enemyPos[8] = (Vector2){200,900};
            r->enemyPos[9] = (Vector2){600,900};
            for (int i=0; i<r->enemyCount; i++) r->enemyType[i] = 2;
            for (int i=0; i<r->enemyCount; i++) r->enemyVel[i] = (Vector2){0,0};
            for (int i=0; i<r->enemyCount; i++) r->enemyStun[i] = 0.1;
            r->boundary = (Rectangle){0,0,2400,1400};
            r->hasQuest = 0;
        default: break;
    }
}

#define MAX_FRAMES 50
#define MAX_ANIMATIONS 15
#define MAX_FRAMES_PER_ANIM 50
struct AnimationSprite {
    int playing;
    Vector2 offset;
    int frame;
    int currentAnim;
    Rectangle allFrames[MAX_FRAMES];
    int animLength[MAX_ANIMATIONS];
    int anim[MAX_ANIMATIONS][MAX_FRAMES_PER_ANIM];
};
void UpdateAnimationSprite (struct AnimationSprite * sprite) {
    if (sprite->playing) {
        if (sprite->frame < sprite->animLength[sprite->currentAnim]-1) sprite->frame++;
        else sprite->frame = 0;
    }
}
void DrawAnimationSprite(Texture2D texture, struct AnimationSprite sprite, Vector2 pos, Color tint) {
    DrawTextureRec(texture, sprite.allFrames[sprite.anim[sprite.currentAnim][sprite.frame]],
    (Vector2){pos.x-sprite.allFrames[sprite.anim[sprite.currentAnim][sprite.frame]].width*sprite.offset.x,
    pos.y-sprite.allFrames[sprite.anim[sprite.currentAnim][sprite.frame]].height*sprite.offset.y}, tint);
}


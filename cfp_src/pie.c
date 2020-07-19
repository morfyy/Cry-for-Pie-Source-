#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "resources/pieskel.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

typedef struct {
    // easter egg
    int dittoPantEnabled;
    // dev variables
    int devmode;
    int devinfo;
    int showcol;
    int noclip;
    int god;
    int console;
    int consolePen;
    char consoleInput[30];
    char consoleHistory[30];
    float consoleTimer;
    float timescale;
    
    // window settings
    int width;
    int height;
    float scale; // scale variable needed for scaleType KEEPING
    RenderTexture2D renderTarget;
    enum enum_scale_type scaleType;
    int filterMode;
    int forceRes; // forces resolution when toggling fullscreen, NO REASON TO TURN THIS OFF, cant exit fullscreen when this is off
    int targetMonitor; // doesnt really work...
    int scaleMouse; // scale mouse position to window
    int autoRes; // auto detect fullscreen resolution
    int fullscreenWidth;
    int fullscreenHeight;
    
    // platform stuff
    int onWeb; // simpsle boolean to check if your on web
    // project state
    enum enum_project_state state;
    
    // translation
    enum enum_language lang;
    
    // splashscreen
    float splashFade;
    float splashTime;
    
    // menu
    enum enum_menu_state menuState;
    int selected;
    char* menuMain[8][MAX_LANG]; 
    char* menuSettings[8][MAX_LANG]; 
    char* menuControls[7][MAX_LANG];
    //char** menu;
    
    // game
    // resources
    Texture2D texture[MAX_TEXTURE_COUNT];
    Music music[MAX_MUSIC_COUNT];
    Sound sound[MAX_SOUND_COUNT];
    Font font;
    float soundVol;
    float musicVol;
    float masterVol;
    // actual game
    int playerDirX;
    int playerDirY;
    Vector2 playerPos;
    Vector2 playerVel;
    Vector2 punchPos;
    float playerFloat;
    int playerPunching;
    float playerSize;
    int playerAlive;
    float respawnTimer;
    int hasWhip;
    int playerWhipping;
    Rectangle whipRect;
    
    //float playerFloat2;
    Camera2D cam;
    struct Room currentRoom;
    //animations
    float animTimer;
    float animSpeed;
    struct AnimationSprite asPlayer;
    struct AnimationSprite asPig;
    struct AnimationSprite asCowboy;
    struct AnimationSprite asMonster;
    struct AnimationSprite asDoor;
    struct AnimationSprite asExplosion;
    int openingDoor;
    // messages
    float messageTimer;
    int message;
    Rectangle messageRect;
    int messageBool;
    int hasSave;
    int startedGame;
    int changingKey;
    float saveTextFloat;
} Project;
// Functions that need Project struct
void ToggleFullscreenPro(Project p) {
    if (!p.onWeb) {
        int mon = 0;
        if (p.targetMonitor >= 0 && p.targetMonitor < GetMonitorCount()) mon = p.targetMonitor;
        int width;
        int height;
        if (p.autoRes) { width = GetMonitorWidth(mon); height = GetMonitorHeight(mon); }
        else { width = p.fullscreenWidth; height = p.fullscreenHeight; }
        if (p.forceRes && !IsWindowFullscreen()) SetWindowSize( width, height);
        ToggleFullscreen();
        if (IsWindowFullscreen()) SetWindowMonitor(mon);
        if (p.forceRes && !IsWindowFullscreen()) {
            SetWindowSize(p.width,p.height);
            SetWindowPosition(0.5*(GetMonitorWidth(mon)-GetScreenWidth()), 0.5*(GetMonitorHeight(mon)-GetScreenHeight()));
        }
    }
}
void CloseProject(Project * p) {
    for (int i=0; i<MAX_TEXTURE_COUNT; i++) UnloadTexture(p->texture[i]);
    for (int i=0; i<MAX_MUSIC_COUNT; i++) UnloadMusicStream(p->music[i]);
    for (int i=0; i<MAX_SOUND_COUNT; i++) UnloadSound(p->sound[i]);
    UnloadFont(defaultFont);
    UnloadFont(p->font);
    free(p);
    CloseAudioDevice();
    CloseWindow();
    exit(0);
}
void GameOver(Project *p) {
    if (!(p->god)) {
        p->playerPunching = 0;
        p->playerWhipping = 0;
        p->playerAlive = 0;
        p->asPlayer.frame = 0;
        p->asPlayer.currentAnim = 12;
        p->asPlayer.playing = 1;
        p->respawnTimer = 1.0;
        p->messageBool = 1;
        if (p->currentRoom.id == ROOM_GRAVEYARD) PlaySound(p->sound[SOUND_MONSTER_LAUGH]);
        //PlaySound(p->sound[SOUND_GAMEOVER]);
    }
}
void Respawn(Project *p) {
    //StopMusicStream(p->music[p->currentRoom.music]);
    ChangeRoom(&(p->currentRoom),ROOM_FOREST);
    //PlayMusicStream(p->music[p->currentRoom.music]);
    p->playerPos = (Vector2){400,300};
    p->playerVel = (Vector2){0,0};
    p->playerAlive = 1;
    p->asPlayer.frame = 0;
    p->asPlayer.currentAnim = 2;
    p->playerDirX = 0;
    p->playerDirY = 1;
    p->respawnTimer = 0.0;
    p->messageBool = 1;
    p->messageTimer = 0.0;
    p->asPlayer.offset = (Vector2){0.5,0.75};
}
void NewGame(Project *p) {
    p->currentRoom.capturedPigs = 0;
    p->hasWhip = 0;
    for (int i=0; i<10; i++) p->currentRoom.spiderAlive[i] = 1;
    p->currentRoom.killedSpiders = 0;
    p->currentRoom.hasKey[0] = 0;
    p->currentRoom.hasKey[1] = 0;
    p->currentRoom.hasKey[2] = 0;
    p->currentRoom.openedDoor[0] = 0;
    p->currentRoom.openedDoor[1] = 0;
    p->currentRoom.openedDoor[2] = 0;
    Respawn(p);
    p->message = 0;
    p->messageTimer = 7.0;
    p->messageBool = 1;
    p->currentRoom.delivered = 0;
    p->startedGame = 1;
    //for (int i=0; i<MAX_ENEMIES; i++) p->currentRoom.enemyStun[i] = 0.1;
}
void SaveGame(Project *p) {
    if (p->startedGame) {
        SaveStorageValue(STORAGE_POSITION_KEY1, p->currentRoom.hasKey[0]);
        SaveStorageValue(STORAGE_POSITION_KEY2, p->currentRoom.hasKey[1]);
        SaveStorageValue(STORAGE_POSITION_KEY3, p->currentRoom.hasKey[2]);
        SaveStorageValue(STORAGE_POSITION_DOOR1, p->currentRoom.openedDoor[0]);
        SaveStorageValue(STORAGE_POSITION_DOOR2, p->currentRoom.openedDoor[1]);
        SaveStorageValue(STORAGE_POSITION_DOOR3, p->currentRoom.openedDoor[2]);
        SaveStorageValue(STORAGE_POSITION_CAPTURED_PIGS, p->currentRoom.capturedPigs);
        SaveStorageValue(STORAGE_POSITION_KILLED_SPIDERS, p->currentRoom.killedSpiders);
        SaveStorageValue(STORAGE_POSITION_DELIVERED_PIE, p->currentRoom.delivered);
        if (FileExists("storage.data")) {
            p->hasSave = 1;
            PlaySound(p->sound[SOUND_SAVE]);
            p->saveTextFloat = 6.0;
        }
    }
}
void LoadGame(Project *p) {
    if (p->hasSave) {
        p->currentRoom.hasKey[0] = LoadStorageValue(STORAGE_POSITION_KEY1);
        p->currentRoom.hasKey[1] = LoadStorageValue(STORAGE_POSITION_KEY2);
        p->currentRoom.hasKey[2] = LoadStorageValue(STORAGE_POSITION_KEY3);
        p->currentRoom.openedDoor[0] = LoadStorageValue(STORAGE_POSITION_DOOR1);
        p->currentRoom.openedDoor[1] = LoadStorageValue(STORAGE_POSITION_DOOR2);
        p->currentRoom.openedDoor[2] = LoadStorageValue(STORAGE_POSITION_DOOR3);
        p->currentRoom.capturedPigs = LoadStorageValue(STORAGE_POSITION_CAPTURED_PIGS);
        p->currentRoom.killedSpiders = LoadStorageValue(STORAGE_POSITION_KILLED_SPIDERS);
        p->currentRoom.delivered = LoadStorageValue(STORAGE_POSITION_DELIVERED_PIE);
        if (p->currentRoom.killedSpiders > 0) {
            for (int i=0; i<p->currentRoom.killedSpiders; i++) p->currentRoom.spiderAlive[i] = 0;
            for (int i=p->currentRoom.killedSpiders; i<10; i++) p->currentRoom.spiderAlive[i] = 1;
        }
        Respawn(p);
        p->state = PS_GAME;
    }
}
void UpdateConsole(Project *);
void DrawConsole(Project);

// UpdateDrawFrame
void UpdateDrawFrame(Project *);

int main(void) {
#if defined(PLATFORM_DESKTOP)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
    const int WINDOW_STARTING_WIDTH = 800;
    const int WINDOW_STARTING_HEIGHT = 600;
    const int TARGET_FPS = 60;
    InitWindow(WINDOW_STARTING_WIDTH, WINDOW_STARTING_HEIGHT, "Cry for Pie - by Morfy");
    InitAudioDevice();
    SetWindowMinSize(MIN_WIDTH, MIN_HEIGHT);
    SetExitKey(0);
    
    // making project struct
    Project *project;
    project =  (Project*)malloc( sizeof(Project));
    if (project == NULL) {
        printf("Memory allocation failed!\n");
        UnloadFont(defaultFont);
        CloseAudioDevice();
        CloseWindow();
    } else {
        printf("Successfully allocated memory.\n");
    }
#if defined(PLATFORM_DESKTOP)
    project->onWeb = 0;
#else
    project->onWeb = 1;
#endif

    // loading default font (sorry, i dont really like the default raylib font. its good in the logo, but not as a default font...) 
    defaultFont = LoadFont("resources/fonts/arial.ttf");
    // easter egg
    project->dittoPantEnabled = 0;
    // dev variables
    project->devmode = 1;
    project->devinfo = 0;
    project->showcol = 0;
    project->noclip = 0;
    project->god = 0;
    project->console = 0;
    project->consolePen = 0;
    for (int i=0;i<arrsize(project->consoleHistory);i++) project->consoleHistory[i] = ' ';
    for (int i=0;i<arrsize(project->consoleInput);i++) project->consoleInput[i] = ' ';
    project->consoleTimer = 0;
    project->timescale = 1.0;
    
    // Window settings, scaling, videomodes ...
    project->width = 800;
    project->height = 600;
    project->scale = 0;
    project->renderTarget = LoadRenderTexture(project->width,project->height);
    project->filterMode = FILTER_BILINEAR;
    SetTextureFilter(project->renderTarget.texture, project->filterMode);
    project->scaleType = 1;
    project->forceRes = 1;
    project->targetMonitor = 0;
    project->scaleMouse = 1;
    project->autoRes = 1;
    project->fullscreenWidth = 1366;
    project->fullscreenHeight = 768;
    
    // translation
    project->lang = L_ENGLISH;
    
    
    // splashscreen
    project->state = PS_SPLASHSCREEN;
    project->splashFade = 1;
    project->splashTime = 3;
    
    // menu
    project->menuState = MS_MAIN;
    project->selected = 0;
    project->menuMain[0][L_ENGLISH] = "Continue";
    project->menuMain[0][L_GERMAN] = "Weiterspielen";
    project->menuMain[1][L_ENGLISH] = "New game";
    project->menuMain[1][L_GERMAN] = "Neues spiel";
    project->menuMain[2][L_ENGLISH] = "Save game";
    project->menuMain[2][L_GERMAN] = "Spiel speichern";
    project->menuMain[3][L_ENGLISH] = "Load game";
    project->menuMain[3][L_GERMAN] = "Spiel laden";
    project->menuMain[4][L_ENGLISH] = "Settings";
    project->menuMain[4][L_GERMAN] = "Einstellungen";
    project->menuMain[5][L_ENGLISH] = "Controls";
    project->menuMain[5][L_GERMAN] = "Steuerung";
    project->menuMain[6][L_ENGLISH] = "Credits";
    project->menuMain[6][L_GERMAN] = "Credits";
    project->menuMain[7][L_ENGLISH] = "Quit";
    project->menuMain[7][L_GERMAN] = "Beenden";
    
    project->menuSettings[0][L_ENGLISH] = "Master: ";
    project->menuSettings[0][L_GERMAN] = "Master: ";
    project->menuSettings[1][L_ENGLISH] = "Music: ";
    project->menuSettings[1][L_GERMAN] = "Musik: ";
    project->menuSettings[2][L_ENGLISH] = "Sounds: ";
    project->menuSettings[2][L_GERMAN] = "Sounds: ";
    project->menuSettings[3][L_ENGLISH] = "Fullscreen: ";
    project->menuSettings[3][L_GERMAN] = "Vollbild: ";
    project->menuSettings[4][L_ENGLISH] = "Monitor: ";
    project->menuSettings[4][L_GERMAN] = "Monitor: ";
    project->menuSettings[5][L_ENGLISH] = "Filter: ";
    project->menuSettings[5][L_GERMAN] = "Filter: ";
    project->menuSettings[6][L_ENGLISH] = "Language: ";
    project->menuSettings[6][L_GERMAN] = "Sprache: ";
    project->menuSettings[7][L_ENGLISH] = "Back";
    project->menuSettings[7][L_GERMAN] = "Zurueck";
    
    project->menuControls[0][L_ENGLISH] = "Move right: ";
    project->menuControls[0][L_GERMAN] = "Nach rechts: ";
    project->menuControls[1][L_ENGLISH] = "Move left: ";
    project->menuControls[1][L_GERMAN] = "Nach links: ";
    project->menuControls[2][L_ENGLISH] = "Move down: ";
    project->menuControls[2][L_GERMAN] = "Nach unten: ";
    project->menuControls[3][L_ENGLISH] = "Move up: ";
    project->menuControls[3][L_GERMAN] = "Nach oben: ";
    project->menuControls[4][L_ENGLISH] = "Interact: ";
    project->menuControls[4][L_GERMAN] = "Interaktion: ";
    project->menuControls[5][L_ENGLISH] = "Whip: ";
    project->menuControls[5][L_GERMAN] = "Peitsche: ";
    project->menuControls[6][L_ENGLISH] = "Back";
    project->menuControls[6][L_GERMAN] = "Zurueck";
    project->changingKey = -1;
    
    // game
    // resources
    project->texture[TEXTURE_RAYLIB] = LoadTexture("resources/vassets/raylib.png");
    project->texture[TEXTURE_TILES] = LoadTexture("resources/vassets/tiles.png");
    project->texture[TEXTURE_PLAYER] = LoadTexture("resources/vassets/player.png");
    project->texture[TEXTURE_DITTO_PANTS] = LoadTexture("resources/vassets/player2.png");
    project->texture[TEXTURE_NPC1] = LoadTexture("resources/vassets/npc1.png");
    project->texture[TEXTURE_NPC2] = LoadTexture("resources/vassets/npc2.png");
    project->texture[TEXTURE_NPC3] = LoadTexture("resources/vassets/npc3.png");
    project->texture[TEXTURE_MISC] = LoadTexture("resources/vassets/misc.png");
    SetTextureWrap(project->texture[TEXTURE_TILES], WRAP_MIRROR_REPEAT);
    project->music[MUSIC_WURFELSPIEL] = LoadMusicStream("resources/music/wurfelspiel.ogg");
    project->music[MUSIC_FOREST] = LoadMusicStream("resources/music/schweinebacken.ogg");
    project->music[MUSIC_DUNGEON] = LoadMusicStream("resources/music/ausderdunkelheit.ogg");
    project->music[MUSIC_GRAVEYARD] = LoadMusicStream("resources/music/ausdemnebel.ogg");
    project->music[MUSIC_BABY_CRYING] = LoadMusicStream("resources/sounds/baby_crying.mp3");
    project->music[MUSIC_BABY_LAUGHING] = LoadMusicStream("resources/sounds/baby_laughing.ogg");
    for (int i=0; i<MAX_MUSIC_COUNT; i++) PlayMusicStream(project->music[i]);
    project->sound[SOUND_SELECT] = LoadSound("resources/sounds/piep.ogg");
    project->sound[SOUND_ERROR] = LoadSound("resources/sounds/bz.ogg");
    project->sound[SOUND_PUNCH1] = LoadSound("resources/sounds/punch1.ogg");
    project->sound[SOUND_PUNCH2] = LoadSound("resources/sounds/punch2.ogg");
    project->sound[SOUND_PUNCH3] = LoadSound("resources/sounds/punch3.ogg");
    project->sound[SOUND_SAVE] = LoadSound("resources/sounds/fox.ogg");
    project->sound[SOUND_KEY] = LoadSound("resources/sounds/bling.ogg");
    project->sound[SOUND_DOOR] = LoadSound("resources/sounds/bupbupbupbup.ogg");
    project->sound[SOUND_WHIP] = LoadSound("resources/sounds/whip.ogg");
    project->sound[SOUND_EXPLOSION] = LoadSound("resources/sounds/explosion.ogg");
    project->sound[SOUND_MONSTER_LAUGH] = LoadSound("resources/sounds/dittocreeper_laugh.ogg");
    project->sound[SOUND_MONSTER_OUCH] = LoadSound("resources/sounds/dittocreeper_ouch.ogg");
    project->sound[SOUND_PIG] = LoadSound("resources/sounds/oink.ogg");
    project->sound[SOUND_CHEW] = LoadSound("resources/sounds/chew.ogg");
    project->font = LoadFont("resources/fonts/RetroGaming.ttf");
    project->soundVol = 0.5;
    project->musicVol = 0.5;
    project->masterVol = 0.5;
    for (int i=0; i<MAX_SOUND_COUNT; i++) SetSoundVolume(project->sound[i], project->soundVol);
    for (int i=0; i<MAX_MUSIC_COUNT; i++) SetMusicVolume(project->music[i], project->musicVol);
    project->currentRoom.music = -1;
    SetMasterVolume(project->masterVol);
    // actual game
    project->playerDirX = 0;
    project->playerDirY = 1;
    //project->playerPos = (Vector2){400,300};
    project->punchPos = project->playerPos;
    project->playerPunching = 0;
    project->playerWhipping = 0;
    project->playerFloat = 0;
    project->playerSize = 50;
    project->hasWhip = 1;
    project->playerWhipping = 0;
    project->cam.rotation = 0;
    project->cam.offset = (Vector2){project->width*0.5, project->height*0.5};
    project->cam.zoom = 1;
    project->cam.target = project->playerPos;
    // animations
    project->animTimer = 0.0;
    project->animSpeed = 14.0;
    project->asPlayer.offset = (Vector2){0.5, 0.75};
    project->asPlayer.currentAnim = 0;
    project->asPlayer.frame = 0;
    for (int i=0; i<5; i++) project->asPlayer.allFrames[i] = (Rectangle){100*i, 100, 100, 100};
    for (int i=5; i<10; i++) project->asPlayer.allFrames[i] = (Rectangle){100*(i-5), 200, 100, 100};
    for (int i=10; i<15; i++) project->asPlayer.allFrames[i] = (Rectangle){100*(i-10), 300, 100, 100};
    for (int i=15; i<20; i++) project->asPlayer.allFrames[i] = (Rectangle){100*(i-15), 400, 100, 100};
    for (int i=20; i<24; i++) project->asPlayer.allFrames[i] = (Rectangle){ 500, 100 + ((i-20)*100), 100, 100};
    project->asPlayer.allFrames[24] = (Rectangle){0,0,100,100};
    project->asPlayer.allFrames[25] = (Rectangle){100,0,200,100};
    project->asPlayer.allFrames[26] = (Rectangle){300,0,300,100};
    project->asPlayer.allFrames[27] = (Rectangle){500,500,100,100};
    project->asPlayer.allFrames[28] = (Rectangle){300,500,200,100};
    project->asPlayer.allFrames[29] = (Rectangle){0,500,300,100};
    project->asPlayer.allFrames[30] = (Rectangle){600,0,100,100};
    project->asPlayer.allFrames[31] = (Rectangle){600,100,100,200};
    project->asPlayer.allFrames[32] = (Rectangle){600,300,100,300};
    project->asPlayer.allFrames[33] = (Rectangle){700,500,100,100};
    project->asPlayer.allFrames[34] = (Rectangle){700,300,100,200};
    project->asPlayer.allFrames[35] = (Rectangle){700,0,100,300};
    project->asPlayer.allFrames[36] = (Rectangle){100,0,100,100};
    project->asPlayer.allFrames[37] = (Rectangle){200,0,100,100};
    project->asPlayer.allFrames[38] = (Rectangle){300,0,100,100};
    project->asPlayer.allFrames[39] = (Rectangle){400,0,100,100};
    project->asPlayer.animLength[0] = 8;
    project->asPlayer.animLength[1] = 8;
    project->asPlayer.animLength[2] = 8;
    project->asPlayer.animLength[3] = 8;
    project->asPlayer.anim[0][0] = 0; 
    project->asPlayer.anim[0][1] = 1; 
    project->asPlayer.anim[0][2] = 2; 
    project->asPlayer.anim[0][3] = 3; 
    project->asPlayer.anim[0][4] = 4; 
    project->asPlayer.anim[0][5] = 3; 
    project->asPlayer.anim[0][6] = 2; 
    project->asPlayer.anim[0][7] = 1; 
    for (int i=0; i<8; i++) {
        project->asPlayer.anim[1][i] = project->asPlayer.anim[0][i]+5;
        project->asPlayer.anim[2][i] = project->asPlayer.anim[0][i]+10;
        project->asPlayer.anim[3][i] = project->asPlayer.anim[0][i]+15;
    }
    for (int i=4; i<8; i++) project->asPlayer.animLength[i] = 1;
    for (int i=0; i<1; i++) {
        project->asPlayer.anim[4][i] = 20;
        project->asPlayer.anim[5][i] = 21;
        project->asPlayer.anim[6][i] = 22;
        project->asPlayer.anim[7][i] = 23;
    }
    project->asPlayer.animLength[8] = 14;
    project->asPlayer.animLength[9] = 14;
    project->asPlayer.animLength[10] = 14;
    project->asPlayer.animLength[11] = 14;
    for (int i=0; i<14; i++) {
        if (i<2) project->asPlayer.anim[8][i] = 24;
        else if (i<4) project->asPlayer.anim[8][i] = 25;
        else project->asPlayer.anim[8][i] = 26;
        if (i<2) project->asPlayer.anim[9][i] = 27;
        else if (i<4) project->asPlayer.anim[9][i] = 28;
        else project->asPlayer.anim[9][i] = 29;
        if (i<2) project->asPlayer.anim[10][i] = 30;
        else if (i<4) project->asPlayer.anim[10][i] = 31;
        else  project->asPlayer.anim[10][i] = 32;
        if (i<2) project->asPlayer.anim[11][i] = 33;
        else if (i<4) project->asPlayer.anim[11][i] = 34;
        else project->asPlayer.anim[11][i] = 35;
    }
    project->asPlayer.animLength[12] = 4;
    project->asPlayer.anim[12][0] = 36;
    project->asPlayer.anim[12][1] = 37;
    project->asPlayer.anim[12][2] = 38;
    project->asPlayer.anim[12][3] = 39;
    project->asPig.offset = (Vector2){0.5, 0.5};
    project->asPig.currentAnim = 0;
    project->asPig.frame = 0;
    for (int i=0; i<3; i++) project->asPig.allFrames[i] = (Rectangle){100*i, 0, 100, 100};
    for (int i=0; i<3; i++) project->asPig.allFrames[3+i] = (Rectangle){100*i, 100, 100, 100};
    for (int i=0; i<3; i++) project->asPig.allFrames[6+i] = (Rectangle){100*i, 200, 100, 100};
    for (int i=0; i<3; i++) project->asPig.allFrames[9+i] = (Rectangle){100*i, 300, 100, 100};
    project->asPig.animLength[0] = 4;
    project->asPig.anim[0][0] = 0;
    project->asPig.anim[0][1] = 1;
    project->asPig.anim[0][2] = 2;
    project->asPig.anim[0][3] = 1;
    for (int i=0; i<4; i++) {
        project->asPig.animLength[i] = 4;
        project->asPig.anim[1][i] = project->asPig.anim[0][i]+3;
        project->asPig.anim[2][i] = project->asPig.anim[0][i]+6;
        project->asPig.anim[3][i] = project->asPig.anim[0][i]+9;
    }
    project->asPig.playing = 1;
    project->asCowboy.offset = (Vector2){0.5,0.75};
    project->asCowboy.currentAnim = 0;
    project->asCowboy.frame = 0;
    for (int i=0; i<4; i++) project->asCowboy.allFrames[i] = (Rectangle){100*i, 0, 100, 100};
    for (int i=0; i<4; i++) project->asCowboy.allFrames[4+i] = (Rectangle){100*i, 100, 100, 100};
    for (int i=0; i<4; i++) project->asCowboy.allFrames[8+i] = (Rectangle){100*i, 300, 100, 100};
    project->asCowboy.animLength[0] = 30;
    for (int i=0; i<17; i++) project->asCowboy.anim[0][i] = 0;
    project->asCowboy.anim[0][17] = 1;
    for (int i=18; i<22; i++) project->asCowboy.anim[0][i] = 2;
    for (int i=22; i<24; i++) project->asCowboy.anim[0][i] = 3;
    for (int i=24; i<30; i++) project->asCowboy.anim[0][i] = 0;
    project->asCowboy.animLength[1] = 40;
    for (int i=0; i<30; i++) project->asCowboy.anim[1][i] = 4;
    project->asCowboy.anim[1][30] = 6;
    project->asCowboy.anim[1][31] = 5;
    project->asCowboy.anim[1][32] = 6;
    for (int i=33; i<39; i++) project->asCowboy.anim[1][i] = 7;
    project->asCowboy.anim[1][39] = 6;
    project->asCowboy.animLength[2] = 1; project->asCowboy.anim[2][0] = 8;
    project->asCowboy.animLength[3] = 30;
    for (int i=0; i<20; i++) project->asCowboy.anim[3][i] = 9;
    project->asCowboy.anim[3][20] = 10; project->asCowboy.anim[3][21] = 10;
    project->asCowboy.anim[3][22] = 11; project->asCowboy.anim[3][23] = 11;
    for (int i=24; i<30; i++) project->asCowboy.anim[3][i] = 9;
    project->asCowboy.playing = 1;
    project->asMonster.offset = (Vector2){0.5,0.75};
    project->asMonster.currentAnim = 0;
    project->asMonster.frame = 0;
    for (int i=0; i<5; i++) project->asMonster.allFrames[i] = (Rectangle){100*i, 0, 100, 100};
    for (int i=5; i<10; i++) project->asMonster.allFrames[i] = (Rectangle){100*(i-5), 100, 100, 100};
    for (int i=10; i<15; i++) project->asMonster.allFrames[i] = (Rectangle){100*(i-10), 200, 100, 100};
    for (int i=15; i<20; i++) project->asMonster.allFrames[i] = (Rectangle){100*(i-15), 300, 100, 100};
    project->asMonster.animLength[0] = 8;
    project->asMonster.animLength[1] = 8;
    project->asMonster.animLength[2] = 8;
    project->asMonster.animLength[3] = 8;
    project->asMonster.anim[0][0] = 0; 
    project->asMonster.anim[0][1] = 1; 
    project->asMonster.anim[0][2] = 2; 
    project->asMonster.anim[0][3] = 3; 
    project->asMonster.anim[0][4] = 4; 
    project->asMonster.anim[0][5] = 3; 
    project->asMonster.anim[0][6] = 2; 
    project->asMonster.anim[0][7] = 1; 
    for (int i=0; i<8; i++) {
        project->asMonster.anim[1][i] = project->asMonster.anim[0][i]+5;
        project->asMonster.anim[2][i] = project->asMonster.anim[0][i]+10;
        project->asMonster.anim[3][i] = project->asMonster.anim[0][i]+15;
    }
    project->asMonster.playing = 1;
    for (int i=0; i<5; i++) project->asDoor.allFrames[i] = (Rectangle){100+100*i,100,100,100};
    project->asDoor.frame = 0;
    project->asDoor.animLength[0] = 5;
    for (int i=0; i<5; i++) project->asDoor.anim[0][i] = i;
    project->asDoor.playing = 0;
    project->asDoor.offset = (Vector2){0,0};
    project->asDoor.currentAnim = 0;
    for (int i=0; i<6; i++) project->asExplosion.allFrames[i] = (Rectangle){100*i,200,100,100};
    project->asExplosion.animLength[0] = 12;
    project->asExplosion.anim[0][0] = 0;
    project->asExplosion.anim[0][1] = 1;
    project->asExplosion.anim[0][2] = 2;
    project->asExplosion.anim[0][3] = 3;
    project->asExplosion.anim[0][4] = 3;
    project->asExplosion.anim[0][5] = 4;
    project->asExplosion.anim[0][6] = 4;
    project->asExplosion.anim[0][7] = 4;
    project->asExplosion.anim[0][8] = 5;
    project->asExplosion.anim[0][9] = 5;
    project->asExplosion.anim[0][10] = 5;
    project->asExplosion.anim[0][11] = 5;
    project->asExplosion.frame = -1;
    project->asExplosion.currentAnim = 0;
    project->asExplosion.playing = 0;
    project->asExplosion.offset = (Vector2){0.5,0.5};
    project->openingDoor = -1;
    project->messageTimer = 0.0;
    project->message = 0;
    project->messageRect = (Rectangle){500,100,250,250};
    project->saveTextFloat = 0.0;
    //NewGame(project);
    project->startedGame = 0;
    if (FileExists("storage.data")) project->hasSave = 1;
    else project->hasSave = 0;
    
    // set main loop for web, if on web
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(UpdateDrawFrame, project, 0, 1);
#else
    SetTargetFPS(TARGET_FPS);
    // start main loop for desktop
    while (!WindowShouldClose()) {
        UpdateDrawFrame(project);
    }
#endif
    // Close things and Unload stuff
    CloseProject(project);
    return 0;
}


// update and render
void UpdateDrawFrame(Project * p) {
    // Define delta
    delta = GetFrameTime()*p->timescale;
    // Devmode
    if (p->devmode) {
        if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_COMMA)) p->console = 1;
        if (p->console) UpdateConsole(p);
    }
    // Update scale variable
    p->scale = min((float)GetScreenWidth()/p->width, (float)GetScreenHeight()/p->height);
    // virtualMouse
    if (p->scaleMouse) {
        switch (p->scaleType) {
            case ST_KEEPING:
                mouse.x = (GetMousePosition().x - (GetScreenWidth() - (p->width*p->scale))*0.5f)/p->scale;
                mouse.y = (GetMousePosition().y - (GetScreenHeight() - (p->height*p->scale))*0.5f)/p->scale;
                if (mouse.x < 0) mouse.x = 0;
                if (mouse.y < 0) mouse.y = 0;
                if (mouse.x > p->width) mouse.x = p->width;
                if (mouse.y > p->height) mouse.y = p->height;
                break;
            case ST_STRETCHING:
                break;
            default: break;
        }
    }
    // UPDATE GAME
    printf("%f\n",1/GetFrameTime());
    if (!(p->console)) {
        if (p->saveTextFloat>0.0) p->saveTextFloat -= delta;
        switch (p->state) {
            case PS_SPLASHSCREEN:
                if (IsAnyKeyPressed()) {p->splashTime = 0; p->splashFade = 0;}
                if (p->splashTime > 0) p->splashTime -= delta;
                else if (p->splashFade > 0) p->splashFade -= delta*SPLASH_FADE_OUT_SPEED;
                else p->state = PS_MENU;
                break;
            case PS_MENU:
                switch (p->menuState) {
                    case MS_MAIN:
                        if (p->startedGame && InputEscape() && !(p->console))  p->state = PS_GAME;
                        if (InputMoveUpPressed() || IsKeyPressed(KEY_UP)) {
                            PlaySound(p->sound[SOUND_SELECT]);
                            p->selected--;
                            if (p->selected < 0) p->selected = arrsize(p->menuMain)-1;
                        } else if (InputMoveDownPressed() || IsKeyPressed(KEY_DOWN)) {
                            PlaySound(p->sound[SOUND_SELECT]);
                            p->selected++;
                            if (p->selected > arrsize(p->menuMain)-1) p->selected = 0;
                        }
                        if (InputPrimary()) {
                            PlaySound(p->sound[SOUND_SELECT]);
                            if (p->selected == 0) {
                                if (p->startedGame) {p->state = PS_GAME;}
                                else if (p->hasSave) {LoadGame(p);p->startedGame = 1;}
                            } else if (p->selected == 1) {NewGame(p); p->state = PS_GAME;}
                            else if (p->selected == 2) SaveGame(p);
                            else if (p->selected == 3) LoadGame(p);
                            else if (p->selected == 4) {
                                p->menuState = MS_SETTINGS;
                                p->selected = arrsize(p->menuSettings)-1;
                            } else if (p->selected == 5) {
                                p->menuState = MS_CONTROLS;
                                p->selected = arrsize(p->menuControls)-1;
                            } else if (p->selected == 7) CloseProject(p);
                        }
                        break;
                    case MS_SETTINGS:
                        if (InputEscape() && !(p->console))  {p->menuState = MS_MAIN; p->selected = 4;}
                        if (InputMoveUpPressed() || IsKeyPressed(KEY_UP)) {
                            PlaySound(p->sound[SOUND_SELECT]);
                            p->selected--;
                            if (p->selected < 0) p->selected = arrsize(p->menuSettings)-1;
                        } else if (InputMoveDownPressed() || IsKeyPressed(KEY_DOWN)) {
                            PlaySound(p->sound[SOUND_SELECT]);
                            p->selected++;
                            if (p->selected > arrsize(p->menuSettings)-1) p->selected = 0;
                        }
                        if (p->selected == 0) {
                            if (InputMoveRightPressed() || IsKeyPressed(KEY_RIGHT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->masterVol += 0.10;
                                p->masterVol = ClampFloat(p->masterVol,0.0,1.0);
                                SetMasterVolume(p->masterVol);
                            } else if (InputMoveLeftPressed() || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->masterVol -= 0.10;
                                p->masterVol = ClampFloat(p->masterVol,0.0,1.0);
                                SetMasterVolume(p->masterVol);
                            }
                        } else if (p->selected == 1) {
                            if (InputMoveRightPressed() || IsKeyPressed(KEY_RIGHT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->musicVol += 0.10;
                                p->musicVol = ClampFloat(p->musicVol,0.0,1.0);
                                for (int i=0; i<MAX_MUSIC_COUNT; i++) SetMusicVolume(p->music[i],p->musicVol);
                            } else if (InputMoveLeftPressed() || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->musicVol -= 0.10;
                                p->musicVol = ClampFloat(p->musicVol,0.0,1.0);
                                for (int i=0; i<MAX_MUSIC_COUNT; i++) SetMusicVolume(p->music[i],p->musicVol);
                            }
                        } else if (p->selected == 2) {
                            if (InputMoveRightPressed() || IsKeyPressed(KEY_RIGHT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->soundVol += 0.10;
                                p->soundVol = ClampFloat(p->soundVol,0.0,1.0);
                                for (int i=0; i<MAX_SOUND_COUNT; i++) SetSoundVolume(p->sound[i],p->soundVol);
                            } else if (InputMoveLeftPressed() || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->soundVol -= 0.10;
                                p->soundVol = ClampFloat(p->soundVol,0.0,1.0);
                                for (int i=0; i<MAX_SOUND_COUNT; i++) SetSoundVolume(p->sound[i],p->soundVol);
                            }
                        } else if (p->selected == 3) {
                            if (InputMoveRightPressed() || InputMoveLeftPressed() || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                ToggleFullscreenPro(*p);
                            }
                        } else if (p->selected == 4) {
                            if (InputMoveRightPressed() || IsKeyPressed(KEY_RIGHT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->targetMonitor++;
                            }
                            else if (InputMoveLeftPressed() || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->targetMonitor--;
                            }
                            p->targetMonitor = ClampInteger(p->targetMonitor, 0, GetMonitorCount()-1);
                        } else if (p->selected == 5) {
                            if (InputMoveRightPressed() || InputMoveLeftPressed() || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->filterMode = toggle(p->filterMode);
                                SetTextureFilter(p->renderTarget.texture,p->filterMode);
                            }
                        } else if (p->selected == 6) {
                            if (InputMoveRightPressed() || InputMoveLeftPressed() || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->lang++;
                                if (p->lang>=MAX_LANG) p->lang = 0;
                            }
                        }
                        if (InputPrimary()) {
                            PlaySound(p->sound[SOUND_SELECT]);
                            if (p->selected == 3) {
                                ToggleFullscreenPro(*p);
                            } else if (p->selected == 5) {
                                p->filterMode = toggle(p->filterMode);
                                SetTextureFilter(p->renderTarget.texture,p->filterMode);
                            } else if (p->selected == 6) {
                                p->lang++;
                                if (p->lang>=MAX_LANG) p->lang = 0;
                            } else if (p->selected == 7) {p->menuState = MS_MAIN; p->selected = 4;}
                        }
                        break;
                    case MS_CONTROLS:
                        if (p->changingKey>=0) {
                            if (InputEscape()) p->changingKey = -1;
                            int key = GetKeyPressed();
                            if (IsKeyPressed(KEY_RIGHT)) key = KEY_RIGHT;
                            else if (IsKeyPressed(KEY_LEFT)) key = KEY_LEFT;
                            else if (IsKeyPressed(KEY_DOWN)) key = KEY_DOWN;
                            else if (IsKeyPressed(KEY_UP)) key = KEY_UP;
                            if (CodepointIsUseable(key)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                if (key>=97 && key<= 122) key -= 32;
                                switch (p->selected) {
                                    case 0: 
                                        keyRight = key;
                                        break;
                                    case 1: 
                                        keyLeft = key;
                                        break;
                                    case 2: 
                                        keyDown = key;
                                        break;
                                    case 3: 
                                        keyUp = key;
                                        break;
                                    case 4: 
                                        keyPrimary = key;
                                        break;
                                    case 5: 
                                        keySecondary = key;
                                        break;
                                    default: p->selected = 0;
                                }
                                p->changingKey = -1;
                            } else if (key>0) {
                                PlaySound(p->sound[SOUND_ERROR]);
                            }
                        } else {
                            if (InputEscape() && !(p->console)) {p->menuState = MS_MAIN; p->selected = 5;}
                            if (InputMoveUpPressed() || IsKeyPressed(KEY_UP)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->selected--;
                                if (p->selected < 0) p->selected = arrsize(p->menuControls)-1;
                            } else if (InputMoveDownPressed() || IsKeyPressed(KEY_DOWN)) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                p->selected++;
                                if (p->selected > arrsize(p->menuControls)-1) p->selected = 0;
                            }
                            if (InputPrimary()) {
                                PlaySound(p->sound[SOUND_SELECT]);
                                if (p->selected == 6) {p->menuState = MS_MAIN; p->selected = 5;}
                                else p->changingKey = p->selected;
                            }
                        }
                        break;
                    default: p->menuState = MS_MAIN;
                }
                break;
            case PS_GAME:
                if (p->currentRoom.music >= 0) UpdateMusicStream(p->music[p->currentRoom.music]);
                if (InputEscape() && !(p->console)) {p->state = PS_MENU; p->selected = 0; p->menuState = MS_MAIN;}
                {int tempCapturedPigs = 1;
                for (int i=0; i<4; i++) {
                    if (p->currentRoom.pigChecker[i] == 0) {
                        tempCapturedPigs = 0;
                        break;
                    }
                }
                if (tempCapturedPigs) p->currentRoom.capturedPigs = 1;
                }
                p->playerVel.x = 0;
                p->playerVel.y = 0;
                if (p->asPlayer.currentAnim>=4 && p->asPlayer.currentAnim<=7) {
                    if (p->playerFloat<=PUNCH_DURATION-PUNCH_COLLISION_DURATION) p->playerPunching=0;
                    if (p->playerFloat >= 0) p->playerFloat -= delta;
                    else {
                        if (p->playerDirX >= 1) p->asPlayer.currentAnim = 0;
                        else if (p->playerDirX <= -1) p->asPlayer.currentAnim = 1;
                        if (p->playerDirY >= 1) p->asPlayer.currentAnim = 2;
                        else if (p->playerDirY <= -1) p->asPlayer.currentAnim = 3;
                        p->asPlayer.frame = 0;
                    }
                } else if (p->asPlayer.currentAnim>=8 && p->asPlayer.currentAnim<=11) {
                    if (p->asPlayer.frame >= 13) {
                        if (p->playerDirX >= 1) p->asPlayer.currentAnim = 0;
                        else if (p->playerDirX <= -1) p->asPlayer.currentAnim = 1;
                        if (p->playerDirY >= 1) p->asPlayer.currentAnim = 2;
                        else if (p->playerDirY <= -1) p->asPlayer.currentAnim = 3;
                        p->asPlayer.frame = 0;
                        p->playerWhipping = 0;
                        p->asPlayer.offset.x = 0.5;
                        p->asPlayer.offset.y = 0.75;
                    } else if (p->asPlayer.frame >= 4 && p->asPlayer.frame < 6) {
                        if (p->playerWhipping != 1) p->playerWhipping = 1;
                        if (p->asPlayer.frame < 4) {
                            if (p->playerDirX != 0) {
                                p->whipRect = (Rectangle){min(p->playerPos.x,p->playerPos.x+(p->playerDirX*100))-12.5,
                                min(p->playerPos.y,p->playerPos.y+(p->playerDirY*100))-25,100,25};
                            }
                            if (p->playerDirY != 0) {
                                p->whipRect = (Rectangle){min(p->playerPos.x,p->playerPos.x+(p->playerDirX*100))-12.5,
                                min(p->playerPos.y,p->playerPos.y+(p->playerDirY*100))-25,25,100};
                            }
                        } else {
                            if (p->playerDirX != 0) {
                                p->whipRect = (Rectangle){min(p->playerPos.x,p->playerPos.x+(p->playerDirX*200))-12.5,
                                min(p->playerPos.y,p->playerPos.y+(p->playerDirY*200))-25,200,25};
                            }
                            if (p->playerDirY != 0) {
                                p->whipRect = (Rectangle){min(p->playerPos.x,p->playerPos.x+(p->playerDirX*200))-12.5,
                                min(p->playerPos.y,p->playerPos.y+(p->playerDirY*200))-25,25,200};
                            }
                        }
                    } else {
                        if (p->playerWhipping != 0) p->playerWhipping = 0;
                    }
                } else if (p->asPlayer.currentAnim <= 3) {
                    if (InputMoveRight()) {p->playerVel.x = PLAYER_SPEED; p->asPlayer.currentAnim = 0; p->playerDirX = 1; p->playerDirY = 0;}
                    else if (InputMoveLeft()) {p->playerVel.x = -PLAYER_SPEED; p->asPlayer.currentAnim = 1; p->playerDirX = -1; p->playerDirY = 0;}
                    if (InputMoveDown()) {p->playerVel.y = PLAYER_SPEED; p->asPlayer.currentAnim = 2; p->playerDirX = 0; p->playerDirY = 1;}
                    else if (InputMoveUp()) {p->playerVel.y = -PLAYER_SPEED; p->asPlayer.currentAnim = 3; p->playerDirX = 0; p->playerDirY = -1;}
                    p->punchPos = p->playerPos;
                    if (InputPrimary()) {
                        p->playerPunching = 1;
                        p->asPlayer.frame = 0;
                        p->playerFloat = PUNCH_DURATION;
                        p->punchPos = (Vector2){p->playerPos.x + p->playerSize*p->playerDirX*0.5, p->playerPos.y + p->playerSize*p->playerDirY*0.5};
                        if (p->playerDirX >= 1) p->asPlayer.currentAnim = 4;
                        else if (p->playerDirX <= -1) p->asPlayer.currentAnim = 5;
                        if (p->playerDirY >= 1) p->asPlayer.currentAnim = 6;
                        else if (p->playerDirY <= -1) p->asPlayer.currentAnim = 7;
                        PlaySound(p->sound[GetRandomValue(SOUND_PUNCH1, SOUND_PUNCH3)]);
                    }
                    if (InputMoveRight() || InputMoveLeft() || InputMoveDown() || InputMoveUp()) p->asPlayer.playing = 1;
                    else p->asPlayer.playing = 0;
                    if (p->hasWhip) {
                        if (InputSecondary()) {
                            //p->playerWhipping = 1;
                            p->asPlayer.frame = 0;
                            if (p->playerDirX >= 1) p->asPlayer.currentAnim = 8;
                            else if (p->playerDirX <= -1) p->asPlayer.currentAnim = 9;
                            if (p->playerDirY >= 1) p->asPlayer.currentAnim = 10;
                            else if (p->playerDirY <= -1) p->asPlayer.currentAnim = 11;
                            p->asPlayer.playing = 1;
                        }
                    }
                    for (int i=0; i<p->currentRoom.doorCount; i++) {
                        if (CheckCollisionRecs( p->currentRoom.doors[i], PointToSquare(p->playerPos, p->playerSize))) {
                            p->playerPos = p->currentRoom.doorToPos[i];
                            //StopMusicStream(p->music[p->currentRoom.music]);
                            ChangeRoom(&(p->currentRoom), p->currentRoom.doorToRoom[i] );
                            //PlayMusicStream(p->music[p->currentRoom.music]);
                            if (p->currentRoom.hasQuest) {
                                p->asCowboy.currentAnim = p->currentRoom.quest;
                                if (p->currentRoom.quest == 2 && p->currentRoom.delivered)  p->asCowboy.currentAnim = p->currentRoom.quest+1;
                            }
                            if (p->currentRoom.id == ROOM_GRAVEYARD) {p->asExplosion.frame = -1;p->asExplosion.playing = 0;}
                            if (p->currentRoom.id == ROOM_DUNGEON) p->asPig.currentAnim = 2;
                            
                            break; 
                        }
                    }
                }
                if (p->playerAlive) {
                    if (p->noclip) {
                        p->playerPos.x += delta*p->playerVel.x;
                        p->playerPos.y += delta*p->playerVel.y;
                    } else {
                        if (p->currentRoom.id == ROOM_FOREST) MoveAndSlide(&(p->playerPos), p->playerSize, p->playerVel, p->currentRoom, 1, 0, 1, 1);
                        else MoveAndSlide(&(p->playerPos), p->playerSize, p->playerVel, p->currentRoom, 1, 0, 0, 1);
                    }
                    if (p->currentRoom.enemyType[2]==1) {
                        if (CheckCollisionRecs(PointToSquare(p->playerPos,p->playerSize),PointToSquare(p->currentRoom.enemyPos[2],p->currentRoom.enemySize))) {
                            GameOver(p);
                        }
                    } else if (p->currentRoom.enemyType[2]==2) {
                        for (int i=2; i<p->currentRoom.enemyCount; i++) {
                            if (p->currentRoom.spiderAlive[i-2] && CheckCollisionRecs(PointToSquare(p->playerPos,p->playerSize),PointToSquare(p->currentRoom.enemyPos[i],p->currentRoom.enemySize)) ) {
                                GameOver(p);
                            }
                        }
                    }
                    if (p->currentRoom.localKey >= 0 && p->currentRoom.localKey <= 2) {
                        if (CheckCollisionPointRec(p->currentRoom.keyPos,PointToSquare(p->playerPos,p->playerSize))) {
                            p->currentRoom.hasKey[p->currentRoom.localKey] = 1;
                            //p->currentRoom.keyPos = (Vector2){-100,-100};
                            p->currentRoom.localKey = -1;
                            PlaySound(p->sound[SOUND_KEY]);
                        }
                    }
                    if (p->currentRoom.id == ROOM_FOREST) {
                        if (!(p->currentRoom.openedDoor[0]) && p->currentRoom.hasKey[0] && p->openingDoor < 0) {
                            if (CheckCollisionRecs(PointToSquare(p->playerPos,p->playerSize),(Rectangle){p->currentRoom.walls[DOOR_AT_WALL[0]].x-10,p->currentRoom.walls[DOOR_AT_WALL[0]].y-10,p->currentRoom.walls[DOOR_AT_WALL[0]].width+20,p->currentRoom.walls[DOOR_AT_WALL[0]].height+20})) {
                                //p->currentRoom.openedDoor[i] = 1;
                                p->openingDoor = 0;
                                p->asDoor.playing = 1;
                                PlaySound(p->sound[SOUND_DOOR]);
                                //p->asDoor.frame = 0;
                                //break;
                            }
                        } else if (!(p->currentRoom.openedDoor[2]) && p->currentRoom.hasKey[2] && p->openingDoor < 0) {
                            if (CheckCollisionRecs(PointToSquare(p->playerPos,p->playerSize),(Rectangle){p->currentRoom.walls[DOOR_AT_WALL[2]].x-10,p->currentRoom.walls[DOOR_AT_WALL[2]].y-10,p->currentRoom.walls[DOOR_AT_WALL[2]].width+20,p->currentRoom.walls[DOOR_AT_WALL[2]].height+20})) {
                                //p->currentRoom.openedDoor[i] = 1;
                                p->openingDoor = 2;
                                p->asDoor.playing = 1;
                                PlaySound(p->sound[SOUND_DOOR]);
                                //p->asDoor.frame = 0;
                                //break;
                            }
                        }
                    } else if (p->currentRoom.id == ROOM_DUNGEON) {
                        if (!(p->currentRoom.openedDoor[1]) && p->currentRoom.hasKey[1] && p->openingDoor < 0) {
                            if (CheckCollisionRecs(PointToSquare(p->playerPos,p->playerSize),(Rectangle){p->currentRoom.walls[DOOR_AT_WALL[1]].x-10,p->currentRoom.walls[DOOR_AT_WALL[1]].y-10,p->currentRoom.walls[DOOR_AT_WALL[1]].width+20,p->currentRoom.walls[DOOR_AT_WALL[1]].height+20})) {
                                //p->currentRoom.openedDoor[i] = 1;
                                p->openingDoor = 1;
                                p->asDoor.playing = 1;
                                PlaySound(p->sound[SOUND_DOOR]);
                                //p->asDoor.frame = 0;
                                //break;
                            }
                        }
                    }
                } else {                        
                    if (p->asPlayer.frame >= 3) p->asPlayer.playing = 0;
                    if (p->respawnTimer > 0.0) p->respawnTimer -= delta;
                    else {
                        if (p->messageBool) {
                            p->message = 1;
                            p->messageBool = 0;
                            p->messageTimer = 6.0;
                        }
                        if (IsAnyKeyPressed()) Respawn(p);
                    }
                }
                if (p->asDoor.frame >= 4) {
                    p->currentRoom.openedDoor[p->openingDoor] = 1;
                    p->currentRoom.walls[DOOR_AT_WALL[p->openingDoor]] = (Rectangle){-100,-100,1,1};
                    p->asDoor.frame = 0;
                    p->asDoor.playing = 0;
                    p->openingDoor = -1;
                }
                if (p->currentRoom.id == ROOM_FOREST && p->currentRoom.capturedPigs && CheckCollisionPointRec((Vector2){2050,250},PointToSquare(p->playerPos,p->playerSize))) {
                    if (!(p->hasWhip)) {p->hasWhip = 1;PlaySound(p->sound[SOUND_KEY]);}
                    p->messageTimer = 4.0;
                    p->message = 2;
                }
                UpdateEnemies(&(p->currentRoom), p->playerPos, p->punchPos, p->playerPunching, p->whipRect, p->playerWhipping, p->cam, p->width, p->height);
                if (p->currentRoom.id == ROOM_GRAVEYARD && p->currentRoom.enemyCount <= 3 && p->currentRoom.enemyStun[2]>=0.25) PlaySound(p->sound[SOUND_MONSTER_OUCH]);
                if (p->currentRoom.id == ROOM_FOREST) { 
                    for (int i=2; i<6; i++) {
                        if (p->currentRoom.enemyStun[i]>=0.5) PlaySound(p->sound[SOUND_PIG]);
                    }
                }
                if (p->currentRoom.hasQuest) {
                    if (CheckCollisionPointCircle(p->playerPos, p->currentRoom.questPos, 100)) {
                        if (p->currentRoom.questFloat < 1.0) p->currentRoom.questFloat += 3.0*delta;
                        else if (p->currentRoom.questFloat > 1.0) p->currentRoom.questFloat = 1.0;
                        if (p->currentRoom.quest==2 && !(p->currentRoom.delivered)) {
                            p->currentRoom.delivered = 1; p->asCowboy.currentAnim++;
                            if (p->currentRoom.music != MUSIC_BABY_LAUGHING) p->currentRoom.music = MUSIC_BABY_LAUGHING;
                            if (IsMusicPlaying(p->music[MUSIC_BABY_CRYING])) StopMusicStream(p->music[MUSIC_BABY_CRYING]);
                            if (!IsMusicPlaying(p->music[MUSIC_BABY_LAUGHING])) PlayMusicStream(p->music[MUSIC_BABY_LAUGHING]);
                        }
                    } else {
                        if (p->currentRoom.questFloat > -1.0) p->currentRoom.questFloat -= delta;
                    }
                }
                ProcessCamera( &(p->cam), p->playerPos, p->currentRoom.boundary );
                // Baby sounds
                // Animations
                if (p->animTimer >= 1.0) {
                    UpdateAnimationSprite(&(p->asPlayer));
                    UpdateAnimationSprite(&(p->asPig));
                    UpdateAnimationSprite(&(p->asMonster));
                    UpdateAnimationSprite(&(p->asDoor));
                    if (p->currentRoom.monsterHealth<=0 && p->asExplosion.frame==-1 && p->currentRoom.id==ROOM_GRAVEYARD) {p->asExplosion.playing=1; p->asExplosion.frame=0; PlaySound(p->sound[SOUND_EXPLOSION]);}
                    if (p->asExplosion.frame >= 11) p->asExplosion.playing = 0;
                    UpdateAnimationSprite(&(p->asExplosion));
                    if (p->currentRoom.hasQuest) UpdateAnimationSprite(&(p->asCowboy));
                    if (p->asPlayer.currentAnim >= 8 && p->asPlayer.frame == 4) PlaySound(p->sound[SOUND_WHIP]);
                    if (!(p->playerAlive) && p->currentRoom.id == ROOM_GRAVEYARD) {
                        if (!(IsSoundPlaying(p->sound[SOUND_MONSTER_LAUGH])) && GetRandomValue(0,10)<2) PlaySound(p->sound[SOUND_MONSTER_LAUGH]);
                    }
                    if (p->currentRoom.id==ROOM_BUSHES && p->asCowboy.currentAnim == 3 && p->asCowboy.frame == 23) PlaySound(p->sound[SOUND_CHEW]);
                    p->animTimer = 0;
                }
                p->animTimer += delta * p->animSpeed;
                if (p->asPlayer.currentAnim>=8 && p->asPlayer.currentAnim<=11) { 
                    if (p->asPlayer.frame < 2) {
                        p->asPlayer.offset.x = 0.5;
                        p->asPlayer.offset.y = 0.75;
                    } else if (p->asPlayer.frame < 4) {
                        p->asPlayer.offset.x = 0.5-(0.25*(float)p->playerDirX);
                        if (p->playerDirY >= 1) p->asPlayer.offset.y = 0.375;
                        else if (p->playerDirY <= -1) p->asPlayer.offset.y = 1.0-(1.0/8.0);
                        else p->asPlayer.offset.y = 0.75;
                    } else {
                        p->asPlayer.offset.x = 0.5-((float)p->playerDirX/3);
                        if (p->playerDirY >= 1) p->asPlayer.offset.y = 0.25;
                        else if (p->playerDirY <= -1) p->asPlayer.offset.y = 1.0-(1.0/12.0);
                        else p->asPlayer.offset.y = 0.75;
                    }
                }
                if (p->messageTimer > 0.0) p->messageTimer -= delta;
                break;
            default: p->state = PS_SPLASHSCREEN;
        }
    }
    //
    BeginDrawing();
        ClearBackground(BLACK);
        // Drawing actual game in RenderTexture2D renderTarget
        //printf("%.2f\n",p->currentRoom.enemyPos[10]);
        BeginTextureMode(p->renderTarget);
            ClearBackground(DARKGRAY);
            // DRAW GAME
            
            switch (p->state) {
                case PS_SPLASHSCREEN:
                    ClearBackground(BLACK);
                    DrawTexture(p->texture[0], p->width/2 - p->texture[0].width/2, p->height/2 - p->texture[0].height/2, (Color){255,255,255,255*p->splashFade});
                    break;
                case PS_MENU:
                    ClearBackground((Color){20,20,40,255});
                    if (p->lang==L_GERMAN)DrawTextEx(p->font,"-",(Vector2){p->width*0.5-MeasureTextEx(p->font,"-",16,0).x*0.5,5},16,0,(Color){MORFYGREEN.r,MORFYGREEN.g,MORFYGREEN.b,200});
                    else DrawTextEx(p->font,"This game was made by morfy",(Vector2){p->width*0.5-MeasureTextEx(p->font,"This game was made by morfy",16,0).x*0.5,5},16,0,(Color){MORFYGREEN.r,MORFYGREEN.g,MORFYGREEN.b,200});
                    switch (p->menuState) {
                        case MS_MAIN:
                            for (int i=0; i < arrsize(p->menuMain); i++) {
                                Color color = WHITE;
                                if (i == p->selected) color = MORFYGREEN;
                                DrawTextShadowEx(p->font, p->menuMain[i][p->lang], (Vector2){MENU_POS_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                            }
                            switch (p->selected) {
                                case 0: 
                                    if (p->hasSave) {
                                        if (p->startedGame) {
                                            if (p->lang == L_GERMAN) DrawTextRec(p->font, "Momentanes Spiel weiterspielen.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                            else DrawTextRec(p->font, "Continue current game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        } else {
                                            if (p->lang == L_GERMAN) DrawTextRec(p->font, "Gespeicherten Spielstand weiterspielen.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                            else DrawTextRec(p->font, "Continue saved game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        }
                                    } else if (p->startedGame) {
                                        if (p->lang == L_GERMAN) DrawTextRec(p->font, "Momentanes Spiel weiterspielen.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        else DrawTextRec(p->font, "Continue current game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    } else {
                                        if (p->lang == L_GERMAN) DrawTextRec(p->font, "Kein gespeichertes Spiel zum weiterspielen. Starten Sie ein neues Spiel.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        else DrawTextRec(p->font, "No saved game to continue. Start a new game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    }
                                    break;
                                case 1: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Startet ein neues Spiel. Loescht nicht den gespeicherten Spielstand.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Starts a new game. Does not delete saved game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 2: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Speichert das momentanen Spiel. Ueberschreibt den vorhaerigen Spielstand.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Saves the current game. Overwrites the previous save game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 3: 
                                    if (p->hasSave) {
                                        if (p->lang == L_GERMAN) DrawTextRec(p->font, "Laedt den gespeicherten Spielstand.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        else DrawTextRec(p->font, "Loads the saved game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    } else {
                                        if (p->lang == L_GERMAN) DrawTextRec(p->font, "Kein Spielstand zum laden vorhanden.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        else DrawTextRec(p->font, "There is no saved game to load.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    }
                                    break;
                                case 6: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "-", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "A game by Morfy made in raylib.\nFox scream sound when saving is by InspectorJ from Freesound.org. His website: www.jshaw.co.uk\nFont 'RetroGaming' by Daymarius. From dafont.com.\nBaby sounds from Youtube Audio Library.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 7: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Beendet das spiel. Nicht gespeicherter Fortschritt geht verloren!", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Quits the game. Non-saved progress will be lost!", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                default: break;
                            }
                            break;
                        case MS_SETTINGS:
                            for (int i=0; i < arrsize(p->menuSettings); i++) {
                                Color color = WHITE;
                                if (i == p->selected) color = MORFYGREEN;
                                if (i==7) DrawTextShadowEx(p->font, p->menuSettings[i][p->lang], (Vector2){MENU_POS_X, MENU_POS_Y + i*30+30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else DrawTextShadowEx(p->font, p->menuSettings[i][p->lang], (Vector2){MENU_POS_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                if (i == 0) DrawTextShadowEx(p->font, TextFormat("%i",(int)(p->masterVol*100)), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 1) DrawTextShadowEx(p->font, TextFormat("%i",(int)(p->musicVol*100)), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 2) DrawTextShadowEx(p->font, TextFormat("%i",(int)(p->soundVol*100)), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 3) DrawTextShadowEx(p->font, BoolToString(IsWindowFullscreen(),p->lang), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 4) DrawTextShadowEx(p->font, TextFormat("%i",p->targetMonitor), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 5) DrawTextShadowEx(p->font, BoolToString(p->filterMode,p->lang), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 6) {
                                    if (p->lang == L_ENGLISH) DrawTextShadowEx(p->font, "English", (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                    else if (p->lang == L_GERMAN) DrawTextShadowEx(p->font, "Deutsch", (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                    else DrawTextShadowEx(p->font, "N/A", (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                }
                            }
                            switch (p->selected) {
                                case 0: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Die Hauptlautstaerke des Spiels.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "The master volume of the game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 1: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Die Musiklautstaerke des Spiels.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "The music volume of the game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 2: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Die Geraeuschlautstaerke des Spiels.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "The sound volume of the game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 3: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Schaltet Vollbildmodus ein und aus.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Toggles fullscreen mode.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 4: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Waehlt den Bildschirm in dem das Spiel erscheinen soll, wenn das Spiel im Vollbildmodus ist. Schalten Sie den Vollbildmodus um, um die Wirkung dieser Einstellungen zu sehen.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Sets the monitor in which the game should be visible when in fullscreen mode. Toggle fullscreen to see the effect of this.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 5: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Schaltet den Filtermodus ein und aus.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Toggles the filter mode.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 6: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Aendert die Sprache des Spiels.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Changes the language of the game.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                case 7: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Zurueck zum Hauptmenu.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Back to main menu.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                default: break;
                            }
                            break;
                        case MS_CONTROLS:
                            {   
                            int d = 0;
                            Color color = WHITE;
                            for (int i=0; i < arrsize(p->menuControls); i++) {
                                if (i == p->selected) color = MORFYGREEN;
                                else color = WHITE;
                                if (p->changingKey == i) color.a = 150;
                                if (i==6) DrawTextShadowEx(p->font, p->menuControls[i][p->lang], (Vector2){MENU_POS_X, MENU_POS_Y + i*30+30}, (Vector2){2,2}, 30, 0, (Color){color.r,color.g,color.b,255}, BLACK);
                                else DrawTextShadowEx(p->font, p->menuControls[i][p->lang], (Vector2){MENU_POS_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, (Color){color.r,color.g,color.b,255}, BLACK);
                                if (i == 0) DrawTextShadowEx(p->font, CodepointToString(keyRight, p->lang, &d), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 1) DrawTextShadowEx(p->font, CodepointToString(keyLeft,p->lang,&d), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 2) DrawTextShadowEx(p->font, CodepointToString(keyDown,p->lang,&d), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 3) DrawTextShadowEx(p->font, CodepointToString(keyUp,p->lang,&d), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 4) DrawTextShadowEx(p->font, CodepointToString(keyPrimary,p->lang,&d), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                                else if (i == 5) DrawTextShadowEx(p->font, CodepointToString(keySecondary,p->lang,&d), (Vector2){MENU_POS_X+MENU_GAP_X, MENU_POS_Y + i*30}, (Vector2){2,2}, 30, 0, color, BLACK);
                            }}
                            switch (p->selected) {
                                case 6: 
                                    if (p->lang == L_GERMAN) DrawTextRec(p->font, "Zurueck zum Hauptmenu.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    else DrawTextRec(p->font, "Back to main menu.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    break;
                                default:
                                    {
                                    int d = 0;
                                    if (p->changingKey<0) {
                                        if (p->lang == L_GERMAN) DrawTextRec(p->font, TextFormat("Waehlen Sie mit '%s' die Aktion, die Sie aendern wollen.",CodepointToString(keyPrimary,p->lang,&d)), MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        else DrawTextRec(p->font, TextFormat("Choose with '%s' the key you want to remap.",CodepointToString(keyPrimary,p->lang,&d)), MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    } else {
                                        if (p->lang == L_GERMAN) DrawTextRec(p->font, "Druecken Sie die erwuenschte Taste. Druecken Sie 'Escape', um ab zu brechen.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                        else DrawTextRec(p->font, "Press the key you want to set this to. Press 'Escape' to cancel.", MENU_INFO_REC, 16, 0, 1, LIGHTGRAY);
                                    }}
                                    break;
                            }
                            break;
                        default: break;
                    }
                    break;
                case PS_GAME:
                    ClearBackground(p->currentRoom.background);
                    BeginMode2D(p->cam);
                        if (p->currentRoom.id == ROOM_FOREST) {
                            if (CheckCollisionRecs((Rectangle){ p->cam.target.x-p->cam.offset.x, p->cam.target.y-p->cam.offset.y, p->width, p->height },(Rectangle){1500,100,800,600})) {
                                DrawRectangleRec((Rectangle){1500,100,800,600}, (Color){107,160,64,255});
                            }
                            if (CheckCollisionRecs((Rectangle){ p->cam.target.x-p->cam.offset.x, p->cam.target.y-p->cam.offset.y, p->width, p->height },(Rectangle){1600,200,600,400})) {
                                DrawRectangleRec((Rectangle){1600,200,600,400}, (Color){160,158,64,255});
                            }
                        }
                        if (p->currentRoom.id == ROOM_FOREST && p->currentRoom.capturedPigs && !(p->hasWhip)) {
                            DrawTextureRec(p->texture[TEXTURE_MISC], WHIP_RECT, (Vector2){2000,200}, WHITE);
                        }
                        if (!(p->playerAlive)) DrawAnimationSprite(p->texture[TEXTURE_MISC], p->asPlayer, p->playerPos, WHITE);
                        for (int i=0; i < p->currentRoom.wallCount; i++) {
                            //DrawTexturePro(p->texture[TEXTURE_TILES], p->currentRoom.wallType[i], p->currentRoom.walls[i], (Vector2){0,0}, 0, WHITE );
                            //DrawTextureQuad(p->texture[TEXTURE_TILES], (Vector2){(int)(p->currentRoom.walls[i].width/100), (int)(p->currentRoom.walls[i].height/100)}, (Vector2){0,0}, p->currentRoom.walls[i], WHITE );
                            DrawWall( p->texture[TEXTURE_TILES], p->currentRoom.walls[i], p->currentRoom.wallType[i], DRAWTYPE_TILED, p->cam, p->width, p->height );
                            if (p->showcol) {
                                DrawRectangleLinesEx(p->currentRoom.walls[i], 4, RED);
                                DrawRectangle( p->currentRoom.walls[i].x, p->currentRoom.walls[i].y, 30, 30, BLACK );
                                DrawTextEx(defaultFont, TextFormat("%i",i), (Vector2){p->currentRoom.walls[i].x,p->currentRoom.walls[i].y}, 24, 0, RED);
                            }
                        }
                        {
                        Rectangle camRect = (Rectangle){ p->cam.target.x-p->cam.offset.x, p->cam.target.y-p->cam.offset.y, p->width, p->height };
                        if (!(p->currentRoom.openedDoor[0]) && p->currentRoom.id == ROOM_FOREST) {
                            if (CheckCollisionRecs(camRect,p->currentRoom.walls[DOOR_AT_WALL[0]])) {
                                DrawAnimationSprite(p->texture[TEXTURE_MISC], p->asDoor, (Vector2){p->currentRoom.walls[DOOR_AT_WALL[0]].x,p->currentRoom.walls[DOOR_AT_WALL[0]].y}, KEY_COLOR[0]);
                            }
                        }
                        if (!(p->currentRoom.openedDoor[1]) && p->currentRoom.id == ROOM_DUNGEON) {
                            if (CheckCollisionRecs(camRect,p->currentRoom.walls[DOOR_AT_WALL[1]])) {
                                DrawAnimationSprite(p->texture[TEXTURE_MISC], p->asDoor, (Vector2){p->currentRoom.walls[DOOR_AT_WALL[1]].x,p->currentRoom.walls[DOOR_AT_WALL[1]].y}, KEY_COLOR[1]);
                            }
                        }
                        if (!(p->currentRoom.openedDoor[2]) && p->currentRoom.id == ROOM_FOREST) {
                            if (CheckCollisionRecs(camRect,p->currentRoom.walls[DOOR_AT_WALL[2]])) {
                                DrawAnimationSprite(p->texture[TEXTURE_MISC], p->asDoor, (Vector2){p->currentRoom.walls[DOOR_AT_WALL[2]].x,p->currentRoom.walls[DOOR_AT_WALL[2]].y}, KEY_COLOR[2]);
                            }
                        }
                        if (p->currentRoom.localKey >= 0 && p->currentRoom.localKey <= 2) {
                            if (CheckCollisionRecs(PointToSquare(p->currentRoom.keyPos,100),camRect)) {
                                DrawTextureRec(p->texture[TEXTURE_MISC],(Rectangle){0,100,100,100},(Vector2){p->currentRoom.keyPos.x-50,p->currentRoom.keyPos.y-50},KEY_COLOR[p->currentRoom.localKey]);
                            }
                        }
                        if (p->currentRoom.hasQuest && CheckCollisionRecs(camRect,(Rectangle){p->currentRoom.questPos.x-100*p->asCowboy.offset.x,p->currentRoom.questPos.y-100*p->asCowboy.offset.y,100,100})) {
                            DrawAnimationSprite(p->texture[TEXTURE_NPC2], p->asCowboy, p->currentRoom.questPos, WHITE);
                        }
                        for (int i=2; i<p->currentRoom.enemyCount; i++) {
                            if (CheckCollisionRecs(PointToSquare(p->currentRoom.enemyPos[i], 100), camRect)) {
                                switch (p->currentRoom.enemyType[i]) {
                                    case 0:
                                        if (p->currentRoom.enemyVel[i].x > 1) p->asPig.currentAnim = 0;
                                        else if (p->currentRoom.enemyVel[i].x < -1) p->asPig.currentAnim = 1;
                                        if (p->currentRoom.enemyVel[i].y > 1) p->asPig.currentAnim = 2;
                                        else if (p->currentRoom.enemyVel[i].y < -1) p->asPig.currentAnim = 3;
                                        if (p->currentRoom.enemyStun[i] >= 0.3) DrawAnimationSprite(p->texture[TEXTURE_NPC1],p->asPig,p->currentRoom.enemyPos[i],(Color){255,175,235,255});
                                        else DrawAnimationSprite(p->texture[TEXTURE_NPC1],p->asPig,p->currentRoom.enemyPos[i],(Color){255,125,175,255});
                                        break;
                                    case 1:
                                        if (p->currentRoom.enemyVel[i].x > 1) p->asMonster.currentAnim = 0;
                                        else if (p->currentRoom.enemyVel[i].x < -1) p->asMonster.currentAnim = 1;
                                        if (p->currentRoom.enemyVel[i].y > 1) p->asMonster.currentAnim = 2;
                                        else if (p->currentRoom.enemyVel[i].y < -1) p->asMonster.currentAnim = 3;
                                        if (p->currentRoom.enemyStun[i] >= 0.15) DrawAnimationSprite(p->texture[TEXTURE_NPC3],p->asMonster,p->currentRoom.enemyPos[i],(Color){184,10,10,255});
                                        else DrawAnimationSprite(p->texture[TEXTURE_NPC3],p->asMonster,p->currentRoom.enemyPos[i],(Color){60,160,50,255});
                                        break;
                                        break;
                                    case 2:
                                        if (p->currentRoom.spiderAlive[i-2]) {
                                            if (p->currentRoom.enemyStun[i] >= 0.3) DrawAnimationSprite(p->texture[TEXTURE_NPC2], p->asPig, p->currentRoom.enemyPos[i], WHITE);
                                            else DrawAnimationSprite(p->texture[TEXTURE_NPC2], p->asPig, p->currentRoom.enemyPos[i], (Color){175,175,175,255});
                                        }
                                        break;
                                    default: break;
                                }
                            }
                        }}
                        if (p->playerAlive) {
                            if (p->dittoPantEnabled)  DrawAnimationSprite(p->texture[TEXTURE_DITTO_PANTS], p->asPlayer, p->playerPos, WHITE);
                            else DrawAnimationSprite(p->texture[TEXTURE_PLAYER], p->asPlayer, p->playerPos, WHITE);
                        }
                        if (p->asExplosion.playing && p->currentRoom.id == ROOM_GRAVEYARD) {
                            DrawAnimationSprite(p->texture[TEXTURE_MISC], p->asExplosion, p->currentRoom.keyPos, WHITE);
                        }
                        if (p->showcol) {
                            if (p->currentRoom.hasQuest) {
                                DrawRectangleRec(PointToSquare(p->currentRoom.questPos, p->currentRoom.questSize), (Color){255,0,0,150});
                                DrawRectangleRec(p->currentRoom.questBox, (Color){255,255,255,50});
                            }
                            DrawRectangleRec( PointToSquare(p->playerPos, p->playerSize), (Color){255,0,0,150} );
                            if (p->asPlayer.currentAnim>=4 && p->asPlayer.currentAnim<=7) {
                                Color color;
                                if (p->playerFloat<=PUNCH_DURATION-PUNCH_COLLISION_DURATION) color = (Color){255,0,0,30};
                                else color = (Color){255,0,0,150};
                                DrawCircleV(p->punchPos, PUNCH_SIZE, color);
                            }
                            if (p->playerWhipping) DrawRectangleRec(p->whipRect, (Color){255,0,0,150});
                            DrawCircleV( p->playerPos, 2, RED );
                            for (int i=0; i < p->currentRoom.enemyCount; i++) {
                                Color color;
                                if (p->currentRoom.enemyStun[i] >= 0) color = (Color){255,255,255,150};
                                else color = (Color){255,0,0,150};
                                DrawRectangleRec(PointToSquare(p->currentRoom.enemyPos[i], p->currentRoom.enemySize), color);
                                DrawTextEx(defaultFont, TextFormat("%i",i), p->currentRoom.enemyPos[i], 24, 0, BLACK);
                            }
                            for (int i=0; i<p->currentRoom.doorCount; i++) {
                                DrawRectangleLinesEx(p->currentRoom.doors[i], 4, BLUE);
                                DrawRectangle( p->currentRoom.doors[i].x, p->currentRoom.doors[i].y, 30, 30, BLACK );
                                DrawTextEx(defaultFont, TextFormat("%i",i), (Vector2){p->currentRoom.doors[i].x,p->currentRoom.doors[i].y}, 24, 0, BLUE);
                            }
                        }
                        if (p->currentRoom.hasQuest) {
                            if (p->currentRoom.questFloat > 0.0) {
                                DrawRectangleRec(p->currentRoom.questBox, (Color){0,0,0,150*p->currentRoom.questFloat});
                                switch (p->currentRoom.quest) {
                                    case 0:
                                        if (!(p->hasWhip)) {
                                            if (p->currentRoom.capturedPigs) {
                                                if (p->lang == L_GERMAN) {
                                                    DrawTextRec(p->font, "Danke, mann! Kannst meine Peitsche haben.", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                    p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                                } else {
                                                    DrawTextRec(p->font, "Thanks, dude! You can have my whip.", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                    p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                                }
                                            } else {
                                                if (p->lang == L_GERMAN) {
                                                    DrawTextRec(p->font, "Yo, mein Vieh ist abgehauen! Ich geb' dir eine Kleinigkeit, wenn du es wieder einfaengst.", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                    p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                                } else {
                                                    DrawTextRec(p->font, "Yo, all of my pigs ran away! I'll give ya somethin' if you bring 'em back.", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                    p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                                }
                                            }
                                        } else {
                                            if (p->lang == L_GERMAN) {
                                                DrawTextRec(p->font, "Pass gut auf die Peitsche auf! Und danke fuer die Hilfe.", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                            } else {
                                                DrawTextRec(p->font, "Take care with the whip! And thanks for the help.", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                            }
                                        }
                                        break;
                                    case 1:
                                        if (p->currentRoom.killedSpiders >= 10) {
                                            if (p->lang == L_GERMAN) {
                                                DrawTextRec(p->font, "Danke, dass du die Spinnen erledigt hast. Kannst den Schluessel haben...", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                            } else {
                                                DrawTextRec(p->font, "Thanks for getting rid of the spiders. You can have the key...", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                            }
                                        } else {
                                            if (p->lang == L_GERMAN) {
                                                DrawTextRec(p->font, "Du bekommst eine Belohung, wenn du die Spinnen im Verliess loswirst....", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                            } else {
                                                DrawTextRec(p->font, "You'll get a reward, if you get rid of the spiders in the dungeon....", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                                p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                            }
                                        }
                                        break;
                                    case 2:
                                        if (p->lang == L_GERMAN) {
                                            DrawTextRec(p->font, "Legger-schmegger! Danke fuer die Torte!", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                            p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                        } else {
                                            DrawTextRec(p->font, "Yummy! Thanks for the pie!", (Rectangle){p->currentRoom.questBox.x+10, p->currentRoom.questBox.y+10, 
                                            p->currentRoom.questBox.width-20, p->currentRoom.questBox.height-20}, 16, 0, 1, (Color){255,255,255,255*p->currentRoom.questFloat});
                                        }
                                        break;
                                    default: break;
                                }
                            }
                        }
                    EndMode2D();
                    if (p->currentRoom.fog.a > 0) DrawRectangle(0, 0, p->width, p->height, p->currentRoom.fog);
                    if (p->messageTimer > 0.0) {
                        switch (p->message) {
                            case 0:
                                {
                                int d = 0;
                                if (p->lang == L_GERMAN) {
                                    DrawRectangleRec(p->messageRect, (Color){0,0,0,150*ClampFloat(p->messageTimer,0.0,1.0)});
                                    DrawTextRec(p->font, TextFormat("\n%s, %s, %s, %s zum Bewegen\n\n%s zum Schlagen",CodepointToString(keyRight,p->lang,&d),CodepointToString(keyLeft,p->lang,&d),CodepointToString(keyDown,p->lang,&d),CodepointToString(keyUp,p->lang,&d),CodepointToString(keyPrimary,p->lang,&d)), (Rectangle){p->messageRect.x+20,p->messageRect.y+20,p->messageRect.width-40,p->messageRect.height-40}, 16, 0, 1, (Color){255,255,255,255*ClampFloat(p->messageTimer,0.0,1.0)});
                                } else {
                                    DrawRectangleRec(p->messageRect, (Color){0,0,0,150*ClampFloat(p->messageTimer,0.0,1.0)});
                                    DrawTextRec(p->font, TextFormat("\n%s, %s, %s, %s to move\n\n%s to punch",CodepointToString(keyRight,p->lang,&d),CodepointToString(keyLeft,p->lang,&d),CodepointToString(keyDown,p->lang,&d),CodepointToString(keyUp,p->lang,&d),CodepointToString(keyPrimary,p->lang,&d)), (Rectangle){p->messageRect.x+20,p->messageRect.y+20,p->messageRect.width-40,p->messageRect.height-40}, 16, 0, 1, (Color){255,255,255,255*ClampFloat(p->messageTimer,0.0,1.0)});
                                }}
                                break;
                            case 1:
                                if (p->lang == L_GERMAN) {
                                    DrawRectangleRec(p->messageRect, (Color){0,0,0,150*ClampFloat(p->messageTimer,0.0,1.0)});
                                    DrawTextRec(p->font, "GAME OVER\n\nBeliebige Taste druecken", (Rectangle){p->messageRect.x+20,p->messageRect.y+20,p->messageRect.width-40,p->messageRect.height-40}, 16, 0, 1, (Color){255,255,255,255*ClampFloat(p->messageTimer,0.0,1.0)});
                                } else {
                                    DrawRectangleRec(p->messageRect, (Color){0,0,0,150*ClampFloat(p->messageTimer,0.0,1.0)});
                                    DrawTextRec(p->font, "GAME OVER\n\nPress any key to retry", (Rectangle){p->messageRect.x+20,p->messageRect.y+20,p->messageRect.width-40,p->messageRect.height-40}, 16, 0, 1, (Color){255,255,255,255*ClampFloat(p->messageTimer,0.0,1.0)});
                                }
                                break;
                            case 2:
                                {
                                int d = 0;
                                if (p->lang == L_GERMAN) {
                                    DrawRectangleRec(p->messageRect, (Color){0,0,0,150*ClampFloat(p->messageTimer,0.0,1.0)});
                                    DrawTextRec(p->font, TextFormat("Du hast jetzt eine Peitsche!\n\nDruecke %s, um die Peitsche zu benutzen",CodepointToString(keySecondary,p->lang,&d)), (Rectangle){p->messageRect.x+20,p->messageRect.y+20,p->messageRect.width-40,p->messageRect.height-40}, 16, 0, 1, (Color){255,255,255,255*ClampFloat(p->messageTimer,0.0,1.0)});
                                } else {
                                    DrawRectangleRec(p->messageRect, (Color){0,0,0,150*ClampFloat(p->messageTimer,0.0,1.0)});
                                    DrawTextRec(p->font, TextFormat("You got a whip!\n\nPress %s to use it",CodepointToString(keySecondary,p->lang,&d)), (Rectangle){p->messageRect.x+20,p->messageRect.y+20,p->messageRect.width-40,p->messageRect.height-40}, 16, 0, 1, (Color){255,255,255,255*ClampFloat(p->messageTimer,0.0,1.0)});
                                }}
                                break;
                            default:
                                //p->messageTimer = 0.0;
                                break;
                        }
                    }
                    //p->cam.target = p->playerPos;
                    break;
                default: break;
            }
            
            if (p->saveTextFloat>0.0) {
                if (p->lang == L_GERMAN) {
                    DrawTextShadowEx(p->font,"Spiel gespeichert.",(Vector2){p->width*0.1,p->height*0.9},(Vector2){2,1},18,0,
                    (Color){255,255,255,ClampFloat(p->saveTextFloat,0.0,1.0)*255},(Color){0,0,0,ClampFloat(p->saveTextFloat,0.0,1.0)*255});
                } else {
                    DrawTextShadowEx(p->font,"Saved game.",(Vector2){p->width*0.1,p->height*0.9},(Vector2){2,1},18,0,
                    (Color){255,255,255,ClampFloat(p->saveTextFloat,0.0,1.0)*255},(Color){0,0,0,ClampFloat(p->saveTextFloat,0.0,1.0)*255});
                }
            }
            // Draw devmode
            if (p->devmode) {
                if (p->devinfo) {
                    //DrawRectangle(0,0,128,22,(Color){0,0,0,50});
                    DrawTextShadow(TextFormat("FPS: %i", GetFPS()), 6, 40, 1, 2, 24, RAYWHITE, BLACK);
                    DrawTextShadow(TextFormat("STATE: %i", p->state), 6, 64, 1, 2, 24, RAYWHITE, BLACK);
                    DrawTextShadow(TextFormat("POS: %.2f, %.2f", p->playerPos.x, p->playerPos.y), 6, 88, 1, 2, 24, RAYWHITE, BLACK);
                    DrawTextShadow(TextFormat("Monster Health: %i", p->currentRoom.monsterHealth), 6, 112, 1, 2, 24, RAYWHITE, BLACK);
                    DrawTextShadow(TextFormat("Spiders: %i", 10-p->currentRoom.killedSpiders), 6, 136, 1, 2, 24, RAYWHITE, BLACK);
                    DrawTextShadow(TextFormat("key: %i", GetKeyPressed()), p->width-100, 0, 1, 2, 24, GRAY, BLACK);
                }
                if (p->console) DrawConsole(*p);
            }
        EndTextureMode();
        // Drawing game (RenderTexture2D) depending on scaleType
        switch (p->scaleType) {
            case ST_STRETCHING: // Scale game, strecthing it to window
                DrawTexturePro(p->renderTarget.texture,
                (Rectangle){0,0,p->renderTarget.texture.width,-p->renderTarget.texture.height},
                (Rectangle){0,0,GetScreenWidth(),GetScreenHeight()},(Vector2){0,0},0,WHITE);
                break;
            case ST_KEEPING: // Scale game, keep ratio and black bars on the side
                DrawTexturePro(p->renderTarget.texture,
                (Rectangle){0,0,p->renderTarget.texture.width,-p->renderTarget.texture.height},
                (Rectangle){(GetScreenWidth()-((float)p->width*p->scale))*0.5, (GetScreenHeight()-((float)p->height*p->scale))*0.5,
                (float)p->width*p->scale, (float)p->height*p->scale},(Vector2){0,0},0,WHITE);
                break;
            default: // Don't scale, game always in top-left corner
                DrawTexturePro(p->renderTarget.texture,
                (Rectangle){0,0,p->renderTarget.texture.width,-p->renderTarget.texture.height},
                (Rectangle){0,0,p->width,p->height},(Vector2){0,0},0,WHITE);
        }
    EndDrawing();
}


void UpdateConsole(Project * p) {
    //float mDelta = delta / p->timescale;
    p->consoleTimer += (delta / p->timescale)*7;
    if (IsKeyPressed(KEY_ESCAPE)) {
        p->console = 0;
        if (p->state == PS_MENU) p->state = PS_GAME;
        else if (p->state == PS_GAME) p->state = PS_MENU;
    }
    if (IsKeyPressed(KEY_RIGHT) && p->consolePen < arrsize(p->consoleInput)) p->consoleTimer=1;
    else if (IsKeyPressed(KEY_LEFT) && p->consolePen > 0) p->consoleTimer=1;
    int key = GetKeyPressed();
    if (key>=32 && key<=125) {
        p->consoleInput[p->consolePen] = (char)key;
        if (p->consolePen < arrsize(p->consoleInput)) p->consolePen++;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        p->consoleTimer = 1;
    } else if (IsKeyPressed(KEY_DELETE)) {
        for (int i=0;i<arrsize(p->consoleInput);i++) p->consoleInput[i] = ' ';
        p->consolePen = 0;
    } else if (IsKeyPressed(KEY_ENTER)) {
        int a[2] = {2,2};
        if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,11)),"fullscreen ")) ToggleFullscreenPro(*p);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,9)),"forceres ")) p->forceRes=CharToInteger(p->consoleInput[9]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,10)),"scaletype ")) p->scaleType=CharToInteger(p->consoleInput[10]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,11)),"filtermode ")) {
            if (CharToInteger(p->consoleInput[11])<=5 && CharToInteger(p->consoleInput[11])>=0) p->filterMode=CharToInteger(p->consoleInput[11]);
            else p->filterMode = 0;
            SetTextureFilter(p->renderTarget.texture, p->filterMode);
        }
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,8)),"devinfo ")) p->devinfo=CharToInteger(p->consoleInput[8]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,11)),"scalemouse ")) p->scaleMouse=CharToInteger(p->consoleInput[11]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"quit ")) {
            CloseProject(p);
        }
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,6)),"state ")) p->state=CharToInteger(p->consoleInput[6]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,10)),"targetmon ")) p->targetMonitor=ClampInteger(CharToInteger(p->consoleInput[10]),0,GetMonitorCount()-1);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,8)),"autores ")) p->autoRes=CharToInteger(p->consoleInput[8]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,7)),"fwidth ")) p->fullscreenWidth=ClampInteger(TextToInteger(SubText(p->consoleInput,7,4)),MIN_WIDTH, 9999);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,8)),"fheight ")) p->fullscreenHeight=ClampInteger(TextToInteger(SubText(p->consoleInput,8,4)),MIN_HEIGHT,9999);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"lang ")) p->lang=ClampInteger(CharToInteger(p->consoleInput[5]),0,MAX_LANG-1);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,8)),"showcol ")) p->showcol=CharToInteger(p->consoleInput[8]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,7)),"noclip ")) p->noclip=CharToInteger(p->consoleInput[7]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"zoom ")) {
            if (p->cam.zoom <= 0.5) p->cam.zoom = 1;
            else p->cam.zoom = 0.5;
        }
        //else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,10)),"timescale ")) p->timescale=0.001*(float)TextToInteger(SubText(p->consoleInput,10,4));
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,10)),"timescale ")) p->timescale= (float)TextToInteger(TextSplit(SubText(p->consoleInput,10,4), '.', a)[0]) + 0.1*(float)TextToInteger(TextSplit(SubText(p->consoleInput,10,4), '.', a)[1]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,10)),"animspeed ")) p->animSpeed= (float)TextToInteger(TextSplit(SubText(p->consoleInput,10,4), '.', a)[0]) + 0.1*(float)TextToInteger(TextSplit(SubText(p->consoleInput,10,4), '.', a)[1]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,7)),"soundv ")) {
            p->soundVol = (float)ClampInteger(TextToInteger(SubText(p->consoleInput, 7, 3)), 0, 100)*0.01;
            for (int i=0; i<MAX_SOUND_COUNT; i++) SetSoundVolume(p->sound[i], p->soundVol); 
        } else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,9)),"musicv ")) {
            p->musicVol = (float)ClampInteger(TextToInteger(SubText(p->consoleInput, 9, 3)), 0, 100)*0.01;
            for (int i=0; i<MAX_MUSIC_COUNT; i++) SetMusicVolume(p->music[i], p->musicVol); 
        } else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,8)),"masterv ")) {
            p->masterVol = (float)ClampInteger(TextToInteger(SubText(p->consoleInput, 8, 3)), 0, 100)*0.01;
            for (int i=0; i<MAX_MUSIC_COUNT; i++) SetMasterVolume(p->masterVol); 
        } else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"whip ")) p->hasWhip=CharToInteger(p->consoleInput[5]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"kill ")) GameOver(p);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,4)),"god ")) p->god=CharToInteger(p->consoleInput[4]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"key1 ")) p->currentRoom.hasKey[0]=CharToInteger(p->consoleInput[5]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"key2 ")) p->currentRoom.hasKey[1]=CharToInteger(p->consoleInput[5]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,5)),"key3 ")) p->currentRoom.hasKey[2]=CharToInteger(p->consoleInput[5]);
        else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,11)),"dittopants ")) {
            p->dittoPantEnabled = toggle(p->dittoPantEnabled);
        }
        //else if (TextIsEqual(TextToLower(TextSubtext(p->consoleInput,0,10)),"animspeed ")) p->animSpeed=0.001*(float)TextToInteger(SubText(p->consoleInput,10,4));
        for (int i=0;i<arrsize(p->consoleHistory);i++) p->consoleHistory[i] = p->consoleInput[i];
        for (int i=0;i<arrsize(p->consoleInput);i++) p->consoleInput[i] = ' ';
        p->consolePen = 0;
    } else if (IsKeyPressed(KEY_UP)) for (int i=0;i<arrsize(p->consoleInput);i++) p->consoleInput[i] = p->consoleHistory[i];
    if (p->consoleTimer >= 1) {
        if (IsKeyDown(KEY_RIGHT) && p->consolePen < arrsize(p->consoleInput)) p->consolePen++;
        else if (IsKeyDown(KEY_LEFT) && p->consolePen > 0) p->consolePen--;
        if (IsKeyDown(KEY_BACKSPACE)) {
            if (p->consolePen > 0) p->consolePen--;
            p->consoleInput[p->consolePen] = ' ';
        }
        p->consoleTimer = 0;
    }
}
void DrawConsole(Project p) {
    DrawRectangle(0, p.height - 34, p.width , 34, (Color){0,0,0,150});
    DrawRectangle(0, p.height - 68, p.width , 34, (Color){0,0,0,100});
    DrawTextEx(defaultFont, TextSubtext(p.consoleHistory,0,arrsize(p.consoleHistory)), (Vector2){10,p.height-66}, 30, 0, (Color){245,245,245,150});
    DrawTextEx(defaultFont, TextSubtext(p.consoleInput,0,arrsize(p.consoleInput)), (Vector2){10,p.height-32}, 30, 0, RAYWHITE);
    DrawRectangle(10+MeasureTextEx(defaultFont, TextSubtext(p.consoleInput,0,p.consolePen),30,0).x,p.height-5,15,5,RAYWHITE);
    if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,11)),"fullscreen ")) {
        DrawTextEx(defaultFont, TextFormat("%i",IsWindowFullscreen()),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,9)),"forceres ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.forceRes),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,10)),"scaletype ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.scaleType),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,11)),"filtermode ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.filterMode),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,8)),"devinfo ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.devinfo),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,11)),"scalemouse ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.scaleMouse),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"quit ")) {
        DrawTextEx(defaultFont, "// quits game", (Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,6)),"state ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.state),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,10)),"targetmon ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.targetMonitor),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,8)),"autores ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.autoRes),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,7)),"fwidth ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.fullscreenWidth),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,8)),"fheight ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.fullscreenHeight),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"lang ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.lang),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,8)),"showcol ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.showcol),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,7)),"noclip ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.noclip),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"zoom ")) {
        DrawTextEx(defaultFont, TextFormat("%.1f",p.cam.zoom),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,10)),"timescale ")) {
        DrawTextEx(defaultFont, TextFormat("%.1f",p.timescale),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,10)),"animspeed ")) {
        DrawTextEx(defaultFont, TextFormat("%.1f",p.animSpeed),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,7)),"soundv ")) {
        DrawTextEx(defaultFont, TextFormat("%.2f",p.soundVol),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,9)),"musicv ")) {
        DrawTextEx(defaultFont, TextFormat("%.2f",p.musicVol),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,8)),"masterv ")) {
        DrawTextEx(defaultFont, TextFormat("%.2f",p.masterVol),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"whip ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.hasWhip),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"kill ")) {
        DrawTextEx(defaultFont, "// kills you",(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,4)),"god ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.god),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"key1 ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.currentRoom.hasKey[0]),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"key2 ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.currentRoom.hasKey[1]),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,5)),"key3 ")) {
        DrawTextEx(defaultFont, TextFormat("%i",p.currentRoom.hasKey[2]),(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    } else if (TextIsEqual(TextToLower(TextSubtext(p.consoleInput,0,11)),"dittopants ")) {
        DrawTextEx(defaultFont, "// toggles dittopants",(Vector2){p.width-256,p.height-34},30,0,(Color){245,245,245,150});
    }
}



#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <dlfcn.h>
#include <cstdlib>
#include <cinttypes>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_android.h"
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/MemoryPatch.h"
#include "KittyMemory/KittyScanner.h"
#include "KittyMemory/KittyUtils.h"
#include"Includes/Dobby/dobbyForHooks.h"
#include <cmath>
#include "Include/Unity.h"
#include "Misc.h"
#include "Include/Vector3.h"
#include "hook.h"
#include "Include/Roboto-Regular.h"
#include "Include/RetroGaming.h"
#include "Include/Minecraftia-Regular.h"
#include "ESP.h"



#define GamePackageName "com.criticalforceentertainment.criticalops"

monoString* CreateIl2cppString(const char* str)
{
    static monoString* (*CreateIl2cppString)(const char* str, int *startIndex, int *length) =
    (monoString* (*)(const char* str, int *startIndex, int *length))(get_absolute_address(string2Offset(OBFUSCATE("0x44865B4"))));
    int* startIndex = 0;
    int* length = (int *)strlen(str);
    return CreateIl2cppString(str, startIndex, length);
}

void* pSys = nullptr;
void* pBones = nullptr;
void* localCharacter = nullptr;
bool unsafe,recoil, radar, flash, smoke, scope, setupimg, spread, aimpunch, speed, reload, esp, snaplines, kickback, crouch, wallbang,
        fov, ggod, killnotes,crosshair, supressor, rifleb, bonesp, viewmodel, viewmodelfov, boxesp, healthesp, healthNumber, espName, weaponEsp, armroFlag, spawnbullets,
        canmove,isPurchasingSkins, fly, removecharacter, tutorial, freeshop, gravity, dropweapon,ragdoll, crouchheight, cameraheight, interactionrange, jumpheight, bhop,
        noslow, god, ccollision, aimbot, freearmor;

float speedval = 1, fovModifier, viemodelposx, viemodelposy, viemodelposz, viewmodelfovval, gravityval, flyval ,crouchval, camval, jumpval;
int screenscale = 0;

extern int glHeight;
extern int glWidth;
ImFont* espFont;
ImFont* flagFont;

uintptr_t find_pattern_in_module(const char* lib_name, const char* pattern) {
    lib_info lib_info = find_library(lib_name);
    return find_pattern((uint8_t*)lib_info.start_address, lib_info.size, pattern);
}

uintptr_t find_pattern_with_lib(const char* pattern) {
    return find_pattern((uint8_t*)libBaseAddress, libSize, pattern);
}

float get_fieldOfView(void *instance) {
    if (instance != nullptr && fov) {
        return fovModifier;
    }
    return old_get_fieldOfView(instance);
}

void* getTransform(void* character)
{
    return *(void**)((uint64_t)character + 0x70);
}

int get_CharacterTeam(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0x150);
    return *(int*)((uint64_t)boxedValueName + 0x1C);
}

int get_PlayerTeam(void* player)
{
    void* boxedValueName = *(void**)((uint64_t)player + 0x150);
    return *(int*)((uint64_t)boxedValueName + 0x1C);
}

std::string get_CharacterName(void* character)
{
    void* player = get_Player(character);
    void* boxedValueName = *(void**)((uint64_t)player + 0xA8);
    monoString* username = *(monoString**)((uint64_t)boxedValueName + 0x19);
    return username->getString();
}

std::string get_characterWeaponName(void* character)
{
    void* characterData = *(void**)((uint64_t)character + 0x98);
    void* m_wpn = *(void**)((uint64_t)characterData + 0x80);
    if (m_wpn)
    {
        monoString* weaponName = *(monoString**)((uint64_t)m_wpn + 0x10);
        return weaponName->getString();
    }
    std::string filler = "";
    return filler;
}

Vector3 getBonePosition(void* character, int bone){
    void* curBone = get_CharacterBodyPart(character, bone);
    void* hitSphere = *(void**)((uint64_t)curBone + 0x20);
    void* transform = *(void**)((uint64_t)hitSphere + 0x30);
    Vector3 bonePos = get_Position(transform);
    return bonePos;
}

float NormalizeAngle (float angle){
    while (angle>360)
        angle -= 360;
    while (angle<0)
        angle += 360;
    return angle;
}

Vector2 NormalizeAngles (Vector2 angles)
{
    angles.X = NormalizeAngle(angles.X);
    angles.Y = NormalizeAngle(angles.Y);
    return angles;
}

int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
        if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
            package_name[0] = '\0';
            LOGW(OBFUSCATE("can't parse %s"), app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, GamePackageName) == 0) {
        LOGI(OBFUSCATE("detect game: %s"), package_name);
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}

void* ShaderFind(std::string name)
{
    LOGE("Shader logged: %s", name.c_str());
    oldShaderFind(name);
}

void GameSystemUpdate(void* obj){
    if(obj != nullptr){
        pSys = obj;
        void* GamePlayModule = *(void**)((uint64_t) obj + 0x80);
        if(GamePlayModule != nullptr){
            void* CameraSystem = *(void**)((uint64_t) GamePlayModule + 0x30);
            if(CameraSystem != nullptr){
                if(fov){
                    *(float*)((uint64_t) CameraSystem + 0x8C) = fovModifier;//m_horizontalFieldOfView
                }

                if(viewmodelfov){
                    *(float*)((uint64_t) CameraSystem + 0x90) = viewmodelfovval;//m_viewModelFieldOfView
                }
            }

            if(removecharacter){
                RemoveCharacter(obj, getLocalId(obj));
                removecharacter = false;
            }

            if (spawnbullets) {
                int id = getLocalId(pSys);
                void *localPlayer = getPlayer(pSys, id);
                int localTeam = get_PlayerTeam(localPlayer);
                // spawn bullets in ppls headlol
                monoList<void **> *characterList = getAllCharacters(pSys);
                void *localCharacter = nullptr;
                for (int i = 0; i < characterList->getSize(); i++) {
                    void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                    if (get_Player(currentCharacter) == localPlayer) {
                        localCharacter = currentCharacter;
                    }
                }
                for (int i = 0; i < characterList->getSize(); i++) {
                    void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
                    int curTeam = get_CharacterTeam(currentCharacter);
                    if (curTeam != localTeam) {
                        Vector3 headPos = getBonePosition(currentCharacter, 10);
                        Ray ray;
                        ray.origin = headPos;
                        ray.direction = Vector3(1, 1, 1);
                        if (localCharacter) {

                        }
                    }
                }
            }
        }
    }
    return oldGameSystemUpdate(obj);
}

void UpdateWeapon(void* obj, float deltatime){
    if(obj != nullptr) {
        void *CharacterData = *(void **) ((uint64_t) obj + 0x98);
        if (CharacterData != nullptr) {
            void *CharacterSettingsData = *(void **) ((uint64_t) CharacterData + 0x78);
            if (CharacterSettingsData != nullptr) {
                if (speed) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x14) = speedval;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x18) = speedval;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x1C) = speedval;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x2C) = speedval;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x30) = speedval;
                }

                if (bhop) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x38) = 20;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x20) = 20;
                    noslow = true;
                }

                if (crouchheight) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x40) = crouchval;
                }

                if (fly) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x3C) = flyval;
                }

                if (cameraheight) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x44) = camval;
                }

                if (interactionrange) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x48) = 100;
                }

                if (jumpheight) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x4C) = jumpval;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x50) = jumpval;
                }

                if (god) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x10) = 99999;
                }

                if (ccollision) {
                    *(bool *) ((uint64_t) CharacterSettingsData + 0x54) = true;
                }

                if (gravity) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x58) = gravityval;
                }

                if (noslow) {
                    *(float *) ((uint64_t) CharacterSettingsData + 0x60) = 0;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x64) = 0;
                    *(float *) ((uint64_t) CharacterSettingsData + 0x68) = 0;
                }
            }
        }
    }
    oldUpdateWeapon(obj, deltatime);
}

void RenderOverlayFlashbang(void* obj){
    if(obj != nullptr && flash){
        *(float*)((uint64_t) obj + 0x38) = 0;//m_flashTime
    }
    oldRenderOverlayFlashbang(obj);
}

void set_Spread(void* obj){
    if(obj != nullptr){
        *(float*)((uint64_t) obj + 0x24) = 0;//m_maxSpread
        *(float*)((uint64_t) obj + 0x20) = 0;//m_spreadFactor
    }
    oldset_Spread(obj);
}

void RenderOverlaySmoke(void* obj) {
    if (obj != nullptr && smoke) {
        *(float *) ((uint64_t) obj + 0x20) = 9999;//m_fadeSpeed
    }
    oldRenderOverlaySmoke(obj);
}

void DrawRenderer(void* obj){
    if(obj != nullptr){
        LOGE("DRENDER");
        void* m_offset = *(void**)((uint64_t) obj + 0x64);
        if(viewmodel){
            *(float*)((uint64_t)m_offset + 0x0) = viemodelposx;
            *(float*)((uint64_t)m_offset + 0x4) = viemodelposy;
            *(float*)((uint64_t)m_offset + 0x8) = viemodelposz;
        }
    }
    oldDrawRenderer(obj);
}

void(*oldFpsLimitUpdate)(void* obj);
void FpsLimitUpdate(void* obj){
    if(obj != nullptr){
        *(int*)((uint64_t) obj + 0x18) = 999;
    }
    oldFpsLimitUpdate(obj);
}


Vector3 get_gravity(){
    if(gravity){
        return gravityval;
    }
    oldget_gravity();
}

Vector3 get_height(){
    if(fly){
        return flyval;
    }
    oldget_height();
}

void Init(void* obj){
    if(obj != nullptr){
        LOGE("CALLED");
        void* GraphicsProfile = *(void**)((uint64_t) obj + 0x38);
        if(GraphicsProfile != nullptr){
            LOGE("GPROFILE");
            int scscale = (int)((uint64_t) GraphicsProfile + 0x30);
            LOGE("SSCALE, %d", scscale);
            screenscale = scscale / 100;
        }
    }
    oldInit(obj);
}

void BackendManager(void* obj){
    if(obj != nullptr && isPurchasingSkins){
        for (int i = 0; i < 9999; i++)
        {
            LOGE("trying to force purchase skins...");
            RequestPurchaseSkin(obj, i, 0, false);
        }
        isPurchasingSkins = false;
    }
    oldBackendManager(obj);
}

void setRotation(void* character, Vector2 rotation)
{
    Vector2 newAngle;
    if (character && localCharacter != nullptr)
    {
        LOGE("Closest ent calc");
        float closestEntDist = 99999.0f;
        void* closestCharacter = nullptr;
        monoList<void **> *characterList = getAllCharacters(pSys);
        for (int i = 0; i < characterList->getSize(); i++)
        {
            void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
            int curTeam = get_CharacterTeam(currentCharacter);
            int localTeam = get_CharacterTeam(localCharacter);
            int health = get_Health(currentCharacter);
            if (aimbot && localCharacter && health > 0 && localTeam != curTeam && curTeam != -1)
            {
                Vector3 localPosition = get_Position(getTransform(localCharacter));
                Vector3 currentCharacterPosition = get_Position(getTransform(currentCharacter));
                Vector3 currentEntDist = Vector3::Distance(localPosition, currentCharacterPosition);
                if (Vector3::Magnitude(currentEntDist) < closestEntDist)
                {
                    LOGE("Closest ent gotten");
                    closestCharacter = currentCharacter;
                }
            }

        }

        if (aimbot && closestCharacter)
        {
            Vector3 localHead = getBonePosition(localCharacter, 10);
            Vector3 enemyBone = getBonePosition(closestCharacter, 10);
            float adjacent = localHead.X - enemyBone.X;
            float opposite = localHead.Z - enemyBone.Z;
            float height = localHead.Y - enemyBone.Y;
            float hypotenuse = Vector3::Distance(localHead, enemyBone);

            Vector3 deltavec = enemyBone - localHead;
            float deltLength = sqrt(deltavec.X * deltavec.X + deltavec.Y + deltavec.Y + deltavec.Z + deltavec.Z);

            newAngle.X = -asin(deltavec.Y / deltLength) * (180.0 / PI);
            newAngle.Y = atan2(deltavec.X, deltavec.Z) * 180.0 / PI;

            LOGE("aimbot math");
        }
    }
    if(aimbot && character == localCharacter)
    {
        LOGE("setting angles");
        return oSetRotation(character, NormalizeAngles(newAngle));
    }


    LOGE("Angle X: %f, Y: %f", rotation.X, rotation.Y);

    oSetRotation(character, rotation);
}

void(*oldApplyKickBack)(void* obj, bool* applied);
void* ApplyKickBack(void* obj, bool* applied)
{
    if(obj != nullptr && recoil){
        *applied = true;
        return NULL;
    }
    oldApplyKickBack(obj, applied);
}

float(*oldFovWorld)(void* obj);
float FovWorld(void* obj){
    if(obj != nullptr && fov){
        return fovModifier;
    }
    return oldFovWorld(obj);
}

float(*oldFovViewModel)(void* obj);
float FovViewModel(void* obj){
    if(obj != nullptr && fov){
        return fovModifier;
    }
    return oldFovViewModel(obj);
}

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

// Initilizers with patterns <3
void Hooks()
{
    HOOK("0x1B872E8", BackendManager, oldBackendManager);
    HOOK("0x1BACFCC", set_Spread, oldset_Spread); // Overlay Scope set spread
    HOOK("0x1BACD84", RenderOverlayFlashbang, oldRenderOverlayFlashbang); // flash render overlay
    HOOK("0x1BB31C8", RenderOverlaySmoke, oldRenderOverlaySmoke); // smoke render overlay
    HOOK("0x106431C", GameSystemUpdate, oldGameSystemUpdate); // GameSystem Update
    HOOK("0x19DD034", UpdateWeapon, oldUpdateWeapon); // character
    HOOK("0x1051658", FovViewModel, oldFovViewModel); // speed
    HOOK("0x1051618", FovWorld, oldFovWorld); // speed
    HOOK("0x19D9890", setRotation, oSetRotation);
    //HOOK("0x1D6F028", ApplyKickBack, oldApplyKickBack); // speed
    //HOOK("0x1B9D608", DrawRenderer, oldDrawRenderer); need args
}

void Pointers()
{
    RequestPurchaseSkin = (void(*)(void*, int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1B80760")));
    SetResolution = (void(*)(int, int, bool)) get_absolute_address(string2Offset(OBFUSCATE("0x1A268A4"))); // SetResolution
    get_Width = (int(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A265AC"))); // screen get_Width
    get_Height = (int(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A265D4"))); // screen get_Height
    getAllCharacters = (monoList<void**>*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x105F4FC"))); // get_AllCharacters
    getLocalId= (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x10537AC"))); // get_LocalId
    getPlayer = (void*(*)(void*,int)) get_absolute_address(string2Offset(OBFUSCATE("0x10627C4"))); // GameSystem GetPlayer
    getLocalPlayer = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x105321C"))); // GameSystem get_LocalPlayer
    getCharacterCount = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x105F50C"))); // GameSystem get_CharacterCount
    get_Health = (int(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8E10"))); // get_Health
    get_Player = (void*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8DF8"))); // Gameplay Character get_Player
    get_IsInitialized = (bool(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8C80"))); // Gameplay Character get_IsInitialized
    get_Position = (Vector3(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A56A4C"))); // Transform get_position
    get_camera = (void*(*)()) get_absolute_address(string2Offset(OBFUSCATE("0x1A2E278"))); // get camera main
    WorldToScreen = (Vector3(*)(void*, Vector3, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1A2D67C"))); // WorldToScreenPoint
    //set_targetFrameRate = (void(*)(int)) get_absolute_address(""); NOT USED
    get_CharacterBodyPart =(void*(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x19D8E6C"))); // Character GetBodyPart
    getNameAndTag = (monoString*(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0xBEA754"))); // get_UsernameWithClanTag
    RaycastCharacter = (void(*)(void*,void*,Ray, int)) get_absolute_address(string2Offset(OBFUSCATE("0x1068A2C"))); // RaycastCharacters
    RemoveCharacter = (void(*)(void*, int)) get_absolute_address(string2Offset(OBFUSCATE("0x106194C")));//RemovePlayer
    getEularAngles = (Vector3(*)(void*)) get_absolute_address(string2Offset(OBFUSCATE("0x1A56CBC"))); // Transform get_eulerAngles
}

void Patches(){
    PATCH_SWITCH("0x1D6EF8C", "1F2003D5C0035FD6", spread);//UpdateSpread
    PATCH_SWITCH("0x10BEED4", "000080D2C0035FD6", recoil);//UpdateCameraShake
    PATCH_SWITCH("0x1D4FCC4", "000080D2C0035FD6", aimpunch);//AimPunchRecover
    PATCH_SWITCH("0x1BB86FC", "200080D2C0035FD6", canmove);//CanMove
    PATCH_SWITCH("0x1BB65D8", "200080D2C0035FD6", canmove);//CanShoot
    PATCH_SWITCH("0x1BB59D8", "200080D2C0035FD6", canmove);//ShootingAllowed
    PATCH_SWITCH("0x1BB59C4", "200080D2C0035FD6", canmove);//MovementAllowed
    PATCH_SWITCH("0x10D2C58", "1F2003D5C0035FD6", wallbang);//UpdateWallHit
    PATCH_SWITCH("0x10D2228", "1F2003D5C0035FD6", wallbang);//ProcessWallhit
    //  PATCH_SWITCH("0x10695C0", "1F2003D5C0035FD6", wallbang);//CheckWallHits
    PATCH_SWITCH("0x106724C", "000080D2C0035FD6", ggod);//GrenadeHitCharacter
    PATCH_SWITCH("0x1067820", "000080D2C0035FD6", ggod);//OnGrenadeExploded
    PATCH_SWITCH("0xF3CB7C", "000080D2C0035FD6", killnotes);//SetKillNotification
    PATCH_SWITCH("0x1DB3698", "1F2003D5C0035FD6", killnotes);//Init
    PATCH_SWITCH("0x1A7C36C", "200080D2C0035FD6", crosshair);//get_Crosshair weapondef
    PATCH_SWITCH("0x10D2F98", "1F2003D5C0035FD6", smoke);//SmokeGrenadeEffect
    PATCH_SWITCH("0x1D8507C", "200080D2C0035FD6", supressor);//isSupressor
    PATCH_SWITCH("0x19D93E4", "1F2003D5C0035FD6", crouch);//isSupressor
    PATCH_SWITCH("0x1BB5A00", "200080D2C0035FD6", dropweapon);//WeaponDroppingAllowed
    PATCH_SWITCH("0x1BB5A14", "200080D2C0035FD6", dropweapon);//WeaponPickupAllowed
    PATCH_SWITCH("0x19DFB00", "1F2003D5C0035FD6", ragdoll);//Ragdoll
    PATCH_SWITCH("0x208BB6C", "1F2003D5C0035FD6", freearmor);//ApplyRestorePriceToArmor
    PATCH_SWITCH("0x208BB28", "1F2003D5C0035FD6", freearmor);//ApplyNormalPriceToArmor
   // PATCH_SWITCH("0x1F96504", "200080D2C0035FD6", tutorial);//get_Tutorial
    PATCH("0x10DC594", "000080D2C0035FD6");//bad word
    PATCH("0x10DC680", "000080D2C0035FD6");//bad word
    PATCH("0x19DEC30", "200080D2C0035FD6");//character visi
    PATCH("0x1D6BBF8", "1F2003D5C0035FD6");//ProcessMovement
    PATCH("0xBDD6F4", "200080D2C0035FD6");//ValidatePosition
    PATCH("0xBDD810", "200080D2C0035FD6");//ValidateMovementDistance
    PATCH("0xBDD6CC", "1F2003D5C0035FD6");//HandleFailedMovement
    PATCH("0xBDD318", "000080D2C0035FD6");//ValidateMovementRequest
    PATCH("0xBDD258", "000080D2C0035FD6");//ValidateMovementRequest
    PATCH("0xA904CC", "1F2003D5C0035FD6");//ValidateMoveRequest
    PATCH("0x1D6BBF8", "1F2003D5C0035FD6");//ProcessMovement
    PATCH("0x1C04C30", "1F2003D5C0035FD6");//afk
    PATCH("0x1C4EA20", "1F2003D5C0035FD6");//afk
    PATCH("0xE6CE4C", "1F2003D5C0035FD6");//SetBanStatus
    PATCH("0x1E4F504", "000080D2C0035FD6");//t
    PATCH("0x1E4FEEC", "000080D2C0035FD6");//t
    PATCH("0x1E4F634", "000080D2C0035FD6");//t
    PATCH("0x1E500B0", "000080D2C0035FD6");//t
    PATCH("0x1E4F32C", "000080D2C0035FD6");//t

}

void DrawMenu(){
    if(pSys != nullptr){
        int id = getLocalId(pSys);
        void *localPlayer = getPlayer(pSys, id);
        int localTeam = get_PlayerTeam(localPlayer);
        float closestEntDist = 99999.0f;
        void* closestCharacter = nullptr;
        monoList<void **> *characterList = getAllCharacters(pSys);
        for (int i = 0; i < characterList->getSize(); i++) {
            void *currentCharacter = (monoList<void **> *) characterList->getItems()[i];
            if(get_Player(currentCharacter) == localPlayer)
            {
                localCharacter = currentCharacter;
            }
            int curTeam = get_CharacterTeam(currentCharacter);
            int health = get_Health(currentCharacter);
            if (health > 0 && get_IsInitialized(currentCharacter) && localTeam != curTeam &&
                curTeam != -1) {
                void *transform = getTransform(currentCharacter);
                Vector3 position = get_Position(transform);
                Vector3 transformPos = WorldToScreen(get_camera(), position, 2);
                transformPos.Y = glHeight - transformPos.Y;
                Vector3 headPos = getBonePosition(currentCharacter, HEAD);
                Vector3 chestPos = getBonePosition(currentCharacter, CHEST);
                Vector3 wschestPos = WorldToScreen(get_camera(), chestPos, 2);
                Vector3 wsheadPos = WorldToScreen(get_camera(), headPos, 2);
                Vector3 wsheadPos2 = wsheadPos;
                Vector3 aboveHead = headPos + Vector3(0, 0.2, 0); // estimate
                Vector3 headEstimate = position + Vector3(0, 1.48, 0);// estimate
                Vector3 wsAboveHead = WorldToScreen(get_camera(), aboveHead, 2);
                Vector3 wsheadEstimate = WorldToScreen(get_camera(), headEstimate, 2);
                wsAboveHead.Y = glHeight - wsAboveHead.Y;
                wsheadEstimate.Y = glHeight - wsheadEstimate.Y;
                float height = transformPos.Y - wsAboveHead.Y;
                float width = (transformPos.Y - wsheadEstimate.Y) / 2;
                int hitindex;
                //Vector3 currentheadPos = getBonePosition(, 10);
                // Vector3 enemyheadPos = getBonePosition(currentCharacter, 10);
                //Ray ray;
                //ray.origin = currentheadPos;
                //ray.direction = currentheadPos - enemyheadPos;
                // RaycastCharacter(pSys, currentCharacter, ray, hitindex);
                //LOGE("hitindex %d", hitindex);
                if (snaplines && transformPos.Z > 0) {
                    DrawLine(ImVec2(glWidth / 2, glHeight), ImVec2(transformPos.X, transformPos.Y),
                             ImColor(172, 204, 255), 3);
                }
                if (bonesp && transformPos.Z > 0) {
                    DrawBones(currentCharacter, LOWERLEG_LEFT, UPPERLEG_LEFT);
                    DrawBones(currentCharacter, LOWERLEG_RIGHT, UPPERLEG_RIGHT);
                    DrawBones(currentCharacter, UPPERLEG_LEFT, STOMACH);
                    DrawBones(currentCharacter, UPPERLEG_RIGHT, STOMACH);
                    DrawBones(currentCharacter, STOMACH, CHEST);
                    DrawBones(currentCharacter, LOWERARM_LEFT, UPPERARM_LEFT);
                    DrawBones(currentCharacter, LOWERARM_RIGHT, UPPERARM_RIGHT);
                    DrawBones(currentCharacter, UPPERARM_LEFT, CHEST);
                    DrawBones(currentCharacter, UPPERARM_RIGHT, CHEST);
                    Vector3 diff = wschestPos - wsheadPos;
                    Vector3 neck = (chestPos + headPos) / 2;
                    Vector3 wsneck = WorldToScreen(get_camera(), neck, 2);
                    wsneck.Y = glHeight - wsneck.Y;
                    wschestPos.Y = glHeight - wschestPos.Y;
                    wsheadPos.Y = glHeight - wsheadPos.Y;
                    if (wschestPos.Z > 0 && wsneck.Z) {
                        DrawLine(ImVec2(wschestPos.X, wschestPos.Y), ImVec2(wsneck.X, wsneck.Y), ImColor(172, 204, 255), 3);
                    }
                    if (wsheadPos.Z > 0 && wschestPos.Z > 0) {
                        float radius = sqrt(diff.X * diff.X + diff.Y * diff.Y);
                        auto background = ImGui::GetBackgroundDrawList();
                        background->AddCircle(ImVec2(wsheadPos.X, wsheadPos.Y), radius / 2, IM_COL32(172, 204, 255, 255), 0, 3.0f);
                    }
                }
                if (boxesp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    DrawOutlinedBox2(wsAboveHead.X - width / 2, wsAboveHead.Y, width, height, ImVec4(255, 255, 255, 255), 3);
                }
                if (healthesp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    DrawOutlinedFilledRect(wsAboveHead.X - width / 2 - 12, wsAboveHead.Y + height * (1 - (static_cast<float>(health) / 100.0f)), 3, height * (static_cast<float>(health) / 100.0f), HealthToColor(health));
                }
                if (healthNumber && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    if (health < 100) {
                        DrawText(ImVec2(wsAboveHead.X - width / 2 - 17, wsAboveHead.Y + height * (1 - static_cast<float>(health) / 100.0f) - 3),ImVec4(255, 255, 255, 255), std::to_string(health), espFont);
                    }
                }
                if (espName && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    void *player = get_Player(currentCharacter);
                    monoString *mono = getNameAndTag(player);
                    std::string name = mono->getString();
                    float compensation = name.length() * 4.0f;
                    DrawText(ImVec2(wsheadPos.X - compensation, wsAboveHead.Y - 20), ImVec4(255, 255, 255, 255), name, espFont);
                }
                if (weaponEsp && transformPos.Z > 0 && wsAboveHead.Z > 0) {
                    std::string weapon = get_characterWeaponName(currentCharacter);
                    // font is 15 px has 2 px outline so has 16px per character, to center the font we do
                    float compensation = weapon.length() * 4.0f;

                    DrawText(ImVec2(wsAboveHead.X - compensation, transformPos.Y + 7), ImVec4(255, 255, 255, 255), weapon, espFont);
                }
            }
        }
    }
    {
        if(unsafe){ImGui::Begin(OBFUSCATE("(UNSAFE HOOK) zyCheats Rage - 1.37.1f2091 - chr1s#4191 & 077 Icemods && faggosito"));}
        else{ImGui::Begin(OBFUSCATE("zyCheats Rage - 1.37.1f2091 - chr1s#4191 & 077 Icemods && faggosito"));}
        ImGui::TextUnformatted("If the menu is broken, set the screenscale in settings to 100.");
        if (ImGui::Button(OBFUSCATE("Join Discord")))
        {
            //isDiscordPressed = true;
        }
        ImGui::TextUnformatted("Its Recommended to join the discord server for mod updates etc.");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_FittingPolicyResizeDown;
        if (ImGui::BeginTabBar("Menu", tab_bar_flags)) {
            if (ImGui::BeginTabItem(OBFUSCATE("Legit"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("Weapon Mods"))) {
                    if (ImGui::CollapsingHeader(OBFUSCATE("Aimbot"))) {
                        ImGui::Checkbox(OBFUSCATE("Aemboat"), &aimbot);
                        if(aimbot){
                            //checkboxes for its properties
                        }
                    }
                    if(ImGui::Checkbox(OBFUSCATE("Less Recoil"), &recoil)){ Patches(); }
                    if(ImGui::Checkbox(OBFUSCATE("No Spread"), &spread)){ Patches(); }
                    if(ImGui::Checkbox(OBFUSCATE("Force Supressor"), &supressor)){ Patches(); }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(OBFUSCATE("Rage"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("Player Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("Godmode"), &god);
                    if(ImGui::Checkbox(OBFUSCATE("No Grenade Damage"), &ggod)){ Patches(); }
                    if( ImGui::Checkbox(OBFUSCATE("Spoof Crouch"), &crouch)){ Patches(); }
                    ImGui::Checkbox(OBFUSCATE("Speed"), &speed);
                    if(speed){
                        ImGui::SliderFloat(OBFUSCATE(" · Speed"), &speedval, 0.0, 20.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Jump Force"), &jumpheight);
                    if(speed){
                        ImGui::SliderFloat(OBFUSCATE(" · Force"), &jumpval, 0.0, 20.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("No Slow-Down"), &noslow);
                    ImGui::Checkbox(OBFUSCATE("BunnyHop"), &bhop);
                    ImGui::Checkbox(OBFUSCATE("Gravity"), &gravity);
                    if(gravity){
                        ImGui::SliderFloat(OBFUSCATE(" · Gravity"), &gravityval, 0.0, 100.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Player Height"), &fly);
                    if(fly){
                        ImGui::SliderFloat(OBFUSCATE(" · Player Height"), &flyval, 0.0, 10.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Crouch Height"), &crouch);
                    if(crouch){
                        ImGui::SliderFloat(OBFUSCATE(" · Crouch Height"), &crouchval, 0.0, 10.0);
                    }
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Weapon Mods"))) {
                    if(ImGui::Checkbox(OBFUSCATE("Wallbang"), &wallbang)){ Patches(); }
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Game Mods"))) {
                    if(ImGui::Checkbox(OBFUSCATE("Spawn Bullets In Enemy"), &spawnbullets));
                    if(ImGui::Checkbox(OBFUSCATE("Move/Shoot before timer"), &canmove)){ Patches(); }
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(OBFUSCATE("Visual"))) {
                if (ImGui::CollapsingHeader(OBFUSCATE("ESP"))) {
                    ImGui::Checkbox(OBFUSCATE("Snaplines"), &snaplines);
                    ImGui::Checkbox(OBFUSCATE("Bones"), &bonesp);
                    ImGui::Checkbox(OBFUSCATE("Boxes"), &boxesp);
                    ImGui::Checkbox(OBFUSCATE("Show Names"), &espName);
                    ImGui::Checkbox(OBFUSCATE("Show Health"), &healthesp);
                    if(healthesp){
                        ImGui::Checkbox(OBFUSCATE("Health Numbers"), &healthNumber);
                    }
                    ImGui::Checkbox(OBFUSCATE(" Show Armor"), &armroFlag);
                    ImGui::Checkbox(OBFUSCATE(" Show Weapon"), &weaponEsp);
                }
                if (ImGui::CollapsingHeader(OBFUSCATE("Camera Mods"))) {
                    ImGui::Checkbox(OBFUSCATE("Camera Height"), &cameraheight);
                    if(viewmodelfov){
                        ImGui::SliderFloat(OBFUSCATE(" · Viewmodel Value"), &camval, 1.0, 360.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("View Model FOV"), &viewmodelfov);
                    if(viewmodelfov){
                        ImGui::SliderFloat(OBFUSCATE(" · Viewmodel Value"), &viewmodelfovval, 1.0, 360.0);
                    }
                    ImGui::Checkbox(OBFUSCATE("Field Of View"), &fov);
                    if(fov){
                        ImGui::SliderFloat(OBFUSCATE(" · FOV Value"), &fovModifier, 1.0, 360.0);
                    }
                }
                if(ImGui::Checkbox(OBFUSCATE("Radar"), &radar)){ Patches(); }
                ImGui::Checkbox(OBFUSCATE("No Flashbang"), &flash);
                ImGui::Checkbox(OBFUSCATE("No Smoke"), &smoke);
                ImGui::Checkbox(OBFUSCATE("No Scope"), &scope);
                if(ImGui::Checkbox(OBFUSCATE("Force Crosshair"), &crosshair)){ Patches(); }
                if(ImGui::Checkbox(OBFUSCATE("No Aimpunch"), &aimpunch)){ Patches(); }
                if(ImGui::Checkbox(OBFUSCATE("Hide Kill Notifications"), &killnotes)){Patches();}
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Account"))) {
                if(ImGui::Button(OBFUSCATE("Purchase all skins"))){
                    isPurchasingSkins = true;
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(OBFUSCATE("Misc"))) {
                if(ImGui::Checkbox(OBFUSCATE("Disable Ragdoll"), &ragdoll)){ Patches(); }
                ImGui::Checkbox(OBFUSCATE("Interaction Range"), &interactionrange);
                ImGui::Checkbox(OBFUSCATE("Force Character Collision"), &ccollision);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    glWidth = get_Width();
    glHeight = get_Height();
    io.DisplaySize = ImVec2((float)glWidth , (float)glHeight);
    ImGui_ImplOpenGL3_Init("#version 100");
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(6.0f);
    io.Fonts->AddFontFromMemoryTTF(Roboto_Regular, 30, 30.0f);
    espFont = io.Fonts->AddFontFromMemoryCompressedTTF(RetroGaming, compressedRetroGamingSize, 15);
    flagFont = io.Fonts->AddFontFromMemoryCompressedTTF(Minecraftia_Regular, compressedMinecraftia_RegularSize, 15);
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);


    if (!setupimg) {
        SetupImgui();
        setupimg = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    DrawMenu();

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)glWidth, (int)glHeight);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

void *hack_thread(void *arg) {
    int tries = 0;
    do {
        sleep(1);
        auto maps =  KittyMemory::getMapsByName("split_config.arm64_v8a.apk");
        for(std::vector<ProcMap>::iterator it = maps.begin(); it != maps.end(); ++it) {
            auto address = KittyScanner::findHexFirst(it->startAddress, it->endAddress,"7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00 03 00 B7 00 01 00 00 00 D0 70 7A 00 00 00 00 00 40 00 00 00 00 00 00 00 B0 DB E2 02 00 00 00 00", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            if(address != 0)
            {
                libBaseAddress = address;
                libBaseEndAddress = it->endAddress;
                libSize = it->length;
            }
        }
        if (tries > 10)
        {
            auto map = KittyMemory::getLibraryBaseMap("libil2cpp.so");
            libBaseAddress = map.startAddress;
            libBaseEndAddress = map.endAddress;
            unsafe = true;
        }
        tries++;
    } while (libBaseAddress == 0);
    Hooks();
    Pointers();
    Patches();
    auto eglhandle = dlopen("libunity.so", RTLD_LAZY);
    auto eglSwapBuffers = dlsym(eglhandle, "eglSwapBuffers");
    DobbyHook((void*)eglSwapBuffers,(void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    void *sym_input = DobbySymbolResolver(("/system/lib/libinput.so"), ("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"));
    if (NULL != sym_input) {
        DobbyHook(sym_input,(void*)myInput,(void**)&origInput);
    }
    return nullptr;
}

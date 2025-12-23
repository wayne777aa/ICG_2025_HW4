#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "header/cube.h"
#include "header/Object.h"
#include "header/shader.h"
#include "header/stb_image.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateCamera();
void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta);
unsigned int loadCubemap(std::vector<std::string> &mFileName);

struct material_t{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float gloss;
};

struct light_t{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct camera_t{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::vec3 target;

    float yaw;
    float pitch;
    float radius;
    float minRadius;
    float maxRadius;
    float orbitRotateSpeed;
    float orbitZoomSpeed;
    float minOrbitPitch;
    float maxOrbitPitch;
    bool enableAutoOrbit;
    float autoOrbitSpeed;
}; 
// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// cube map 
unsigned int cubemapTexture;
unsigned int cubemapVAO, cubemapVBO;

// shader programs 
int shaderProgramIndex = 0;
std::vector<shader_program_t*> shaderPrograms;
shader_program_t* cubemapShader;

light_t light;
material_t material;
camera_t camera;

Object* staticModel = nullptr;
Object* staticModel2 = nullptr;
Object* cubeModel = nullptr;
bool isCube = false;
glm::mat4 modelMatrix(1.0f);
glm::mat4 modelMatrix2(1.0f);
glm::mat4 originModelMatrix(1.0f);
glm::mat4 originModelMatrix2(1.0f);
glm::mat4 sphereMatrix(1.0f);
float model2RotateSpeed = -60.0f;
float currentTime = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float timeCounter = 0.0f;
float sceneStartTime = 0.0f;
int scene = 0;
int lastScene = 0;
bool sphereActive = true;
bool staticModelStopped = false;
bool startCrash = false;// for scene 5,7 GS process
float sphereT = 0.0f;
bool startAnimation = false;

// explosion effect parameters
float explodeBlastTime = 0.6f;
float explodeHoldTime = 0.5f;
float explodeReturnTime = 2.0f;

float explodeStrength = 200.0f;
// ===== model2 only (explosion + wire) =====
bool model2EnableWire = false; // 是否畫線框（wire pass）
bool s5TriggeredExplode = false;

bool  model2ExplodeActive = false; // 實體爆出去
float model2ExplodeStart  = 0.0f;

bool  model2WireOnly      = false; // 不畫實體，只畫線
bool  model2WireCollapse  = false; // 線條縮回中
float model2WireStart     = 0.0f;

// ===== scene7 wire-explode -> swap to model1 =====
bool  s7WireExploding = false;
float s7WireStart = 0.0f;

bool  s7Holding = false;       // 最大狀態停住
float s7HoldStart = 0.0f;      // hold 起點

bool  s7BecameModel1 = false;  // scene7 後半段：用 model1 取代 model2
bool  s7Collapsing = false;
float s7CollapseStart = 0.0f;

// wire frame mode
shader_program_t* wireShader = nullptr;
float wireWidth = 1.0f;
glm::vec3 wireColor = glm::vec3(0.1f, 0.5f, 0.9f);

void model_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string obj_path = "..\\..\\src\\asset\\obj\\Mei_Run.obj";
    std::string texture_path = "..\\..\\src\\asset\\texture\\Mei_TEX.png";
#else
    std::string obj_path = "..\\..\\src\\asset\\obj\\v2reimu_low.obj";
    std::string obj2_path = "..\\..\\src\\asset\\obj\\cirno_low.obj";
    std::string texture_path = "..\\..\\src\\asset\\texture\\v2reimu_low_Material_u1_v1_Base_Color.png";
    std::string texture2_path = "..\\..\\src\\asset\\texture\\cirno_low_u1_v1.jpeg";
    std::string cube_obj_path = "..\\..\\src\\asset\\obj\\cube.obj";
    std::string cube_texture_path = "..\\..\\src\\asset\\texture\\red-paper-texture.jpg";
#endif

    staticModel = new Object(obj_path);
    staticModel->loadTexture(texture_path);
    staticModel2 = new Object(obj2_path);
    staticModel2->loadTexture(texture2_path);
    cubeModel = new Object(cube_obj_path);
    cubeModel->loadTexture(cube_texture_path);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -10.0f));
    originModelMatrix = modelMatrix;

    modelMatrix2 = glm::mat4(1.0f);
    modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(10.0f));
    modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(0.0f, 0.0f, -10.0f));
    originModelMatrix2 = modelMatrix2;
}

void camera_setup(){
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.yaw = 90.0f;
    camera.pitch = 10.0f;
    camera.radius = 400.0f;
    camera.minRadius = 150.0f;
    camera.maxRadius = 800.0f;
    camera.orbitRotateSpeed = 60.0f;
    camera.orbitZoomSpeed = 400.0f;
    camera.minOrbitPitch = -80.0f;
    camera.maxOrbitPitch = 80.0f;
    camera.target = glm::vec3(0.0f);
    camera.enableAutoOrbit = true;
    camera.autoOrbitSpeed = 0.0f;

    updateCamera();
}

void updateCamera(){
    float yawRad = glm::radians(camera.yaw);
    float pitchRad = glm::radians(camera.pitch);
    float cosPitch = cos(pitchRad);

    camera.position.x = camera.target.x + camera.radius * cosPitch * cos(yawRad);
    camera.position.y = camera.target.y + camera.radius * sin(pitchRad);
    camera.position.z = camera.target.z + camera.radius * cosPitch * sin(yawRad);

    camera.front = glm::normalize(camera.target - camera.position);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta) {
    camera.yaw += yawDelta;
    camera.pitch = glm::clamp(camera.pitch + pitchDelta, camera.minOrbitPitch, camera.maxOrbitPitch);
    camera.radius = glm::clamp(camera.radius + radiusDelta, camera.minRadius, camera.maxRadius);
    updateCamera();
}

void light_setup(){
    light.position = glm::vec3(1000.0, 1000.0, 0.0);
    light.ambient = glm::vec3(1.0);
    light.diffuse = glm::vec3(1.0);
    light.specular = glm::vec3(1.0);
}

void material_setup(){
    material.ambient = glm::vec3(0.5);
    material.diffuse = glm::vec3(1.0);
    material.specular = glm::vec3(0.7);
    material.gloss = 50.0;
}

int getScene(){
    // return 7;
    if(timeCounter < 4.0f)
        return 1;
    else if(timeCounter < 8.0f)
        return 2;
    else if(timeCounter < 11.0f)
        return 3;
    else if(timeCounter < 14.0f)
        return 4;
    else if(timeCounter < 19.0f)
        return 5;
    else if(timeCounter < 22.0f)
        return 6;
    else if(timeCounter < 27.0f)
        return 7;
    else if(timeCounter < 40.0f)
        return 8;
    else{
        timeCounter = 0.0f;
        return 0;
    }
}

void shader_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string shaderDir = "..\\..\\src\\shaders\\";
#else
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> shadingMethod = {
        "default", "bling-phong"
    };

    for(int i=0; i<shadingMethod.size(); i++){
        std::string vpath = shaderDir + shadingMethod[i] + ".vert";
        std::string fpath = shaderDir + shadingMethod[i] + ".frag";

        shader_program_t* shaderProgram = new shader_program_t();
        shaderProgram->create();
        shaderProgram->add_shader(vpath, GL_VERTEX_SHADER);
        shaderProgram->add_shader(fpath, GL_FRAGMENT_SHADER);
        shaderProgram->link_shader();
        shaderPrograms.push_back(shaderProgram);
    }

    // ===== explosion program (index = 2) =====
    {
        auto prog = new shader_program_t();
        prog->create();
        std::string v = shaderDir + "explode.vert";
        std::string g = shaderDir + "explode.geom";
        std::string f = shaderDir + "explode.frag";
        prog->add_shader(v, GL_VERTEX_SHADER);
        prog->add_shader(g, GL_GEOMETRY_SHADER);
        prog->add_shader(f, GL_FRAGMENT_SHADER);
        prog->link_shader();
        shaderPrograms.push_back(prog);
    }

    // ===== wireframe program =====
    {
        std::string v = shaderDir + "wire.vert";
        std::string g = shaderDir + "wire.geom";
        std::string f = shaderDir + "wire.frag";
        wireShader = new shader_program_t();
        wireShader->create();
        wireShader->add_shader(v, GL_VERTEX_SHADER);
        wireShader->add_shader(g, GL_GEOMETRY_SHADER);
        wireShader->add_shader(f, GL_FRAGMENT_SHADER);
        wireShader->link_shader();
    }
}

void cubemap_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string cubemapDir = "..\\..\\src\\asset\\texture\\skybox\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";
#else
    std::string cubemapDir = "..\\..\\src\\asset\\texture\\tonhou\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> faces
    {
        cubemapDir + "px.jpg",
        cubemapDir + "nx.jpg",
        cubemapDir + "py.jpg",
        cubemapDir + "ny.jpg",
        cubemapDir + "pz.jpg",
        cubemapDir + "nz.jpg"
    };
    cubemapTexture = loadCubemap(faces);   

    std::string vpath = shaderDir + "cubemap.vert";
    std::string fpath = shaderDir + "cubemap.frag";
    
    cubemapShader = new shader_program_t();
    cubemapShader->create();
    cubemapShader->add_shader(vpath, GL_VERTEX_SHADER);
    cubemapShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    cubemapShader->link_shader();

    glGenVertexArrays(1, &cubemapVAO);
    glGenBuffers(1, &cubemapVBO);
    glBindVertexArray(cubemapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void setup(){
    light_setup();
    model_setup();
    shader_setup();
    camera_setup();
    cubemap_setup();
    material_setup();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

void update(){   
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;
    if (startAnimation) {
        timeCounter += deltaTime;
    }
    lastScene = scene;
    scene = getScene();

    if(scene != lastScene){
        staticModelStopped = false;
        sphereActive = true;
        sphereT = 0.0f;
        sceneStartTime = currentTime;
        camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.yaw = 90.0f;
        camera.pitch = 10.0f;
        camera.radius = 400.0f;
        updateCamera();

        // 只重置「這一幕會用到的狀態」
        if(scene == 5){
            s5TriggeredExplode  = false;
            model2ExplodeActive = false;
            model2WireOnly      = false;
            model2EnableWire    = false;
        }
        if(scene == 7){
            s7WireExploding = false;
            s7WireStart     = 0.0f;

            s7Holding       = false;
            s7HoldStart     = 0.0f;
            s7BecameModel1  = false;

            s7Collapsing    = false;
            s7CollapseStart = 0.0f;

            model2ExplodeActive = false;
            model2WireOnly      = true;
            model2EnableWire    = true;
            model2WireCollapse  = false;
        }
    }
    if (camera.enableAutoOrbit) {
        float yawDelta = camera.autoOrbitSpeed * deltaTime;
        applyOrbitDelta(yawDelta, 0.0f, 0.0f);
    }
    if (scene == 1){
        modelMatrix2 = originModelMatrix2;
        modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(1.0f, 1.0f, 0.925f + abs(fmod(currentTime * 5.0f, 2.0f) - 1.0f) * 0.15f));
        modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(model2RotateSpeed * currentTime), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    if (scene == 2){
        modelMatrix2 = originModelMatrix2;
        modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-60.0f, 25.0f , 10.0f));
        modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(1.0f, 1.0f, 0.925f + abs(fmod(currentTime * 5.0f, 2.0f) - 1.0f) * 0.15f));
        modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(model2RotateSpeed * currentTime), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = originModelMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(15.0f, -5.0f ,0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(110.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        float timeInScene = currentTime - sceneStartTime;
        float duration = 4.0f;
        float t = glm::clamp(timeInScene / duration, 0.0f, 1.0f);
        camera.yaw = 110.0f - (20.0f * t);
        updateCamera();
    }
    if (scene == 3) {
        modelMatrix = originModelMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -3.0f));
        float timeInScene = currentTime - sceneStartTime;
        float duration = 3.0f;
        float t = glm::clamp(timeInScene / duration, 0.0f, 1.0f);
        camera.yaw = 120.0f - (60.0f * t);
        camera.radius = 100.0f;
        updateCamera();
    }
    if (scene == 4 || scene == 6){
        float timeInScene = currentTime - sceneStartTime;
        float shootDuration = 1.0f;
        static float rx, ry, rz;
        static int lastChangeTime = 0;
        static int changeInterval = 5;
        if(changeInterval < lastChangeTime){
            lastChangeTime = 0;
            rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360.0f;
            ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360.0f;
            rz = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360.0f;
        }else{
            lastChangeTime ++;
        }
        if (timeInScene <= shootDuration) {
            sphereT = timeInScene / shootDuration;
        } else {
            sphereT = 1.0f;
            staticModelStopped = true;
            sphereActive = false;
        }

        modelMatrix = originModelMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-50.0f, 25.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-100.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        modelMatrix2 = originModelMatrix2;
        modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-50.0f, -35.0f, 0.0f));
        if (staticModelStopped) { 
            if(timeInScene <= 2.0f){
                modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(300.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            }else{
                modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));
            }
        } else{
            modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(1.0f, 1.0f, 0.925f + abs(fmod(currentTime * 5.0f, 2.0f) - 1.0f) * 0.15f));
            modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(model2RotateSpeed * currentTime), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        glm::vec3 startPos = glm::vec3(120.0f, 10.0f, -30.0f);
        glm::vec3 endPos = glm::vec3(-120.0f, 10.0f, -30.0f);
        glm::vec3 currentSpherePos = glm::mix(startPos, endPos, sphereT);

        sphereMatrix = glm::mat4(1.0f);
        sphereMatrix = glm::translate(sphereMatrix, currentSpherePos);
        sphereMatrix = glm::scale(sphereMatrix, glm::vec3(20.0f));
    }
    if (scene == 5 || scene == 7){
        float timeInScene = currentTime - sceneStartTime;
        static float rx, ry, rz;
        static int lastChangeTime = 0;
        static int changeInterval = 5;
        float rotateDuration = 1.0f;
        if(changeInterval < lastChangeTime){
            lastChangeTime = 0;
            rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360.0f;
            ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360.0f;
            rz = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 360.0f;
        }else{
            lastChangeTime ++;
        }
        if(timeInScene <= rotateDuration){
            startCrash = false;
        }else{
            startCrash = true;
        }
        modelMatrix2 = originModelMatrix;
        modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-10.0f, 0.0f , 0.0f));
        if(!startCrash){
            modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));
        }else{
            float faceAngle = (scene == 7) ? 350.0f : 210.0f;
            modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(faceAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        // 觸發（例如 startCrash 的那一刻）
        if (scene == 5 && startCrash && !s5TriggeredExplode) {

            model2ExplodeActive = true;
            model2ExplodeStart  = currentTime;

            model2WireOnly     = false;
            model2WireCollapse = false;

            s5TriggeredExplode = true;
        }

        // 爆到最外圈：t >= explodeBlastTime
        if (model2ExplodeActive) {
            float t = currentTime - model2ExplodeStart;
            if (t >= explodeBlastTime + explodeHoldTime) {
                model2ExplodeActive = false;
                model2WireOnly     = true;   // 從現在開始不畫實體
                model2WireCollapse = true;   // 線條開始縮回
                model2EnableWire = true;
                model2WireStart    = currentTime;
            }
        }

        // 線條縮回結束（你可以選擇：縮回後保持線條，或恢復實體）
        if (model2WireCollapse) {
            float x = (currentTime - model2WireStart) / explodeReturnTime;
            if (x >= 1.0f) {
                model2WireCollapse = false;

                // 縮回後「保持線條」
                model2WireOnly = true;
                model2EnableWire   = true;   // 保持畫線

            }
        }

        if(scene == 7){
            float timeInScene = currentTime - sceneStartTime;
            float rotateDuration = 1.0f; // 你原本用來決定 startCrash 的時間

            bool startCrashLocal = (timeInScene > rotateDuration);

            // 1) 觸發：第七幕「一開始線條爆炸」
            if(startCrashLocal && !s7WireExploding && !s7Holding && !s7BecameModel1 && !s7Collapsing){
                s7WireExploding = true;
                s7WireStart = currentTime;

                model2ExplodeActive = false;
                model2WireOnly      = true;
                model2EnableWire    = true;
            }

            // 2) 爆到最大：進入 hold（不要立刻 swap）
            if(s7WireExploding){
                float t = currentTime - s7WireStart;
                if(t >= explodeBlastTime){
                    s7WireExploding = false;

                    s7Holding   = true;       // 進入 hold
                    s7HoldStart = currentTime;
                }
            }

            // 3) hold 結束：才 swap 成 model1，並開始縮回
            if(s7Holding){
                float th = currentTime - s7HoldStart;
                if(th >= explodeHoldTime){     // 你想停多久就改 explodeHoldTime
                    s7Holding = false;

                    s7BecameModel1 = true;         // 換 model1
                    s7Collapsing   = true;         // 開始縮回
                    s7CollapseStart = currentTime;
                }
            }

            // 4) 縮回結束：關掉 wire，回到正常 model1
            if(s7Collapsing){
                float x = (currentTime - s7CollapseStart) / explodeReturnTime;
                if(x >= 1.0f){
                    s7Collapsing = false;

                    model2EnableWire = false;
                    model2WireOnly   = false;

                }
            }
        }
    }
    if (scene == 8){
        modelMatrix2 = originModelMatrix2;
        modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(1.0f, 1.0f, 0.925f + abs(fmod(currentTime * 5.0f, 2.0f) - 1.0f) * 0.15f));
        modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-10.0f, 16.0f , 0.0f));
        modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(model2RotateSpeed * currentTime), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = originModelMatrix2;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 0.925f + abs(fmod(currentTime * 5.0f, 2.0f) - 1.0f) * 0.15f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-10.0f, -16.0f , 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(model2RotateSpeed * currentTime), glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

void render(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(camera.position - glm::vec3(0.0f, 0.2f, 0.1f), camera.position + camera.front, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f);

    const int scene = getScene();

    auto setCommonUniforms = [&](shader_program_t* sh){
        sh->set_uniform_value("view", view);
        sh->set_uniform_value("projection", projection);
        sh->set_uniform_value("viewPos", camera.position - glm::vec3(0.0f, 0.2f, 0.1f));

        // light
        sh->set_uniform_value("lightPos", light.position);
        sh->set_uniform_value("lightAmbient", light.ambient);
        sh->set_uniform_value("lightDiffuse", light.diffuse);
        sh->set_uniform_value("lightSpecular", light.specular);

        // material
        sh->set_uniform_value("matAmbient", material.ambient);
        sh->set_uniform_value("matDiffuse", material.diffuse);
        sh->set_uniform_value("matSpecular", material.specular);
        sh->set_uniform_value("matGloss", material.gloss);

        // texture sampler
        sh->set_uniform_value("texture_diffuse1", 0);
    };

    shader_program_t* defaultSh = shaderPrograms[0];
    shader_program_t* explodeSh = shaderPrograms[2];

    // =========================
    // 1) Draw model1 (always solid, default shader)
    // =========================
    if (scene == 7 && s7BecameModel1 && s7Collapsing) {
        // 這裡用 explode shader 讓 model1 維持爆開並縮回
        explodeSh->use();
        setCommonUniforms(explodeSh);
        explodeSh->set_uniform_value("model", modelMatrix2);

        float t;
        if (s7Collapsing) {
            float r = glm::clamp((currentTime - s7CollapseStart), 0.0f, explodeReturnTime);
            t = explodeBlastTime + explodeHoldTime + r; // 直接走 return 段
        } else {
            // 理論上不會進來（因為你 swap 後立刻進 collapsing），但保險
            t = explodeBlastTime + 1e-4f;
        }

        explodeSh->set_uniform_value("uTime", t);
        explodeSh->set_uniform_value("uBlastTime", explodeBlastTime);
        explodeSh->set_uniform_value("uHoldTime", explodeHoldTime);
        explodeSh->set_uniform_value("uReturnTime", explodeReturnTime);
        explodeSh->set_uniform_value("uStrength", explodeStrength);

        staticModel->draw();
        explodeSh->release();
    }
    if(scene == 2 || scene == 3 || scene == 4 || scene == 6 || scene == 8 || (scene == 7 && s7BecameModel1 && !s7Collapsing)){
        defaultSh->use();
        setCommonUniforms(defaultSh);
        defaultSh->set_uniform_value("model", (scene == 7 && s7BecameModel1) ? modelMatrix2 : modelMatrix);
        staticModel->draw();
        defaultSh->release();
    }

    if(scene == 8){
        defaultSh->use();
        setCommonUniforms(defaultSh);
        defaultSh->set_uniform_value("model", modelMatrix2);
        staticModel->draw();
        defaultSh->release();
    }

    // =========================
    // 2) Draw model2 (state-dependent)
    //    - exploding: explode shader
    //    - wireOnly:  do NOT draw solid
    //    - otherwise: default shader
    // =========================
    if(scene == 1 || scene == 2 || scene == 4 || scene == 5 || scene == 6 || (scene == 7 && !s7BecameModel1)){
        if (model2ExplodeActive) {
            explodeSh->use();
            setCommonUniforms(explodeSh);
            explodeSh->set_uniform_value("model", modelMatrix2);

            float t = currentTime - model2ExplodeStart;

            // 你想「爆到最外圈就換線條」：這裡建議把實體爆炸卡在 blastTime
            // （不跑回來那段）
            // 卡在 blast+hold 的尾端，避免進入 return 段（你等一下會切 wire）
            float cap = explodeBlastTime + std::max(0.0f, explodeHoldTime) - 1e-4f;
            // 避免 holdTime=0 時 cap < blast
            cap = std::max(cap, explodeBlastTime + 1e-4f);

            t = std::min(t, cap);

            explodeSh->set_uniform_value("uTime", t);
            explodeSh->set_uniform_value("uBlastTime", explodeBlastTime);
            explodeSh->set_uniform_value("uHoldTime", explodeHoldTime);
            explodeSh->set_uniform_value("uReturnTime", explodeReturnTime);
            explodeSh->set_uniform_value("uStrength", explodeStrength);

            staticModel2->draw();
            explodeSh->release();
        }
        else if (!model2WireOnly) {
            defaultSh->use();
            setCommonUniforms(defaultSh);
            defaultSh->set_uniform_value("model", modelMatrix2);
            staticModel2->draw();
            defaultSh->release();
        }
    }

    // =========================
    // 3) Draw sphere/cube (keep solid, default shader)
    // =========================
    if((scene == 4 || scene == 6) && sphereActive){
        defaultSh->use();
        setCommonUniforms(defaultSh);
        defaultSh->set_uniform_value("model", sphereMatrix);
        cubeModel->draw();
        defaultSh->release();
    }

    // =========================
    // 4) Cubemap environment (unchanged)
    // =========================
    cubemapShader->use();

    glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(view));
    cubemapShader->set_uniform_value("view", viewNoTranslate);
    cubemapShader->set_uniform_value("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    cubemapShader->set_uniform_value("skybox", 0);

    glBindVertexArray(cubemapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    cubemapShader->release();

    // =========================
    // 5) Wireframe overlay
    // =========================
    if (model2EnableWire && wireShader) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        glDisable(GL_DEPTH_TEST); // 想要 XRay 效果就一直 disable，想乾淨就改成條件式

        wireShader->use();
        wireShader->set_uniform_value("model", modelMatrix2);
        wireShader->set_uniform_value("view", view);
        wireShader->set_uniform_value("projection", projection);
        wireShader->set_uniform_value("uLineColor", wireColor);
        wireShader->set_uniform_value("uLineWidth", wireWidth);

        float amp = 0.0f;

        // ===== scene5：外圈 -> 0（縮回）=====
        if (scene == 5) {
            if (model2WireCollapse) {
                float x = (currentTime - model2WireStart) / explodeReturnTime;
                x = glm::clamp(x, 0.0f, 1.0f);
                float e = 1.0f - pow(1.0f - x, 3.0f);
                amp = explodeStrength * (1.0f - e);
            } else {
                amp = 0.0f;
            }
            wireShader->set_uniform_value("uAmp", amp);
            staticModel2->draw();
        }

        if (scene == 6) {
            amp = 0.0f;
            wireShader->set_uniform_value("uAmp", amp);
            staticModel2->draw();
        }

        // ===== scene7：爆到最大 / 換 model1 / 縮回 =====
        if (scene == 7) {
            if (!s7BecameModel1) {
                // 爆到最大（0 -> strength）
                if (s7WireExploding) {
                    float t = currentTime - s7WireStart;
                    float x = glm::clamp(t / explodeBlastTime, 0.0f, 1.0f);

                    // 跟 scene5 同款：爆出去「一開始快、後面慢」(ease-out cubic)
                    float e = 1.0f - pow(1.0f - x, 3.0f);

                    amp = explodeStrength * e;
                }
                // 最大停住
                else if (s7Holding) {
                    amp = explodeStrength;
                }
                else {
                    amp = 0.0f;
                }

                wireShader->set_uniform_value("uAmp", amp);
                staticModel2->draw();
            } 
        }

        wireShader->release();

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW4-group2", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    setup();
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        update(); 
        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete staticModel;
    delete staticModel2;
    delete cubeModel;
    for (auto shader : shaderPrograms) {
        delete shader;
    }
    delete cubemapShader;
    delete wireShader;

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec2 orbitInput(0.0f);
    float zoomInput = 0.0f;

    /*if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        orbitInput.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        orbitInput.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        orbitInput.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        orbitInput.y -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        zoomInput -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        zoomInput += 1.0f;*/
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        startAnimation = true;
    if (orbitInput.x != 0.0f || orbitInput.y != 0.0f || zoomInput != 0.0f) {
        float yawDelta = orbitInput.x * camera.orbitRotateSpeed * deltaTime;
        float pitchDelta = orbitInput.y * camera.orbitRotateSpeed * deltaTime;
        float radiusDelta = zoomInput * camera.orbitZoomSpeed * deltaTime;
        applyOrbitDelta(yawDelta, pitchDelta, radiusDelta);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_0 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
        shaderProgramIndex = 0;
    if (key == GLFW_KEY_1 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
        shaderProgramIndex = !shaderProgramIndex;
     if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        shaderProgramIndex = 2; // 切到 explosion shader
    }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

unsigned int loadCubemap(vector<std::string>& faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        stbi_set_flip_vertically_on_load(false);
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}  

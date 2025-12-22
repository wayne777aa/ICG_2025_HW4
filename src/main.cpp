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
bool startAnimation = true;

// explosion effect
bool explodeActive = false;
float explodeStart = 0.0f;

float explodeBlastTime = 0.6f;
float explodeHoldTime = 0.3f;
float explodeReturnTime = 1.0f;

float explodeStrength = 200.0f;

// wire frame mode
shader_program_t* wireShader = nullptr;
bool enableWire = false;
bool wireXRay = false; // 整個透出線框
float wireWidth = 1.0f;
glm::vec3 wireColor = glm::vec3(1.0f, 0.5f, 0.2f);
bool wireOnly = false;  // true: 只畫 wire，不畫原本 model

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
    //return 8;
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

    // ===== wireframe program (index = 6) =====
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
    std::string cubemapDir = "..\\..\\src\\asset\\texture\\skybox\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> faces
    {
        cubemapDir + "right.jpg",
        cubemapDir + "left.jpg",
        cubemapDir + "top.jpg",
        cubemapDir + "bottom.jpg",
        cubemapDir + "front.jpg",
        cubemapDir + "back.jpg"
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
    if(scene == 2){
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
    if(scene == 4 || scene == 6){
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
    if(scene == 5 || scene == 7){
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
            modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(210.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }
    if(scene == 8){
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

    if (!wireOnly) {
        // set matrix for view, projection, model transformation
        shaderPrograms[shaderProgramIndex]->use();
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", modelMatrix);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("view", view);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("projection", projection);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("viewPos", camera.position - glm::vec3(0.0f, 0.2f, 0.1f));

        // TODO: set additional uniform value for shader program
        // ===== TODO1: Load all uniform parameters =====

        // light
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightPos", light.position);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightAmbient", light.ambient);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightDiffuse", light.diffuse);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightSpecular", light.specular);

        // material
        shaderPrograms[shaderProgramIndex]->set_uniform_value("matAmbient", material.ambient);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("matDiffuse", material.diffuse);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("matSpecular", material.specular);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("matGloss", material.gloss);

        // For model texture sampler (if exists)
        shaderPrograms[shaderProgramIndex]->set_uniform_value("texture_diffuse1", 0);

        // explosion shader parameters
        if(shaderProgramIndex == 2){
            float t = explodeActive ? (currentTime - explodeStart) : 999.0f;
            shaderPrograms[shaderProgramIndex]->set_uniform_value("uTime", t);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("uBlastTime", explodeBlastTime);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("uHoldTime", explodeHoldTime);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("uReturnTime", explodeReturnTime);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("uStrength", explodeStrength);
            if (t > explodeBlastTime+explodeHoldTime+explodeReturnTime) explodeActive = false;
        }
    }
    // For model texture sampler (if exists)
    shaderPrograms[shaderProgramIndex]->set_uniform_value("texture_diffuse1", 0);

    // specifying sampler for shader program

    if(getScene() == 2 || getScene() == 3 || getScene() == 4 || getScene() == 6 || getScene() == 8){
        staticModel->draw();
    }
    shaderPrograms[shaderProgramIndex]->set_uniform_value("model", modelMatrix2);
    if(getScene() == 1 || getScene() == 2 || getScene() == 4 || getScene() == 5 || getScene() == 6 || getScene() == 7){
        staticModel2->draw();
    }
    if(getScene() == 8){
        staticModel->draw();
    }
    if(getScene() == 4 && sphereActive){
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", sphereMatrix);
        cubeModel->draw();
    }
    shaderPrograms[shaderProgramIndex]->release();

    // TODO 
    // Rendering cubemap environment
    // Hint:
    // 1. All the needed things are already set up in cubemap_setup() function.
    // 2. You can use the vertices in cubemapVertices provided in the header/cube.h
    // 3. You can use the cubemapShader to render the cubemap 
    //    (refer to the above code to get an idea of how to use the shader program)
    // ===== Render Cubemap Environment =====
    cubemapShader->use();

    // 1. remove translation from the view matrix
    glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(view));
    cubemapShader->set_uniform_value("view", viewNoTranslate);
    cubemapShader->set_uniform_value("projection", projection);

    // 2. bind skybox texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    cubemapShader->set_uniform_value("skybox", 0);  

    // 3. draw cube
    glBindVertexArray(cubemapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    cubemapShader->release();

    // ===== Render Wireframe Overlay =====
    if (enableWire && wireShader){
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 想更像光 → (SRC_ALPHA, ONE)
        glDepthMask(GL_FALSE);

        if (wireXRay) glDisable(GL_DEPTH_TEST);  // 透視線框（像你圖）
        else          glEnable(GL_DEPTH_TEST);   // 只畫表面線（會比較「乾淨」）

        wireShader->use();
        wireShader->set_uniform_value("model", modelMatrix);
        wireShader->set_uniform_value("view", view);
        wireShader->set_uniform_value("projection", projection);
        wireShader->set_uniform_value("uLineColor", wireColor);
        wireShader->set_uniform_value("uLineWidth", wireWidth);

        staticModel->draw();

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
        enableWire = false;
        wireOnly = false;
        explodeActive = true;
        explodeStart = glfwGetTime();
        shaderProgramIndex = 2; // 切到 explosion shader
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        if (!wireOnly) enableWire = !enableWire;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
        wireXRay = !wireXRay;
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        wireOnly = !wireOnly;            // 只畫 wire / 正常模式
        if (wireOnly) enableWire = true; // 進 wireOnly 時，強制確保 wire 開著
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

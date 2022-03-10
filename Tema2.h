#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/lab_camera.h"
#include <vector>
#include <string>

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3 &color);
        void Tema2::backupPosition(float& playerOffsetX, float& playerOffsetY, float& playerOffsetZ);
        //void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture1, Texture2D* texture2);
     protected:
        implemented_tema2::Camera *camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;

        // TODO(student): If you need any other class variables, define them here.
        glm::vec3 lightPosition;
        glm::vec3 lightDirection;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;
    };

    class Bullet 
    {
    public:
        float offsetBulletX;
        float offsetBulletY;
        float offsetBulletZ;
        float maxTime;
        float time;
        glm::vec3 bulletTrajectory; // forward ? 
        Bullet() : offsetBulletX(0), offsetBulletY(0), maxTime(0), time(0), bulletTrajectory(0) {}
        Bullet(float offsetBulletX, float offsetBulletY, float offsetBulletZ, glm::vec3 bulletTrajectory, float maxTime, float time = 0)
            : offsetBulletX(offsetBulletX), offsetBulletY(offsetBulletY), offsetBulletZ(offsetBulletZ), bulletTrajectory(bulletTrajectory), maxTime(maxTime), time(time) {}
     
    };

    //class Gun
    //{
    //public:
    //    //vector<m1::Bullet> bullets;
    //    vector<Bullet> bullets;
    //    string gunName;
    //    float maxTime;
    //    float cooldownBullet;
    //    float scaleBullet;
    //    bool enemyPiercing;
    //    Gun(float cooldownBullet) : cooldownBullet(cooldownBullet), gunName(gunName), scaleBullet(scaleBullet), enemyPiercing(enemyPiercing = false) {}
    //    Gun(vector<m1::Bullet> bullets, string gunName, float cooldownBullet, float maxTime, float  scaleBullet, bool enemyPiercing) : bullets(bullets), cooldownBullet(cooldownBullet), gunName(gunName), maxTime(maxTime), scaleBullet(scaleBullet), enemyPiercing(enemyPiercing) {}
    //    


    //};

    class Enemy
    {
    public:
        float offsetEnemyX;
        float offsetEnemyY;
        float offsetEnemyZ;
        glm::vec3 angleEnemy;
        float centralAngle;
        float cooldownAttack;
        float timeFromAttack;
        bool living;
        float timeFromDeath;
        float maxDeadTime;
        Enemy(float offsetEnemyX, float offsetEnemyY, float offsetEnemyZ, glm::vec3 angleEnemy, float centralAngle, float cooldownAttack, float timeFromAttack, bool living, float timeFromDeath, float maxDeadTime)
            : offsetEnemyX(offsetEnemyX), offsetEnemyY(offsetEnemyY),offsetEnemyZ(offsetEnemyZ), angleEnemy(angleEnemy), centralAngle(centralAngle),  cooldownAttack(cooldownAttack), timeFromAttack(timeFromAttack),
        living(living), timeFromDeath(timeFromDeath), maxDeadTime(maxDeadTime){}
       
    };

    /*class Player
    {
    public:
        int indexGun;
        vector<Gun> guns;
        float offsetPlayerX;
        float offsetPlayerY;
        float offsetHealthbarX;
        float offsetHealthbarY;
        float maxHealth;
        float health;
        float score;
        Player() : guns(), indexGun(0), offsetPlayerX(0), offsetPlayerY(0) {}
        Player(vector<Gun> guns, int indexGun, float offsetPlayerX, float offsetPlayerY, float offsetHealthbarX, float offsetHealthbarY, float maxHealth, float health)
            : guns(guns), indexGun(indexGun = 0), offsetPlayerX(offsetPlayerX), offsetPlayerY(offsetPlayerY), offsetHealthbarX(offsetHealthbarX), offsetHealthbarY(offsetHealthbarY), maxHealth(maxHealth), health(health) {}
        
    };*/
}   // namespace m1

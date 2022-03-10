#include "lab_m1/Tema2/Tema2.h"
#include "lab_m1/Tema2/maze/maze.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */
static float leftOrtho = 0.f;
static float rightOrtho = 2.f;
static float topOrtho = 2.f;
static float bottomOrtho = 0.f;

static float projectionMin = 0.01f;
static float projectionMax = 200.f;
static float projectionAngle = 60;

static Maze maze;
static int x_exit;
static int y_exit;


static int isMixing = 0;
static int spot = 0;
static float angle = RADIANS(30);

static vector<m1::Bullet> bullets;
static vector<m1::Enemy> enemies;
static vector<glm::vec3> healthPacks;
static glm::vec3 offsetPlayer;

static float WallSize = 4.0f;
static vector<glm::mat3> mapObjects;

static float bulletSize = 0.3;
static float playerRadius = 0.5;
static float enemyWidth = 0.2;
static float enemyHeight = 0.5;
static float playerHealth = 1000;
static float maxPlayerHealth = 1000;
static float playerAngleX;
static float playerAngleY;
static float simulationTime = 0;
static float maxMazeTime = 135;


static glm::mat3 bulletLights;

int checkAABBCollision(glm::vec3 objectAMin, glm::vec3 objectAMax, glm::vec3 objectBMin, glm::vec3 objectBMax);
int checkObjectCollision(glm::vec3 playerHitboxMin, glm::vec3 playerHitboxMax, vector<glm::mat3> mapObjects);
int checkObjectEnemyCollision(glm::vec3 playerHitboxMin, glm::vec3 playerHitboxMax, vector<m1::Enemy> enemies);


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{

	renderCameraTarget = true;

	// 
	// maze generator part
	// generate random exit , far from (0, 0) start point

	if (rand() % 2) { x_exit = maze.NumCells - 1; y_exit = rand() % maze.NumCells; }
	else { y_exit = maze.NumCells - 1; x_exit = rand() % maze.NumCells; }


	gen.seed(time(NULL));

	// clear maze
	std::fill(maze.g_Maze, maze.g_Maze + maze.NumCells * maze.NumCells, 0);

	// setup initial point
	maze.g_PtX = maze.RandomInt();
	maze.g_PtY = maze.RandomInt();

	// traverse
	maze.GenerateMaze();

	// prepare BGR image
	size_t DataSize = 3 * maze.ImageSize * maze.ImageSize;

	unsigned char* Img = new unsigned char[DataSize];

	memset(Img, 0, DataSize);

	// render maze on bitmap
	maze.RenderMaze(Img);

	maze.SaveBMP("Maze.bmp", Img, maze.ImageSize, maze.ImageSize);

	// save maze to hitbox objects map
	{

		for (int y = 0; y < maze.NumCells; y++)
		{
			for (int x = 0; x < maze.NumCells; x++)
			{

				// opening entrance and exit 
				char v = maze.g_Maze[y * maze.NumCells + x];
				if (x == 0 && y == 0)
					v = v | 8;
				if (x == x_exit && y == y_exit) {
					if (x_exit == maze.NumCells - 1) v = v | 2;
					else v = v | 4;
				}


				float nx = (float)x * WallSize + WallSize / 2; // x 
				float ny = (float)y * WallSize + WallSize / 2;

				if (!(v & maze.eDirection_Up)) {
					
					mapObjects.push_back(glm::mat3(glm::vec3(nx - WallSize / 2, 1 - 2 / 2, ny - WallSize / 2 - 0.1 / 2), glm::vec3(nx + WallSize / 2, 1 + 2 / 2, ny - WallSize / 2 + 0.1 / 2), glm::vec3(1)));
					
				}
				if (!(v & maze.eDirection_Right)) {
					
					mapObjects.push_back(glm::mat3(glm::vec3(nx + WallSize / 2 - 0.1f / 2, 1 - 2 / 2, ny - WallSize / 2), glm::vec3(nx + WallSize / 2 + 0.1f / 2, 1 + 2 / 2, ny + WallSize / 2), glm::vec3(1)));
					
				}
				if (!(v & maze.eDirection_Down)) {
					
					mapObjects.push_back(glm::mat3(glm::vec3(nx - WallSize / 2, 1 - 2 / 2, ny + WallSize / 2 - 0.1f / 2), glm::vec3(nx + WallSize / 2, 1 + 2 / 2, ny + WallSize / 2 + 0.1f / 2), glm::vec3(1)));
					
				}
				if (!(v & maze.eDirection_Left)) {
					
					mapObjects.push_back(glm::mat3(glm::vec3(nx - WallSize / 2 - 0.1f / 2, 1 - 2 / 2, ny - WallSize / 2), glm::vec3(nx - WallSize / 2 + 0.1f / 2, 1 + 2 / 2, ny + WallSize / 2), glm::vec3(1)));
					
				}
			}

		}

	}


	// inamic


	for (int i = 0; i < maze.NumCells * maze.NumCells / 3; i++) {
		float nx = rand() % maze.NumCells * WallSize + WallSize / 2; // 
		float ny = rand() % maze.NumCells * WallSize + WallSize / 2;
		enemies.push_back(Enemy(nx, 1.1, ny, glm::vec3(0), 0, 2, 0, true, 0.0f, 20.0f));
	}

	// power up health
	healthPacks.push_back(glm::vec3(-2, 1, -2));
	for (int i = 0; i < maze.NumCells * maze.NumCells / 5; i++) {
		float nx = rand() % maze.NumCells * WallSize + WallSize / 2; // Oz
		float ny = rand() % maze.NumCells * WallSize + WallSize / 2; // Ox
		healthPacks.push_back(glm::vec3(nx, 0.3, ny));
	}


	// camera 
	//renderCameraTarget = true;

	camera = new implemented_tema2::Camera();
	camera->Set(glm::vec3(0, 0.61, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("teapot");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "teapot.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("plane");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	
	{
		Shader* shader = new Shader("LabShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
	// shader inamic
	{
		Shader* shader = new Shader("EnemyShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "enemy_shader", "VertexShader.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "enemy_shader", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	// lumina si material
	{
		lightPosition = glm::vec3(0, 1, 1);
		lightDirection = glm::vec3(0, -1, 0);
		materialShininess = 30;
		materialKd = 1.5;
		materialKs = 0.5;
	}

	

	projectionMatrix = glm::perspective(RADIANS(projectionAngle), window->props.aspectRatio, projectionMin, projectionMax);


}


void Tema2::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
	simulationTime += deltaTimeSeconds;

	static int k = 0;
	if (simulationTime > k) {
		cout << "Health : " << playerHealth << endl;
		k += 5;
	}


	// check exit 
	{
		float playerOffsetX;
		float playerOffsetY;
		float playerOffsetZ;
		backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);
		glm::vec3 playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
		glm::vec3 playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

		glm::vec3 exitHitboxMin = glm::vec3(x_exit * WallSize , 1, y_exit * WallSize );
		glm::vec3 exitHitboxMax = glm::vec3(x_exit * WallSize + WallSize , 1, y_exit * WallSize + WallSize );
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(exitHitboxMin.x, exitHitboxMin.y, exitHitboxMin.z));
		glm::mat4 foobar = glm::mat4(1);
		foobar = glm::translate(foobar, glm::vec3(exitHitboxMax.x, exitHitboxMax.y, exitHitboxMax.z));
		//RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(0.75, 0.75, 0.75));
		//RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], foobar, glm::vec3(0.25, 0.75, 0.75));
		if (checkAABBCollision(playerHitboxMin, playerHitboxMax, exitHitboxMin, exitHitboxMax)) {
			cout << "You won! You've found the exit" << endl;
			exit(0);
			
		}
	}


	for (int i = 0; i < bullets.size(); i++) {
		bullets[i].time += deltaTimeSeconds;
		float speed = 20;

		bullets[i].offsetBulletX += deltaTimeSeconds * speed * bullets[i].bulletTrajectory.x;
		bullets[i].offsetBulletY += deltaTimeSeconds * speed * bullets[i].bulletTrajectory.y;
		bullets[i].offsetBulletZ += deltaTimeSeconds * speed * bullets[i].bulletTrajectory.z;



		glm::vec3 bulletHitboxMin = glm::vec3(bullets[i].offsetBulletX - bulletSize / 2, bullets[i].offsetBulletY - bulletSize / 2, bullets[i].offsetBulletZ - bulletSize / 2);
		glm::vec3 bulletHitboxMax = glm::vec3(bullets[i].offsetBulletX + bulletSize / 2, bullets[i].offsetBulletY + bulletSize / 2, bullets[i].offsetBulletZ + bulletSize / 2);
		for (int j = 0; j < mapObjects.size(); j++)
			if (checkAABBCollision(bulletHitboxMin, bulletHitboxMax, mapObjects[j][0], mapObjects[j][1])) {
				bullets[i].time = bullets[i].maxTime;
				break;
			}


		if (bullets[i].time >= bullets[i].maxTime) {
			bullets.erase(bullets.begin() + i);
			continue;
		}

		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(bullets[i].offsetBulletX, bullets[i].offsetBulletY, bullets[i].offsetBulletZ));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(bulletSize));

		/*if (window->KeyHold(GLFW_MOUSE_BUTTON_RIGHT)) {*/
			
			//if(window->KeyHold(GLFW_MOUSE_BUTTON_LEFT))
				RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(1, 0, 0));
		//}
	}

	// render maze
	for (int i = 0; i < mapObjects.size(); i++) {
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3((mapObjects[i][0].x + mapObjects[i][1].x) / 2, (mapObjects[i][0].y + mapObjects[i][1].y) / 2, (mapObjects[i][0].z + mapObjects[i][1].z) / 2));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(mapObjects[i][1].x - mapObjects[i][0].x, mapObjects[i][1].y - mapObjects[i][0].y, mapObjects[i][1].z - mapObjects[i][0].z));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.25, 0.75, 0.75));
	}


	// player
	
	if (renderCameraTarget)
	{
		// cap
		glm::mat4 modelMatrix = glm::mat4(1);

		modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition());
		modelMatrix = glm::rotate(modelMatrix, atan(-camera->forward.z / camera->forward.x), glm::vec3(0, 1, 0));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90.0f), glm::vec3(0, 1, 0));
		
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
		glm::mat4 ModelAuxiliar = modelMatrix;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.5f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.4f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.5f, 0));
		

		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.8, 0.3, 0.3));


		// trunchi
		
		modelMatrix = ModelAuxiliar;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -3.1, 0));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 3, 1));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));

		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 1, 0));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(1, 0, 0));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 1, 0));


		glm::mat4 model = modelMatrix; 
		// brate
		modelMatrix = glm::translate(modelMatrix, glm::vec3(1.1, 0, 0));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 0.25, 1));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
		

		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 3, 1));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 1, 0));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], glm::translate(modelMatrix, glm::vec3(-3.2, 0, 0)), glm::vec3(0, 1, 0));

		// picioare
		modelMatrix = glm::translate(model, glm::vec3(0, -1.02, 0));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1, (float)2.5 / 3, 1));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.4, 0.1, 1));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.05, 0, 0));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.4, 0.1, 1));




	}


	// enemy
	{


		for (int i = 0; i < enemies.size(); i++) {
			bool isDead = false;
			
			if(enemies[i].living) enemies[i].centralAngle += deltaTimeSeconds;
				
			enemies[i].timeFromAttack += deltaTimeSeconds;
			if (enemies[i].living == false) enemies[i].timeFromDeath += deltaTimeSeconds;

			if (enemies[i].timeFromDeath > enemies[i].maxDeadTime) {
				enemies.erase(enemies.begin() + i);
				continue;
			}

			// player - enemy collision 
			{
				glm::vec3 enemyHitboxMax = glm::vec3(enemies[i].offsetEnemyX + enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle), enemies[i].offsetEnemyY + enemyHeight / 2, enemies[i].offsetEnemyZ + enemyWidth + WallSize / 2 * sin(enemies[i].centralAngle));
				glm::vec3 enemyHitboxMin = glm::vec3(
					enemies[i].offsetEnemyX - enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle),
					enemies[i].offsetEnemyY - enemyHeight * 2,
					enemies[i].offsetEnemyZ - enemyWidth + 3 * WallSize / 8 * sin(enemies[i].centralAngle));


				float playerOffsetX;
				float playerOffsetY;
				float playerOffsetZ;
				backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);
				glm::vec3 playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
				glm::vec3 playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

				if (checkAABBCollision(playerHitboxMin, playerHitboxMax, enemyHitboxMin, enemyHitboxMax)) {
					enemies[i].centralAngle -= deltaTimeSeconds;

					if (enemies[i].timeFromAttack > enemies[i].cooldownAttack) {
						enemies[i].timeFromAttack = 0;


						playerHealth -= 200;
						if (playerHealth <= 0)
						{
							cout << "You are dead" << endl;
							exit(1);
						}
						else

							cout << playerHealth << endl;
					}
				}
			}

			

			for (int j = 0; j < bullets.size(); j++) {
				glm::vec3 bulletHitboxMin = glm::vec3(bullets[j].offsetBulletX - bulletSize / 2, bullets[j].offsetBulletY - bulletSize / 2, bullets[j].offsetBulletZ - bulletSize / 2);
				glm::vec3 bulletHitboxMax = glm::vec3(bullets[j].offsetBulletX + bulletSize / 2, bullets[j].offsetBulletY + bulletSize / 2, bullets[j].offsetBulletZ + bulletSize / 2);

				glm::vec3 enemyHitboxMin = glm::vec3(
					enemies[i].offsetEnemyX - enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle),
					enemies[i].offsetEnemyY - enemyHeight * 2,
					enemies[i].offsetEnemyZ - enemyWidth + 3 * WallSize / 8 * sin(enemies[i].centralAngle));

				glm::mat4 modelMatrix = glm::mat4(1);
				modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].offsetEnemyX - enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle), enemies[i].offsetEnemyY - enemyHeight * 2, enemies[i].offsetEnemyZ - enemyWidth + 3 * WallSize / 8 * sin(enemies[i].centralAngle)));
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1));
				RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3((float)177 / 255, (float)166 / 255, (float)150 / 255));

				glm::vec3 enemyHitboxMax = glm::vec3(enemies[i].offsetEnemyX + enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle), enemies[i].offsetEnemyY + enemyHeight / 2, enemies[i].offsetEnemyZ + enemyWidth + WallSize / 2 * sin(enemies[i].centralAngle));

				modelMatrix = glm::mat4(1);
				modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].offsetEnemyX + enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle), enemies[i].offsetEnemyY + enemyHeight / 2, enemies[i].offsetEnemyZ + enemyWidth + 3 * WallSize / 8 * sin(enemies[i].centralAngle)));
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1));
				RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3((float)177 / 255, (float)166 / 255, (float)150 / 255));


				if (checkAABBCollision(bulletHitboxMin, bulletHitboxMax, enemyHitboxMin, enemyHitboxMax)) {
					
					enemies[i].living = false;
					bullets[j].time = bullets[j].maxTime;
					isDead = true;
					break;
				}

			}
			if (isDead) continue;





			//cap
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].offsetEnemyX + 3 * WallSize / 8 * cos(enemies[i].centralAngle), enemies[i].offsetEnemyY, enemies[i].offsetEnemyZ + 3 * WallSize / 8 * sin(enemies[i].centralAngle)));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.12));
			glm::mat4 ModelAuxiliar = modelMatrix;
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.5f, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(2));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.5f, 0));

			
			string usedShader = enemies[i].living ? "LabShader" : "EnemyShader";
			RenderSimpleMesh(meshes["box"], shaders[usedShader], modelMatrix, glm::vec3((float)177 / 255, (float)166 / 255, (float)150 / 255));


			// trunchi
			modelMatrix = ModelAuxiliar;
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -6.1, 0));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(2, 6, 2.5));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], modelMatrix, glm::vec3(0, 0, 1));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(1, 0, 0));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], modelMatrix, glm::vec3(0, 0, 1));


			glm::mat4 Aux = modelMatrix; 
			// brate
			modelMatrix = glm::translate(modelMatrix, glm::vec3(1.1, 0, 0));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 0.5, 1));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], glm::translate(modelMatrix, glm::vec3(0, -0.1, 0)), glm::vec3((float)177 / 255, (float)166 / 255, (float)150 / 255));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], glm::translate(modelMatrix, glm::vec3(-3.2, -0.1, 0)), glm::vec3((float)177 / 255, (float)166 / 255, (float)150 / 255));


			modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 2.5, 1));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], modelMatrix, glm::vec3(0.85, 1, 1));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], glm::translate(modelMatrix, glm::vec3(-3.2, 0, 0)), glm::vec3(1, 1, 1));

			// picioare
			modelMatrix = glm::translate(Aux, glm::vec3(0, -1.02, 0));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.5f, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1, (float)2.5 / 3, 1));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], modelMatrix, glm::vec3(0, 0, 0.2));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.05, 0, 0));
			RenderSimpleMesh(meshes["box"], shaders[usedShader], modelMatrix, glm::vec3(0, 0.2, 0));
		}
	}

	{


		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
			// TODO(student): Add or change the object colors

			//RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(0.85, 0, 0));

		}

		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(2, 0.5f, 0));
			modelMatrix = glm::rotate(modelMatrix, RADIANS(60.0f), glm::vec3(1, 0, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
			// TODO(student): Add or change the object colors
			//RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.25, 0.75, 0.25));

		}

		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-3, 0.5f, 0));
			modelMatrix = glm::rotate(modelMatrix, RADIANS(60.0f), glm::vec3(1, 1, 0));
			//RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.25, 0.75, 0.75));
		}


		// Render maze ground
		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3((float)maze.NumCells / 2 * WallSize, 0.03f, (float)maze.NumCells / 2 * WallSize));
			RenderSimpleMesh(meshes["plane"], shaders["LabShader"], modelMatrix, glm::vec3((float)65 / 255, (float)66 / 255, (float)76 / 255));

		}

		// Render the point light in the scene
		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, lightPosition);
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
			//RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
		}

		{
			static float myangle = 0.0f;
			myangle += deltaTimeSeconds;

			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(+1 * cos(myangle), 1, 1 * sin(myangle)));

			//RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.25, 0.75, 0.25));
		}


	}

	


	// bonus : health pickup
	{
		
		static float healthAngle = 0;
		healthAngle += deltaTimeSeconds;
		for (int i = 0; i < healthPacks.size(); i++) {

			glm::vec3 playerHitboxMin;
			glm::vec3 playerHitboxMax;

		

			float playerOffsetX;
			float playerOffsetY;
			float playerOffsetZ;

			backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);

			playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
			playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

			glm::vec3 healthHitboxMin = glm::vec3(healthPacks[i].x - 0.2, healthPacks[i].y - 0.2, healthPacks[i].z - 0.2);
			glm::vec3 healthHitboxMax = glm::vec3(healthPacks[i].x + 0.2, healthPacks[i].y + 0.2, healthPacks[i].z + 0.2);
			if (checkAABBCollision(playerHitboxMin, playerHitboxMax, healthHitboxMin, healthHitboxMax)) {
				healthPacks.erase(healthPacks.begin() + i);
				playerHealth += 500;
				if (playerHealth < 0) playerHealth = 500;
				if (playerHealth > maxPlayerHealth) playerHealth = maxPlayerHealth;
				continue;
			}

			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(healthPacks[i].x, healthPacks[i].y, healthPacks[i].z));
			modelMatrix = glm::rotate(modelMatrix, healthAngle, glm::vec3(0, 1, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2));
			glm::mat4 fooMatrix = glm::scale(modelMatrix, glm::vec3(1, 2, 1));
			RenderSimpleMesh(meshes["box"], shaders["LabShader"], fooMatrix, glm::vec3(0.25, 0.75, 0.25));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(2, 1, 1));
			RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.25, 0.75, 0.25));

		}
	}


	
	// health bar

	{
		glm::ivec2 resolution = window->GetResolution();
		glViewport(resolution.x / 9, 0, resolution.x , resolution.y * 0.9);
		leftOrtho = 0;
		rightOrtho = 1;
		bottomOrtho = 0;
		topOrtho = 1;
		projectionMatrix = glm::ortho(leftOrtho, rightOrtho, bottomOrtho, topOrtho, -50.f, 500.f);
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition());
		modelMatrix = glm::translate(modelMatrix, camera->up);
		modelMatrix = glm::rotate(modelMatrix, atan(-camera->forward.z / camera->forward.x), glm::vec3(0, 1, 0));
		float healthScale = playerHealth > 0 ? 8 * playerHealth / maxPlayerHealth : 0;
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2, 0.2, healthScale));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
		
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.85, 0.25, 0.25));
		
		modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition());
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.95 * camera->up.x, camera->up.y * 0.95, 0.95 * camera->up.z));
		modelMatrix = glm::rotate(modelMatrix, atan(-camera->forward.z / camera->forward.x), glm::vec3(0, 1, 0));
		
		healthScale = simulationTime / maxMazeTime < 1 ? 8 -  8 * simulationTime / maxMazeTime : 0;
		if (8 - 8 * simulationTime / maxMazeTime < 0) {
			cout << "You lost, time ran out!" << endl;
			exit(1);
		}
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2, 0.1, healthScale));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
		RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.25, 0.25, 0.75));

		projectionMatrix = glm::perspective(RADIANS(projectionAngle), window->props.aspectRatio, projectionMin, projectionMax);
		
		glViewport(0, 0, resolution.x, resolution.y);
	}

	
}


void Tema2::FrameEnd()
{
	DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
	if (!mesh || !shader || !shader->program)
		return;

	// Render an object using the specified shader and the specified position
	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	mesh->Render();
}




void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// Render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Set shader uniforms for light properties
	int light_position = glGetUniformLocation(shader->program, "light_position");
	glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

	int light_direction = glGetUniformLocation(shader->program, "light_direction");
	glUniform3f(light_direction, lightDirection.x, lightDirection.y, lightDirection.z);

	// Set shader uniforms for bullets light 

	for (int i = 0; i < bullets.size() && i < 3; i++)
	{
		bulletLights[i].x = bullets[bullets.size() - 1 - i].offsetBulletX;
		bulletLights[i].y = bullets[bullets.size() - 1 - i].offsetBulletY;
		bulletLights[i].z = bullets[bullets.size() - 1 - i].offsetBulletZ;
	}

	int bullet_lights = glGetUniformLocation(shader->program, "bullet_lights");
	glUniformMatrix3fv(bullet_lights, 1, GL_FALSE, glm::value_ptr(bulletLights));




	// Set eye position (camera position) uniform
	glm::vec3 eyePosition = camera->GetTargetPosition();
	int eye_position = glGetUniformLocation(shader->program, "eye_position");
	glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

	// Set material property uniforms (shininess, kd, ks, object color) 
	int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(material_shininess, materialShininess);

	int material_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(material_kd, materialKd);

	int material_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(material_ks, materialKs);

	int object_color = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(object_color, color.r, color.g, color.b);

	// TODO(student): Set any other shader uniforms that you need


	// deformare enemy
	if (shader == shaders["EnemyShader"]) {
		glUniform1f(glGetUniformLocation(shader->program, "time"), (float)Engine::GetElapsedTime());
		// pana in momentul in care "moare", se foloseste Lab Shader, apoi fac deformarea prin EnemyShader

	}
	else {
		glUniform1f(glGetUniformLocation(shader->program, "time"), -1.0f);

	}




	// spotlight
	int spot_location = glGetUniformLocation(shader->program, "spot");
	glUniform1i(spot_location, spot);


	// angle
	int angle_location = glGetUniformLocation(shader->program, "angle");
	glUniform1f(angle_location, angle);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = camera->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix2 = projectionMatrix; // sau GetCameraScene()->ProjectionMatrix .//
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix2));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->m_VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}





/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		renderCameraTarget = true;
		float cameraSpeed = 5.0f;
		glm::vec3 playerHitboxMin;
		glm::vec3 playerHitboxMax;

		
		
		float playerOffsetX;
		float playerOffsetY;
		float playerOffsetZ;

		backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);


		if (window->KeyHold(GLFW_KEY_W)) {
			// TODO(student): Translate the camera forward
			//camera->TranslateForward(cameraSpeed * deltaTime);

			renderCameraTarget = true;

			playerOffsetX += camera->forward.x * cameraSpeed * deltaTime;
			playerOffsetY += camera->forward.y * cameraSpeed * deltaTime;
			playerOffsetZ += camera->forward.z * cameraSpeed * deltaTime;
			playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
			playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

			if (!checkObjectCollision(playerHitboxMin, playerHitboxMax, mapObjects) && !checkObjectEnemyCollision(playerHitboxMin, playerHitboxMax, enemies))
				camera->TranslateForward(cameraSpeed * deltaTime);
			backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);
		}

		if (window->KeyHold(GLFW_KEY_A)) {
			// TODO(student): Translate the camera to the left
			renderCameraTarget = true;
			playerOffsetX -= camera->right.x * cameraSpeed * deltaTime;
			playerOffsetY -= camera->right.y * cameraSpeed * deltaTime;
			playerOffsetZ -= camera->right.z * cameraSpeed * deltaTime;
			playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
			playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

			if (!checkObjectCollision(playerHitboxMin, playerHitboxMax, mapObjects))
				camera->TranslateRight(-cameraSpeed * deltaTime);
			backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);

		}

		if (window->KeyHold(GLFW_KEY_S)) {
			// TODO(student): Translate the camera backward
			renderCameraTarget = true;
			backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);
			playerOffsetX -= camera->forward.x * cameraSpeed * deltaTime;
			playerOffsetY -= camera->forward.y * cameraSpeed * deltaTime;
			playerOffsetZ -= camera->forward.z * cameraSpeed * deltaTime;
			playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
			playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

			if (!checkObjectCollision(playerHitboxMin, playerHitboxMax, mapObjects))
				camera->TranslateForward(-cameraSpeed * deltaTime);
		}

		if (window->KeyHold(GLFW_KEY_D)) {
			// TODO(student): Translate the camera to the right
			renderCameraTarget = true;
			backupPosition(playerOffsetX, playerOffsetY, playerOffsetZ);
			playerOffsetX += camera->right.x * cameraSpeed * deltaTime;
			playerOffsetY += camera->right.y * cameraSpeed * deltaTime;
			playerOffsetZ += camera->right.z * cameraSpeed * deltaTime;
			playerHitboxMin = glm::vec3(playerOffsetX - playerRadius, playerOffsetY - playerRadius, playerOffsetZ - playerRadius);
			playerHitboxMax = glm::vec3(playerOffsetX + playerRadius, playerOffsetY + playerRadius, playerOffsetZ + playerRadius);

			if (!checkObjectCollision(playerHitboxMin, playerHitboxMax, mapObjects))
				camera->TranslateRight(cameraSpeed * deltaTime);
		}

		//if (window->KeyHold(GLFW_KEY_Q)) {
		//	// TODO(student): Translate the camera downward
		//	camera->TranslateUpward(-cameraSpeed * deltaTime);
		//}

		//if (window->KeyHold(GLFW_KEY_E)) {
		//	// TODO(student): Translate the camera upward
		//	camera->TranslateUpward(cameraSpeed * deltaTime);
		//}
	}

	float speed = 2;
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 right = camera->right;
	glm::vec3 forward = camera->forward;
	forward = glm::normalize(glm::vec3(forward.x, 0, forward.z));

	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		renderCameraTarget = false;

		


		// Control light position using on W, A, S, D, E, Q
		if (window->KeyHold(GLFW_KEY_W)) lightPosition += forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_A)) lightPosition -= right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_S)) lightPosition -= forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_D)) lightPosition += right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_E)) lightPosition += up * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_Q)) lightPosition -= up * deltaTime * speed;

		// TODO(student): Set any other keys that you might need

	}


}


void Tema2::OnKeyPress(int key, int mods)
{
	//// Add key press event
	//if (key == GLFW_KEY_T)
	//{
	//	renderCameraTarget = !renderCameraTarget;

	//	if (renderCameraTarget)
	//		camera->setCameraThirdPerson();
	//	else
	//		camera->setCameraFirstPerson();
	//}
	//// TODO(student): Switch projections
	//if (key == GLFW_KEY_O) {

	//	projectionMatrix = glm::ortho(leftOrtho, rightOrtho, bottomOrtho, topOrtho, 0.f, 50.f);
	//}
	//if (key == GLFW_KEY_P) {
	//	projectionMatrix = glm::perspective(RADIANS(projectionAngle), window->props.aspectRatio, projectionMin, projectionMax);
	//}

	//if (window->KeyHold(GLFW_KEY_F)) {
	//	spot = (spot + 1) % 2;

	//}
}


void Tema2::OnKeyRelease(int key, int mods)
{
	// Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Add mouse move event

	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float sensivityOX = 0.001f;
		float sensivityOY = 0.001f;

		if (!renderCameraTarget) {

			// TODO(student): Rotate the camera in first-person mode around
			// OX and OY using `deltaX` and `deltaY`. Use the sensitivity
			// variables for setting up the rotation speed.
			camera->RotateFirstPerson_OX((-sensivityOX * deltaY));
			camera->RotateFirstPerson_OY(-sensivityOY * deltaX);

		}

		

	}

	if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {

		float sensivityOX = 0.001f;
		float sensivityOY = 0.003f;
		if (renderCameraTarget) {

			// TODO(student): Rotate the camera in third-person mode around
			// OX and OY using `deltaX` and `deltaY`. Use the sensitivity
			// variables for setting up the rotation speed.
			//camera->RotateThirdPerson_OX((-sensivityOX * deltaY));
			camera->RotateThirdPerson_OY(-sensivityOY * deltaX);

		}

	}
}




void Tema2::backupPosition(float& playerOffsetX, float& playerOffsetY, float& playerOffsetZ) {
	if (renderCameraTarget) {
		playerOffsetX = camera->GetTargetPosition().x;
		playerOffsetY = camera->GetTargetPosition().y;
		playerOffsetZ = camera->GetTargetPosition().z;
	}
	else {
		playerOffsetX = camera->position.x;
		playerOffsetY = camera->position.y;
		playerOffsetZ = camera->position.z;
	}
}

int checkObjectCollision(glm::vec3 playerHitboxMin, glm::vec3 playerHitboxMax, vector<glm::mat3> mapObjects) {

	for (int i = 0; i < mapObjects.size(); i++) {

		if (checkAABBCollision(playerHitboxMin, playerHitboxMax, mapObjects[i][0], mapObjects[i][1])) {
			return 1;
		}

	}

	return 0;

}

int checkObjectEnemyCollision(glm::vec3 playerHitboxMin, glm::vec3 playerHitboxMax, vector<m1::Enemy> enemies) {

	for (int i = 0; i < enemies.size(); i++) {
		glm::vec3 enemyHitboxMin = glm::vec3(
			enemies[i].offsetEnemyX - enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle),
			enemies[i].offsetEnemyY - enemyHeight * 2,
			enemies[i].offsetEnemyZ - enemyWidth + 3 * WallSize / 8 * sin(enemies[i].centralAngle));
		glm::vec3 enemyHitboxMax = glm::vec3(enemies[i].offsetEnemyX + enemyWidth + 3 * WallSize / 8 * cos(enemies[i].centralAngle), enemies[i].offsetEnemyY + enemyHeight / 2, enemies[i].offsetEnemyZ + enemyWidth + WallSize / 2 * sin(enemies[i].centralAngle));
		if (checkAABBCollision(playerHitboxMin, playerHitboxMax, enemyHitboxMin, enemyHitboxMax)) {

		


			return 1;

		}
	}
	return 0;
}


// objectA ( x,y,z ) 
int checkAABBCollision(glm::vec3 objectAMin, glm::vec3 objectAMax, glm::vec3 objectBMin, glm::vec3 objectBMax) {
	if (objectAMax.y >= objectBMin.y && objectAMin.y <= objectBMax.y)
		if (objectAMax.x >= objectBMin.x && objectAMin.x <= objectBMax.x)
			if (objectAMax.z >= objectBMin.z && objectAMin.z <= objectBMax.z)
				return 1;
	return 0;
}





void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button press event
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {

		if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {

			bullets.push_back(Bullet(camera->GetTargetPosition().x, camera->GetTargetPosition().y, camera->GetTargetPosition().z, camera->forward, 2, 0));
		}
	}
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}



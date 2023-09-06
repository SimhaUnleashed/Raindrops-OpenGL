//------------Computer Graphics Project ----------------------
// Raindrop Mathematical Game using OpenGL
// Designed And Developed by Sri Harsha Simha and Amulya C V
//------------------------------------------------------------
// This is a 3D game where the user can test their mathematical 
// abilities by solving the equations present in the rain drops
// and the score will be awarded based on the number of equations
// solved. The user will have 3 attempts before the rain drop 
// goes unsolved to the bottom of the screen after which the 
// game is over.  
//-------------------------------------------------------------



//------- Ignore this ----------
#include<filesystem>
namespace fs = std::filesystem;
//------------------------------

#include<GL/glut.h>
#include"Model.h"
#include"Camera.h"
#include"shader.h"

#include<windows.h>
#include<stdlib.h>
#include <cstdlib>




const unsigned int width = 1920;
const unsigned int height = 1080;

#include <ft2build.h>
#include FT_FREETYPE_H

#include"filesystem.h"





float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;

//Game logic variables
std::string userAnswer = "";
int gameStart = 0, score = 0, prevScore = 0, attempts = -1, rightAnsDrop = 0;
int num1[10], num2[10], symbol[10], ans[10], y[10];
int lb = 1, ub = 20, op1 = 1, op2 = 6; //lb lower bound ub upper bound (numbers 1 to 20)
// op1 =1 and op2 =4 for 1 = add 2= sub 3= multiply 4= divide
float speedCounter = 0.5; //for increasing speed as score increases



void RenderText(TextShader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
	// activate corresponding render state	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader.use();
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);


	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
	//glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	//glDepthMask(GL_TRUE);
}

void generateRandoms(int i) {
	num1[i] = (rand() % (ub - lb + 1)) + lb;

	num2[i] = (rand() % (ub - lb + 1)) + lb;
	symbol[i] = (rand() % (op1 - op2 + 1)) + op1;

	if (symbol[i] == 1) ans[i] = num1[i] + num2[i];
	else if (symbol[i] == 2) ans[i] = abs(num1[i] - num2[i]);
	else if (symbol[i] == 3) ans[i] = num1[i] * num2[i];
	else  ans[i] = (num1[i] >= num2[i]) ? round(num1[i] / num2[i]) : round(num2[i] / num1[i]);
}




int main() {
	// Initialize GLFW
	glfwInit();


	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a GLFW window named Raindrops
	GLFWwindow* window = glfwCreateWindow(width, height, "Raindrops", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, width, height);





	// Generates Shader objects
	Shader shaderProgram("default.vert", "default.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");

	TextShader shader("text.vs", "text.fs");
	glm::mat4 textprojection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "textprojection"), 1, GL_FALSE, glm::value_ptr(textprojection));


	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	skyboxShader.Activate();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);





	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);


	glm::vec3 position;

	Camera camera[10] = {
		Camera(width, height, glm::vec3(28.0f, -60.0f, 60.0f)),
		Camera(width, height, glm::vec3(21.0f, -25.0f, 80.0f)),
		Camera(width, height, glm::vec3(14.0f, -60.0f, 55.0f)),
		Camera(width, height, glm::vec3(7.0f, -55.0f, 65.0f)),
		Camera(width, height, glm::vec3(0.0f, -40.0f, 70.0f)),
		Camera(width, height, glm::vec3(-7.0f, -90.0f, 55.0f)),
		Camera(width, height, glm::vec3(-14.0f, -70.0f, 75.0f)),
		Camera(width, height, glm::vec3(-21.0f, -45.0f, 65.0f)),
		Camera(width, height, glm::vec3(-28.0f, -25.0f, 70.0f)),
		Camera(width, height, glm::vec3(-35.0f, -85.0f, 55.0f)),

	};





	// FreeType
	// --------
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	// find path to font
	std::string font_name = FileSystem::getPath("Resources/fonts/Montserrat-Medium.ttf");
	//std::cout << "font name " << font_name;
	if (font_name.empty())
	{
		std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
		return -1;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 128 characters of ASCII set
		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);



	/*camera[0] = Camera(width, height, glm::vec3(0.0f, -20.0f, 60.0f));*/



	/*
	* I'm doing this relative path thing in order to centralize all the resources into one folder and not
	* duplicate them between tutorial folders. You can just copy paste the resources from the 'Resources'
	* folder and then give a relative path from this folder to whatever resource you want to get to.
	* Also note that this requires C++17, so go to Project Properties, C/C++, Language, and select C++17
	*/
	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string modelPath = "/Resources/models/drop/scene.gltf";


	// Load in models
	Model model((parentDir + modelPath).c_str());




	// Variables to create periodic event for FPS displaying
	double prevTime = 0.0;
	double crntTime = 0.0;
	double timeDiff;
	// Keeps track of the amount of frames in timeDiff
	unsigned int counter = 0;

	// Use this to disable VSync (not advized)
	//glfwSwapInterval(0);


	// Create VAO, VBO, and EBO for the skybox
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// configure VAO/VBO for text quads
	// -----------------------------------
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// All the faces of the cubemap (make sure they are in this exact order)
	std::string facesCubemap[6] =
	{
		parentDir + "/Resources/skybox/right.jpg",
		parentDir + "/Resources/skybox/left.jpg",
		parentDir + "/Resources/skybox/top.jpg",
		parentDir + "/Resources/skybox/bottom.jpg",
		parentDir + "/Resources/skybox/front.jpg",
		parentDir + "/Resources/skybox/back.jpg"
	};



	// Creates the cubemap texture object
	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}



	//Mark the initial y position
	for (int i = 0;i < 10;i++) {
		y[i] = camera[i].Position.y;

	}

	for (int i = 0;i < 10;i++) {
		generateRandoms(i);

		
	}
	

	// Main while loop
	while (!glfwWindowShouldClose(window))
	{



		// Updates counter and times
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		counter++;


		//Position of texts relative to drops
		float posx[10] = { 350,610,625,810,950,1110,1190,1370,1470,1765 };
		float posy[10] = { 610,590,610,600,600,615,580,600,600,600};

		//

		if (timeDiff >= 1.0 / 30.0)
		{
			

			// Resets times and counter
			prevTime = crntTime;
			counter = 0;
			
			

			// Updates and exports the camera matrix to the Vertex Shader
			posy[0] += camera[0].Position.y * -21.5;
			posy[1] += camera[1].Position.y * -16;
			posy[2] += camera[2].Position.y * -23.5;
			posy[3] += camera[3].Position.y * -20;
			posy[4] += camera[4].Position.y * -18.5;
			posy[5] += camera[5].Position.y * -23.5;
			posy[6] += camera[6].Position.y * -17.5;
			posy[7] += camera[7].Position.y * -19.5;
			posy[8] += camera[8].Position.y * -18.5;
			posy[9] += camera[9].Position.y * -23.5;

			for (int i = 0;i < 10;i++) {
				//posy[0] += camera[0].Position.y * -18;
				//posy[i] += camera[i].Position.y * -16.5;

				//std::cout << camera[4].Position.y << "\n";
				if (camera[i].Position.y < 24) {
					camera[i].Position += camera[i].speed * camera[i].Up * speedCounter;
					
					//std::cout << abs(50 / camera[i].Position.z) <<"\n";
					camera[i].updateMatrix(45.0f, 10.1f, 200.0f);
					
					
				}
				else {
					/*camera[i].updateMatrix(0.0f, 10.1f, 200.0f);
					camera[i].Position.y = y[i];*/
					if (attempts > 0) {
						attempts--;  //Reduce attempts if the game is still ongoing

						for (int i = 0;i < 10;i++) {
							camera[i].Position.y = y[i] * 1.5; // Place the drops slightly above its previous position
							camera[i].updateMatrix(0.0f, 10.1f, 200.0f);// Make the drop disappear
							camera[i].speed -= 0.05; //Reduce the speed of drops so that the user can recover from the error

						}

						for (i = 0;i < 10;i++) {
							generateRandoms(i); // Generate new set of random numbers and symbols
						}
					}
					

				}
				
				

			}







			//// Clean the back buffer and depth buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Handles camera inputs (delete this if you have disabled VSync)

			for (int i = 0;i < 10;i++) {
				camera[i].Inputs(window);
			}

			// Draw the normal model
			for (int i = 0;i < 10;i++) {
				model.Draw(shaderProgram, camera[i]);
			}



			// Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
			glDepthFunc(GL_LEQUAL);
			//glDepthMask(GL_FALSE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



			skyboxShader.Activate();
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);

			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
				 view = glm::mat4(1.0f);
				 projection = glm::mat4(1.0f);
			}
			
			// We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
			// The last row and column affect the translation of the skybox (which we don't want to affect)
			for (int i = 0;i < 10;i++) {
				view = glm::mat4(glm::mat3(glm::lookAt(camera[i].Position, camera[i].Position + camera[i].Orientation, camera[i].Up)));
			}
			projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

			glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			//glUniformMatrix4fv(glGetUniformLocation(shader.ID, "textprojection"), 1, GL_FALSE, glm::value_ptr(textprojection));


			// Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
			// where an object is present (a depth of 1.0f will always fail against any object's depth value)

			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			// Switch back to the normal depth function
			glDepthFunc(GL_LESS);
			
			//Render score and attempts left texts
			if (attempts >= 0) {
				RenderText(shader, "Score :", 30, 1000, 0.7, glm::vec3(1.0, 1.0, 1.0));
				RenderText(shader, std::to_string(score), 150, 1000, 0.7, glm::vec3(1.0, 1.0, 1.0));
			}
			if (attempts == 3) {
				RenderText(shader, "Attempts Left :", 1590, 1000, 0.7, glm::vec3(0.0, 1.0, 0.0));
				RenderText(shader, std::to_string(attempts), 1850, 1000, 0.7, glm::vec3(0.0, 1.0, 0.0));
			}
			if (attempts == 2) {
				RenderText(shader, "Attempts Left :", 1590, 1000, 0.7, glm::vec3(1.5, 1.0, 0.0));
				RenderText(shader, std::to_string(attempts), 1850, 1000, 0.7, glm::vec3(1.5, 1.0, 0.0));
			}
			if (attempts == 1) {
				RenderText(shader, "Attempts Left :", 1590, 1000, 0.7, glm::vec3(1.0, 0.0, 0.0));
				RenderText(shader, std::to_string(attempts), 1850, 1000, 0.7, glm::vec3(1.0, 0.0, 0.0));
			}

			if (attempts == 0) {
				for (int i = 0;i < 10;i++) {
					camera[i].Position.y = 2000; //Make the drops disappear
				}

				RenderText(shader, "Game Over!", 700, 300, 2.0, glm::vec3(1.0, 0.0, 0.0));
				RenderText(shader, "Press Q to restart", 850, 220, 0.7, glm::vec3(1.0, 1.0, 1.0));
			}

			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
				if (attempts == -1) {
					attempts = 3;
					score = 0;
					for (int i = 0;i < 10;i++) {
						camera[i].speed =0.1; //Original speed
					}
					
				}
				if (attempts == 0) { //Since camera position is 2000 previously, it will auto reduce an attempt,
					//therefore attempts = 4;
					attempts = 4;
					score = 0;
					for (int i = 0;i < 10;i++) {
						camera[i].speed =0.1; //Original speed
					}
				}
			}

			//Remove characters from string when backspace is pressed
			if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
			{
				if(userAnswer.size()>=1)
				userAnswer.pop_back();
				glfwWaitEventsTimeout(500);

			}
			
			//Add appropriate characters to the string.
			if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
			{
					
					userAnswer += '0';
					glfwWaitEventsTimeout(500);
					
				
				}
			
			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			{
				userAnswer += '1';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
			{
				userAnswer += '2';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			{
				userAnswer += '3';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			{
				userAnswer += '4';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			{
				userAnswer += '5';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
			{
				userAnswer += '6';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
			{
				userAnswer += '7';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
			{
				userAnswer += '8';
				glfwWaitEventsTimeout(500);
			}
			if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
			{
				userAnswer += '9';
				glfwWaitEventsTimeout(500);
			}
			//Render the answer entered by user at the bottom
			RenderText(shader, userAnswer, 940, 100, 1.3, glm::vec3(1.0, 0.0, 0.0));
			int answer=-2000;

			//Convert string into integer 
			if(userAnswer!="")  answer = stoi(userAnswer);

			//Evaluate answer when enter is pressed
			if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
				for (int i = 0;i < 10;i++) {
				
					if (answer == ans[i]&& camera[i].Position.y>-30) {
					// Evaluate only for the drops visible on screen	
						score += 500;
						
						userAnswer = "";
						camera[i].updateMatrix(0.0f, 10.1f, 200.0f); //Make the drop disappear
						camera[i].Position.y = y[i]*1.25; // Reposition the drop slighly above
						generateRandoms(i); // Generate random symbols and numbers for that particular drop.
					}
				}
				glfwWaitEventsTimeout(500);
			}

			//Increment speed for every 2000 points
			if (prevScore!=score && score!=0&& score % 2000 == 0) {
				for (int i = 0;i < 10;i++) {
					camera[i].speed += 0.03;
				}
				prevScore = score;
			}

			//Give the 3d rotation effect as intro scene (manipulating position and orientation)
			if (attempts == -1) {
				float rot = -0.2;
				for(int i=0;i<10;i++){
					
					camera[i].Position += camera[i].speed * 2 * -glm::normalize(glm::cross(camera[i].Orientation, camera[i].Up));
					camera[i].Orientation = glm::rotate(camera[i].Orientation, glm::radians(rot), camera[i].Up);
				}
				
				RenderText(shader, "WELCOME TO RAINDROPS", 500, 300, 1.5, glm::vec3(0.0, 0.0, 0.8));
				RenderText(shader, "Press Q to start", 850, 220, 0.7, glm::vec3(1.0, 1.0, 1.0));
			}

			//Render equation on the drop
			if (attempts > 0) {
				for (int i = 0;i < 10;i++) {
					//Place the greater number on top to avoid ambiguity
					RenderText(shader, (num1[i] > num2[i]) ? std::to_string(num1[i]) : std::to_string(num2[i]), posx[i], posy[i], 0.55, glm::vec3(1.0, 1.0f, 1.0f));
					switch (symbol[i]) {
					case 1: RenderText(shader, "+", posx[i] - 20, posy[i] - 30, 0.55, glm::vec3(1.0, 1.0f, 1.0f));
						break;
					case 2: RenderText(shader, "-", posx[i] - 20, posy[i] - 30, 0.55, glm::vec3(1.0, 1.0f, 1.0f));
						break;
					case 3: RenderText(shader, "*", posx[i] - 20, posy[i] - 30, 0.55, glm::vec3(1.0, 1.0f, 1.0f));
						break;
					case 4: RenderText(shader, "/", posx[i] - 10, posy[i] - 30, 0.55, glm::vec3(1.0, 1.0f, 1.0f));
						break;
					}
					RenderText(shader, (num2[i] <= num1[i]) ? std::to_string(num2[i]) : std::to_string(num1[i]), posx[i], posy[i] - 30, 0.55, glm::vec3(1.0, 1.0f, 1.0f));
					posy[0] += camera[0].Position.y * -25;
				}
			}

			

			// Swap the back buffer with the front buffer
			glfwSwapBuffers(window);
			// Take care of all GLFW events
			glfwPollEvents();
		}

	}


		// Delete all the objects we've created
		shaderProgram.Delete();
		skyboxShader.Delete();

		// Delete window before ending the program
		glfwDestroyWindow(window);
		// Terminate GLFW before ending the program
		glfwTerminate();
		return 0;
	}

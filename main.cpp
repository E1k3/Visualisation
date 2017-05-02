#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <experimental/filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Data/ensemblemanager.h"
#include "logger.h"

using namespace std::literals::string_literals;
using namespace vis;

[[noreturn]] void glfw_error_callback(int error, const char* description)
{
	Logger::instance() << Logger::Severity::ERROR << "GLFW ERROR: " << error << " " << description << std::endl;

	// TODO:ERROR handling
	throw std::runtime_error("GLFW ERROR");
}

void glfw_framebuffsize_callback(GLFWwindow* window, int width, int height)
{
	(void)window; // UNUSED
	glViewport(0, 0, width, height);
}

struct Vec4
{
	Vec4()
	{

	}

	Vec4(float x, float y, float z, float w) :
		_x{x}, _y{y}, _z{z}, _w{w}
	{

	}

	float _x{0.f}, _y{0.f}, _z{0.f}, _w{0.f};
};

/**
 * @brief genGrid Generates a grid with points spanning from [0, 0] to [1, 1].
 * @param width Number of points per row.
 * @param height Number of points per column
 * @return A vector containing all points, row wise.
 */
std::vector<Vec4> genGrid(unsigned width, unsigned height)
{
	auto grid = std::vector<Vec4>(width * height);

	if(width*height == 0)
		return grid;	// TODO:error handling?

	for(unsigned row = 0; row < height; ++row)
	{
		for(unsigned col = 0; col < width; ++col)
		{
			float x = static_cast<float>(col)/(width-1);
			float y = static_cast<float>(row)/(height-1);
			grid[row*width + col] = Vec4{x*2 -1, y*2 -1, x, 1-y};
		}
	}
	return grid;
}

/**
 * @brief genIndices Generates indices to be used to draw a grid with element buffer.
 * @param width The width of the grid.
 * @param height The height of the grid.
 * @return A vector containing all indices.
 */
std::vector<unsigned> genIndices(unsigned width, unsigned height)
{
	auto indices = std::vector<unsigned>{};
	if(width*height <= 2)
		return indices;	// TODO:error handling?
	for(unsigned y = 0; y < height-1; ++y)
	{
		for(unsigned x = 0; x < width-1; ++x)
		{
			unsigned i = y*width + x;
			indices.push_back(i);
			indices.push_back(i+1);
			indices.push_back(i+width+1);
			indices.push_back(i);
			indices.push_back(i+width+1);
			indices.push_back(i+width);
		}
	}
	return indices;
}

int main(int argc, char *argv[])
{
	(void)argc;	// UNUSED
	(void)argv;	// UNUSED

	// Data root directory
	auto path = fs::path{"/home/eike/CurrentStuff/bachelor/weatherdata"};

	auto ensemblemngr = EnsembleManager(path);
	ensemblemngr.processSingleStep(256);
	auto step = ensemblemngr.currentStep();
	step.normaliseAll();


	// OpenGL context and window creation
	glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit())
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW init failed" << std::endl;
		// TODO:ERROR handling
		throw std::runtime_error("GLFW init failed");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Test", NULL, NULL);

	if(!window)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW window creation failed" << std::endl;
		// TODO:ERROR handling
		throw std::runtime_error("GLFW window creation failed");
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffsize_callback);

	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	if(status != GLEW_OK)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLEW init failed: "
						   << reinterpret_cast<const char*>(glewGetErrorString(status)) << std::endl;
		// TODO:ERROR handling
		throw std::runtime_error("GLEW init failed");
	}

	// Buffer
	GLuint vao, vbos[2], tex, vs, fs, program;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float quad[12] = {-1.f, 1.f, 1.f, -1.f, -1.f, -1.f,
					 -1.f, 1.f, 1.f, 1.f, 1.f, -1.f};
	float texcoords[12] = {0.0f, 1.0f, 1.f, 0.f, 0.f, 0.f,
						  0.0f, 1.0f, 1.f, 1.f, 1.f, 0.f};
	glGenBuffers(2, vbos);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, texcoords, GL_STATIC_DRAW);

	// Texture
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(step.xSize()), static_cast<int>(step.ySize()),
				 0, GL_RED, GL_FLOAT, static_cast<const void*>(step.scalarFieldStart(3))); // Target, level, int_format, width, height, border, format, data

	// Shader
	const char* vs_code = R"glsl(
		#version 330 core

		layout(location = 0) in vec2 position;
		layout(location = 1) in vec2 texcoords;
		out vec2 uv;

		void main()
		{
			gl_Position = vec4(position, 1.f, 1.f);
			uv = texcoords;
		}
		)glsl";
	const char* fs_code = R"glsl(
		#version 330

		in vec2 uv;
		out vec4 color;

		uniform sampler2D tex;

		void main()
		{
			color = vec4(texture(tex, uv).r, 0.f, 1-texture(tex, uv).r, 1.f);
		}
		)glsl";

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_code, nullptr);
	glCompileShader(vs);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_code, nullptr);
	glCompileShader(fs);

	GLint cstatus;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &cstatus);
	if(cstatus != GL_TRUE)
	{
		int logsize = 0;
		std::vector<char> log;

		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logsize);
		log.resize(logsize+1);
		glGetShaderInfoLog(vs, logsize+1, NULL, log.data());

		//Logging
		Logger::instance() << Logger::Severity::ERROR
						<< "Compiling shader \""
						<< vs
						<< "\" failed because of reason:\n"
						<< static_cast<char*>(&log[0])
				<< std::endl;

		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	glGetShaderiv(fs, GL_COMPILE_STATUS, &cstatus);
	if(cstatus != GL_TRUE)
	{
		int logsize = 0;
		std::vector<char> log;

		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &logsize);
		log.resize(logsize+1);
		glGetShaderInfoLog(fs, logsize+1, NULL, log.data());

		//Logging
		Logger::instance() << Logger::Severity::ERROR
						<< "Compiling shader \""
						<< fs
						<< "\" failed because of reason:\n"
						<< static_cast<char*>(&log[0])
				<< std::endl;

		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glBindFragDataLocation(program, 0, "color");
	glLinkProgram(program);
	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glClearColor(.2f, .2f, .2f, 1.f);

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// DESTRUCTION!!
	glDeleteProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glDeleteBuffers(2, vbos);
	glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();
}

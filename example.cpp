#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gl_letterbox.hpp"

#include <cassert>
#include <string>
#include <iostream>
#include <vector>

static const char* g_sprite_vert = R"""(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uViewProj;
uniform mat4 uModel;
uniform vec2 uTexSize;

out vec2 TexCoord;

void main() {
    vec4 _pos = vec4(aPos, 1.0);
    _pos.x *= uTexSize.x;
    _pos.y *= - uTexSize.y;
    gl_Position = ((uViewProj * uModel) * _pos);
    TexCoord = aTexCoord;
}
)""";

static const char* g_sprite_frag = R"""(
#version 330 core

in vec2 TexCoord;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
    FragColor = texture(uTexture, TexCoord);
}
)""";

class GLShaderProgram final {
private:
	GLuint m_ID = 0U;
public:
	GLShaderProgram() = default;
	GLShaderProgram(const std::string& _vert, const std::string& _frag) {
		const char* vShaderCode = _vert.data();
		const char* fShaderCode = _frag.data();
		unsigned int vertex, fragment;
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		this->_check_compile_errors(vertex, "VERTEX");
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		this->_check_compile_errors(vertex, "FRAGMENT");
		m_ID = glCreateProgram();
		glAttachShader(m_ID, vertex);
		glAttachShader(m_ID, fragment);
		glLinkProgram(m_ID);
		this->_check_compile_errors(m_ID, "PROGRAM");
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	~GLShaderProgram() {
		glDeleteProgram(m_ID);
	}
public:
	void use() const {
		glUseProgram(m_ID);
	}
	static void use_default() {
		glUseProgram(0);
	}
public:
	void set_uint(const std::string& _name, const uint32_t& _value) const {
		glUniform1ui(glGetUniformLocation(m_ID, _name.c_str()), _value);
	}
	void set_vec2(const std::string& _name, const float& _x, const float& _y) const {
		glUniform2f(glGetUniformLocation(m_ID, _name.c_str()), _x, _y);
	}
	void set_mat4(const std::string& _name, const float* const _value) const {
		glUniformMatrix4fv(glGetUniformLocation(m_ID, _name.c_str()), 1, GL_FALSE, _value);
	}
private:
	void _check_compile_errors(uint32_t _shader, const std::string& _type) const {
		int success;
		char infoLog[1024];
		if (_type != "PROGRAM") {
			glGetShaderiv(_shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(_shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << _type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else {
			glGetProgramiv(_shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(_shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << _type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};

class GLApp final {
private:
	gllb::LetterboxManager* m_letterboxManager = nullptr;
	float m_currentScreenScaleX = 1.0f;
	float m_currentScreenScaleY = 1.0f;
private:
	uint32_t m_fboID = 0U;
	uint32_t m_screenTextureID = 0U;
private:
	uint32_t         m_sprite_vaoID = 0U;
	uint32_t         m_sprite_vboID = 0U;
	uint32_t         m_sprite_eboID = 0U;
	GLShaderProgram* m_spriteShader = nullptr;
private:
	GLFWwindow* m_window = nullptr;
	uint32_t    m_windowOriginWidth = 0U;
	uint32_t    m_windowOriginHeight = 0U;
	uint32_t    m_windowWidth = 0U;
	uint32_t    m_windowHeight = 0U;
private:
	double      m_mousePosX = 0.0;
	double      m_mousePosY = 0.0;
	double      m_mouseVirtualPosX = 0.0;
	double      m_mouseVirtualPosY = 0.0;
private:
	std::vector<float> m_viewProjMatrix { };
	std::vector<float> m_modelMatrix { };
public:
	GLApp(int _window_width, int _window_height, const char* _window_title) 
		: m_windowWidth(_window_width), m_windowHeight(_window_height),
		  m_windowOriginWidth(_window_width), m_windowOriginHeight(_window_height)
	{
		glfwInit();
		m_window = glfwCreateWindow(_window_width, _window_height, _window_title, nullptr, nullptr);
		glfwMakeContextCurrent(m_window);
		assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) != 0);

		glfwSetWindowUserPointer(m_window, this);
		glfwSetWindowSizeCallback(m_window, GLApp::_on_window_resized);
		glfwSetCursorPosCallback(m_window, GLApp::_on_cursor_pos_changed);

		m_letterboxManager = new gllb::LetterboxManager(m_windowOriginWidth, m_windowOriginHeight);

		// Create Sprite shader
		m_spriteShader = new GLShaderProgram(g_sprite_vert, g_sprite_frag);

		// Generate Screen Texture
		glGenTextures(1, &m_screenTextureID);
		glBindTexture(GL_TEXTURE_2D, m_screenTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_windowOriginWidth, m_windowOriginHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Generate Frame Buffer
		glGenFramebuffers(1, &m_fboID);

		// Link Screen Texture to Frame Buffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screenTextureID, 0);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::vector<float> _sprite_vertices; {
			_sprite_vertices.emplace_back(-0.5f);
			_sprite_vertices.emplace_back(0.5f);
			_sprite_vertices.emplace_back(0.0f);
			_sprite_vertices.emplace_back(0.0f);
			_sprite_vertices.emplace_back(0.0f);

			_sprite_vertices.emplace_back(0.5f);
			_sprite_vertices.emplace_back(0.5f);
			_sprite_vertices.emplace_back(0.0f);
			_sprite_vertices.emplace_back(1.0f);
			_sprite_vertices.emplace_back(0.0f);

			_sprite_vertices.emplace_back(0.5f);
			_sprite_vertices.emplace_back(-0.5f);
			_sprite_vertices.emplace_back(0.0f);
			_sprite_vertices.emplace_back(1.0f);
			_sprite_vertices.emplace_back(1.0f);

			_sprite_vertices.emplace_back(-0.5f);
			_sprite_vertices.emplace_back(-0.5f);
			_sprite_vertices.emplace_back(0.0f);
			_sprite_vertices.emplace_back(0.0f);
			_sprite_vertices.emplace_back(1.0f);
		}

		std::vector<uint32_t> _sprite_indices; {
			_sprite_indices.emplace_back(0);
			_sprite_indices.emplace_back(1);
			_sprite_indices.emplace_back(2);

			_sprite_indices.emplace_back(2);
			_sprite_indices.emplace_back(3);
			_sprite_indices.emplace_back(0);
		}

		glGenVertexArrays(1, &m_sprite_vaoID);
		glGenBuffers(1, &m_sprite_vboID);
		glGenBuffers(1, &m_sprite_eboID);

		glBindVertexArray(m_sprite_vaoID);
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_sprite_vboID);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _sprite_vertices.size(), _sprite_vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sprite_eboID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * _sprite_indices.size(), _sprite_indices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3 + sizeof(float) * 2, (GLvoid*)(0));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3 + sizeof(float) * 2, (GLvoid*)(sizeof(float) * 3));
		}
		glBindVertexArray(0);

		// Setup 2D view & projection matrix
		m_viewProjMatrix = {
			2.0f / m_windowOriginWidth, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / m_windowOriginHeight, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f / (1000.0f - 0.1f), (0.1f) / (0.1f - 1000.0f),
			0.0f, 0.0f, 0.0f, 1.0f
		};

		// Setup model matrix
		m_modelMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
	~GLApp() {
		glDeleteVertexArrays(1, &m_sprite_vaoID);
		glDeleteBuffers(1, &m_sprite_vboID);
		glDeleteBuffers(1, &m_sprite_eboID);

		glDeleteTextures(1, &m_screenTextureID);
		glDeleteFramebuffers(1, &m_fboID);
		
		delete m_spriteShader;
		m_spriteShader = nullptr;

		delete m_letterboxManager;
		m_letterboxManager = nullptr;
		
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
	void run() {
		while (glfwWindowShouldClose(m_window) == GLFW_FALSE) {
			// Begin Virtual Drawing
			glBindFramebuffer(GL_FRAMEBUFFER, m_fboID); {
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glViewport(0, 0, m_windowOriginWidth, m_windowOriginHeight);
				// Draw Scene here
				{

				}
			}

			// Begin Actual Drawing
			glBindFramebuffer(GL_FRAMEBUFFER, 0); {
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glViewport(0, 0, m_windowWidth, m_windowHeight);
				// Draw Screen Texture
				{
					m_modelMatrix[0] = m_currentScreenScaleX;
					m_modelMatrix[5] = m_currentScreenScaleY;

					glDisable(GL_DEPTH_TEST);
					glDisable(GL_CULL_FACE);
					m_spriteShader->use();
					
					glActiveTexture(GL_TEXTURE0 + 0);
					glBindTexture(GL_TEXTURE_2D, m_screenTextureID);
					m_spriteShader->set_uint("uTexture", m_screenTextureID);
					m_spriteShader->set_vec2("uTexSize", static_cast<float>(m_windowOriginWidth), static_cast<float>(m_windowOriginHeight));
					m_spriteShader->set_mat4("uViewProj", m_viewProjMatrix.data());
					m_spriteShader->set_mat4("uModel", m_modelMatrix.data());

					glBindVertexArray(m_sprite_vaoID);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);

					glBindTexture(GL_TEXTURE_2D, 0);

					GLShaderProgram::use_default();
					glEnable(GL_DEPTH_TEST);
					glEnable(GL_CULL_FACE);
				}
			}

			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}
	}
public:
	static void _on_window_resized(GLFWwindow* _window, int _width, int _height) {
		GLApp* _instance = reinterpret_cast<GLApp*>(glfwGetWindowUserPointer(_window));
		_instance->m_letterboxManager->on_window_resized(_width, _height);
		_instance->m_letterboxManager->get_screen_scale(
			_instance->m_currentScreenScaleX,
			_instance->m_currentScreenScaleY
		);
		_instance->m_windowWidth = _width;
		_instance->m_windowHeight = _height;
	}
	static void _on_cursor_pos_changed(GLFWwindow* _window, double _x, double _y) {
		GLApp* _instance = reinterpret_cast<GLApp*>(glfwGetWindowUserPointer(_window));
		_instance->m_mousePosX = _x;
		_instance->m_mousePosY = _y;
		_instance->m_letterboxManager->get_virtual_mouse_position(
			_x, 
			_y,
			_instance->m_mouseVirtualPosX,
			_instance->m_mouseVirtualPosY
		);
		std::cout << "Origin mouse position = (" << _instance->m_mousePosX << ", " << _instance->m_mousePosY << ")\n";
		std::cout << "Virtual mouse position = (" << _instance->m_mouseVirtualPosX << ", " << _instance->m_mouseVirtualPosY << ")\n";
	}
};

int main() {
	GLApp(1280, 720, "GLApp").run();
}
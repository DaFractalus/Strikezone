#include "SafeGL.hpp"

namespace SGL
{
	// SafeGLContext
	SafeGLContext& IntializeSGLContext(bool is_main)
	{
		if (SafeGLContext::_sglcontext) throw std::logic_error("Duplicate SGL context initialization.");
		SafeGLContext::_sglcontext = std::make_unique<SafeGLContext>();
		auto lock = std::scoped_lock(SafeGLContext::_sglcontext->GLTaskMutex);
		SafeGLContext::threadHasCtx = true;
		if (is_main)
		{
			if (SafeGLContext::_mainsglcontext) throw std::logic_error("Duplicate main SGL context claim.");
			SafeGLContext::mainThreadId = SafeGLContext::_sglcontext->threadId;
			SafeGLContext::_mainsglcontext = SafeGLContext::_sglcontext.get();
			SafeGLContext::mainHasCtx = true;
		}
		return *SafeGLContext::_sglcontext;
	}

	SafeGLContext& context()
	{
		if (!SafeGLContext::threadHasCtx)
		{

			std::ostringstream oss; oss << "SGL context isn't initialized for thread id : " << std::this_thread::get_id();
			throw std::logic_error(oss.str());
		}
		return *SafeGLContext::_sglcontext;
	}

	SafeGLContext& maincontext()
	{
		if (!SafeGLContext::mainHasCtx)
		{
			std::ostringstream oss; oss << "main SGL context requested but not found for thread id : " << std::this_thread::get_id();
			throw std::logic_error(oss.str());
		}
		return *SafeGLContext::_mainsglcontext;
	}

	void DestroySGLContext()
	{
		if (!SafeGLContext::threadHasCtx)
		{
			std::ostringstream oss; oss << "Attempted destroying non-existing SGL context by thread: " << std::this_thread::get_id();
			throw std::logic_error(oss.str());
		}
		if (SafeGLContext::_sglcontext.get() == SafeGLContext::_mainsglcontext)
		{
			SafeGLContext::mainHasCtx = false;
			SafeGLContext::_mainsglcontext = nullptr;
		}
		SafeGLContext::threadHasCtx = false;
		SafeGLContext::_sglcontext = nullptr;

	}

	void PrintoutActiveCounts()
	{
		auto& ctx = context();
		std::cout << "	| --- Active Objects:\n";
		std::cout << "	| Vertex Arrays: " << ctx.ACTV_VertexArrays << "\n";
		std::cout << "	| Buffers: " << ctx.ACTV_Buffers << "\n";
		std::cout << "	| Textures: " << ctx.ACTV_Textures << "\n";
		std::cout << "	| Framebuffers: " << ctx.ACTV_Framebuffers << "\n";
		std::cout << "	| Renderbuffers: " << ctx.ACTV_Renderbuffers << "\n";
		std::cout << "	| Samplers: " << ctx.ACTV_Samplers << "\n";
		std::cout << "	| Queries: " << ctx.ACTV_Queries << "\n";
		std::cout << "	| Programs: " << ctx.ACTV_Programs << "\n";
		std::cout << "	| Shaders: " << ctx.ACTV_Shaders << "\n";
	}

	//-------------
	void Safe_GlGen(GLsizei n, GLuint* objects, std::string name, std::function<void(GLsizei, GLuint*)> glGenFunc, std::function<void(GLsizei, const GLuint*)> glDeleteFunc, int& ACTV_Counter)
	{
		for (int i = 0; i < n; i++)
		{
			if (objects[i] != 0) { std::cerr << "WARNING: Regenerating [" << name << "] ID before deletion: " << objects[i] << "\n";	glDeleteFunc(1, &objects[i]);	ACTV_Counter--; }
		}
		glGenFunc(n, objects);	ACTV_Counter += n;
	}

	void Safe_GlGenVertexArrays(GLsizei n, GLuint* arrays) { Safe_GlGen(n, arrays, "Vertex Arrays", glGenVertexArrays, glDeleteVertexArrays, context().ACTV_VertexArrays); }
	void Safe_GlGenBuffers(GLsizei n, GLuint* buffers) { Safe_GlGen(n, buffers, "Buffers", glGenBuffers, glDeleteBuffers, context().ACTV_Buffers); }
	void Safe_GlGenTextures(GLsizei n, GLuint* textures) { Safe_GlGen(n, textures, "Textures", glGenTextures, glDeleteTextures, context().ACTV_Textures); }
	void Safe_GlGenFramebuffers(GLsizei n, GLuint* framebuffers) { Safe_GlGen(n, framebuffers, "Framebuffers", glGenFramebuffers, glDeleteFramebuffers, context().ACTV_Framebuffers); }
	void Safe_GlGenRenderbuffers(GLsizei n, GLuint* renderbuffers) { Safe_GlGen(n, renderbuffers, "Renderbuffers", glGenRenderbuffers, glDeleteRenderbuffers, context().ACTV_Renderbuffers); }
	void Safe_GlGenSamplers(GLsizei n, GLuint* samplers) { Safe_GlGen(n, samplers, "Samplers", glGenSamplers, glDeleteSamplers, context().ACTV_Samplers); }
	void Safe_GlGenQueries(GLsizei n, GLuint* ids) { Safe_GlGen(n, ids, "Queries", glGenQueries, glDeleteQueries, context().ACTV_Queries); }
	void Safe_GlCreateProgram(GLuint* program) {
		if (*program != 0) { std::cerr << "WARNING: Creating [Program] ID before deletion.\n";	glDeleteProgram(*program); }
		*program = glCreateProgram();	context().ACTV_Programs++;
	}
	void Safe_GlCreateShader(GLenum type, GLuint* shader) {
		if (*shader != 0) { std::cerr << "WARNING: Creating [Shader] ID before deletion.";	glDeleteShader(*shader); }
		*shader = glCreateShader(type);	context().ACTV_Shaders++;
	}

	void Safe_GlDelete(GLsizei n, GLuint* objects, std::function<void(GLsizei, const GLuint*)> glDeleteFunc, int& ACTV_Counter) {
		for (GLsizei i = 0; i < n; i++)
		{
			if (objects[i] != 0) { glDeleteFunc(1, &objects[i]); objects[i] = 0;	ACTV_Counter--; }
		}
	}
	void Safe_GlDeleteVertexArrays(GLsizei n, GLuint* arrays) { Safe_GlDelete(n, arrays, glDeleteVertexArrays, context().ACTV_VertexArrays); }
	void Safe_GlDeleteBuffers(GLsizei n, GLuint* buffers) { Safe_GlDelete(n, buffers, glDeleteBuffers, context().ACTV_Buffers); }
	void Safe_GlDeleteTextures(GLsizei n, GLuint* textures) { Safe_GlDelete(n, textures, glDeleteTextures, context().ACTV_Textures); }
	void Safe_GlDeleteFramebuffers(GLsizei n, GLuint* framebuffers) { Safe_GlDelete(n, framebuffers, glDeleteFramebuffers, context().ACTV_Framebuffers); }
	void Safe_GlDeleteRenderbuffers(GLsizei n, GLuint* renderbuffers) { Safe_GlDelete(n, renderbuffers, glDeleteRenderbuffers, context().ACTV_Renderbuffers); }
	void Safe_GlDeleteSamplers(GLsizei n, GLuint* samplers) { Safe_GlDelete(n, samplers, glDeleteSamplers, context().ACTV_Samplers); }
	void Safe_GlDeleteQueries(GLsizei n, GLuint* ids) { Safe_GlDelete(n, ids, glDeleteQueries, context().ACTV_Queries); }
	void Safe_GlDeleteProgram(GLuint& program) {
		if (program != 0) { glDeleteProgram(program); program = 0; context().ACTV_Programs--; }
	}
	void Safe_GlDeleteShader(GLuint& shader) {
		if (shader != 0) { glDeleteShader(shader); shader = 0; context().ACTV_Shaders--; }
	}


	///////////
	void ProcessGLTask(GLTask& task) {

		auto& data = task.data;

		switch (task.type) {
		case GLTask::Type::Generate: {
			GLuint dataid = data->id.load();
			switch (data->type) {
			case GLObjectType::Buffer: Safe_GlGenBuffers(1, &dataid); break;
			case GLObjectType::VertexArray: Safe_GlGenVertexArrays(1, &dataid); break;
			case GLObjectType::Texture: Safe_GlGenTextures(1, &dataid); break;
			case GLObjectType::Framebuffer: Safe_GlGenFramebuffers(1, &dataid); break;
			case GLObjectType::Renderbuffer: Safe_GlGenRenderbuffers(1, &dataid); break;
			case GLObjectType::Sampler: Safe_GlGenSamplers(1, &dataid); break;
			case GLObjectType::Query: Safe_GlGenQueries(1, &dataid); break;
			}
			data->id.store(dataid);
			data->isReady = true;
			break;
		}

		case GLTask::Type::Delete: {
			GLuint dataid = data->id.load();
			switch (data->type) {
			case GLObjectType::Buffer: Safe_GlDeleteBuffers(1, &dataid); break;
			case GLObjectType::VertexArray: Safe_GlDeleteVertexArrays(1, &dataid); break;
			case GLObjectType::Texture: Safe_GlDeleteTextures(1, &dataid); break;
			case GLObjectType::Framebuffer: Safe_GlDeleteFramebuffers(1, &dataid); break;
			case GLObjectType::Renderbuffer: Safe_GlDeleteRenderbuffers(1, &dataid); break;
			case GLObjectType::Sampler: Safe_GlDeleteSamplers(1, &dataid); break;
			case GLObjectType::Query: Safe_GlDeleteQueries(1, &dataid); break;
			}
			data->id.store(dataid);
			break;
		}

		case GLTask::Type::Upload:
			if (!data->isReady) break;
			switch (data->type) {
			case GLObjectType::Buffer:
				glBindBuffer(data->bufferTarget, data->id);
				glBufferData(data->bufferTarget, data->rawUploadData.size(), data->rawUploadData.data(), data->bufferUsage);
				if (data->callback)
				{
					data->callback();
					data->callback = nullptr;
				}

				break;

			case GLObjectType::VertexArray:
				if (data->callback)
				{
					data->callback();
					data->callback = nullptr;
				}
				break;

			default: break;
			}
			break;
		}
	}

	void ProcessQueueGLTasks(double lastTime, int fps, bool limited)
	{
		auto& ctx = context();
		double currentTime = glfwGetTime();
		double frameTime = 1.f / fps;
		double elapsedTime;

		std::scoped_lock lock(ctx.GLTaskMutex);
		if (!(ctx.GU_GLTaskQueue.empty() && ctx.DE_GLTaskQueue.empty()))
		{
			do {
				if (!ctx.GU_GLTaskQueue.empty()) { ProcessGLTask(ctx.GU_GLTaskQueue.front()); ctx.GU_GLTaskQueue.pop(); }
				if (!ctx.DE_GLTaskQueue.empty()) { ProcessGLTask(ctx.DE_GLTaskQueue.front()); ctx.DE_GLTaskQueue.pop(); }

			} while (currentTime = glfwGetTime(), elapsedTime = currentTime - lastTime, (!limited || elapsedTime < frameTime) && !(ctx.GU_GLTaskQueue.empty() && ctx.DE_GLTaskQueue.empty()));
		}
	}
}
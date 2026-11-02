#pragma once
#ifndef SAFEGLFUNCS_HPP
#define SAFEGLFUNCS_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>

namespace SGL
{
	void Safe_GlGen(GLsizei n, GLuint* objects, std::string name, std::function<void(GLsizei, GLuint*)> glGenFunc, std::function<void(GLsizei, const GLuint*)> glDeleteFunc, int& ACTV_Counter);
	void Safe_GlGenVertexArrays(GLsizei n, GLuint* arrays);
	void Safe_GlGenBuffers(GLsizei n, GLuint* buffers);
	void Safe_GlGenTextures(GLsizei n, GLuint* textures);
	void Safe_GlGenFramebuffers(GLsizei n, GLuint* framebuffers);
	void Safe_GlGenRenderbuffers(GLsizei n, GLuint* renderbuffers);
	void Safe_GlGenSamplers(GLsizei n, GLuint* samplers);
	void Safe_GlGenQueries(GLsizei n, GLuint* ids);
	void Safe_GlCreateProgram(GLuint* program);
	void Safe_GlCreateShader(GLenum type, GLuint* shader);

	void Safe_GlDelete(GLsizei n, GLuint* objects, std::function<void(GLsizei, const GLuint*)> glDeleteFunc, int& ACTV_Counter);
	void Safe_GlDeleteVertexArrays(GLsizei n, GLuint* arrays);
	void Safe_GlDeleteBuffers(GLsizei n, GLuint* buffers);
	void Safe_GlDeleteTextures(GLsizei n, GLuint* textures);
	void Safe_GlDeleteFramebuffers(GLsizei n, GLuint* framebuffers);
	void Safe_GlDeleteRenderbuffers(GLsizei n, GLuint* renderbuffers);
	void Safe_GlDeleteSamplers(GLsizei n, GLuint* samplers);
	void Safe_GlDeleteQueries(GLsizei n, GLuint* ids);
	void Safe_GlDeleteProgram(GLuint& program);
	void Safe_GlDeleteShader(GLuint& shader);

	////////////////////////
	enum class GLObjectType {
		VertexArray,
		Buffer,
		Texture,
		Framebuffer,
		Renderbuffer,
		Sampler,
		Query,
	};

	struct GLObjectData {
		std::atomic<GLuint> id = 0;
		GLObjectType type;
		std::atomic<bool> isReady = false;

		// Uploading VBO/VAO data
		std::vector<uint8_t> rawUploadData;
		GLenum bufferTarget = GL_ARRAY_BUFFER;
		GLenum bufferUsage = GL_STATIC_DRAW;
		std::function<void()> callback = nullptr; // lambda to set layout after VAO is bound or update new meshsize
		operator GLuint() const { return id.load(); }
	};

	struct GLTask {
		enum class Type { Generate, Delete, Upload };
		Type type;
		std::shared_ptr<GLObjectData> data;
	};

	//SafeGLContext
	class SafeGLContext
	{
	public:
		inline static std::thread::id mainThreadId;
		inline static SafeGLContext* _mainsglcontext = nullptr;
		inline static std::atomic<bool> mainHasCtx = false;
		inline static thread_local std::unique_ptr<SafeGLContext> _sglcontext = nullptr;
		inline static thread_local std::atomic<bool> threadHasCtx = false;

	public:
		std::thread::id threadId = std::this_thread::get_id();
		std::queue<GLTask> GU_GLTaskQueue;
		std::queue<GLTask> DE_GLTaskQueue;
		std::mutex GLTaskMutex;

		int ACTV_VertexArrays = 0;
		int ACTV_Buffers = 0;
		int ACTV_Textures = 0;
		int ACTV_Framebuffers = 0;
		int ACTV_Renderbuffers = 0;
		int ACTV_Samplers = 0;
		int ACTV_Queries = 0;
		int ACTV_TansformFeedbacks = 0;
		int ACTV_Programs = 0;
		int ACTV_Shaders = 0;

		~SafeGLContext()
		{
			auto lock = std::scoped_lock(GLTaskMutex);
		}
	};

	SafeGLContext& IntializeSGLContext(bool is_main);
	SafeGLContext& context();
	SafeGLContext& maincontext();
	void DestroySGLContext();
	void PrintoutActiveCounts();

	inline bool IsMainThread() { return std::this_thread::get_id() == SafeGLContext::mainThreadId; }
	inline bool IsThreadSGLCInit() { return SafeGLContext::threadHasCtx; }

	///////////////////////
	void ProcessGLTask(GLTask& task);

	inline void EnqueueGLGenerate(const std::shared_ptr<GLObjectData>& data) {
		GLTask task{ GLTask::Type::Generate, data };
		if (IsThreadSGLCInit()) { ProcessGLTask(task); }
		else if (SafeGLContext::mainHasCtx) {
			auto& ctx = maincontext();
			std::scoped_lock lock(ctx.GLTaskMutex); ctx.GU_GLTaskQueue.push(task);
		}
	}

	inline void EnqueueGLDelete(const std::shared_ptr<GLObjectData>& data) {
		GLTask task{ GLTask::Type::Delete, data };
		if (IsThreadSGLCInit()) { ProcessGLTask(task); }
		else if (SafeGLContext::mainHasCtx) {
			auto& ctx = maincontext();
			std::scoped_lock lock(ctx.GLTaskMutex); ctx.GU_GLTaskQueue.push(task);
		}
	}

	inline void EnqueueGLUpload(const std::shared_ptr<GLObjectData>& data) {
		GLTask task{ GLTask::Type::Upload, data };
		if (IsThreadSGLCInit()) { ProcessGLTask(task); }
		else if (SafeGLContext::mainHasCtx) {
			auto& ctx = maincontext();
			std::scoped_lock lock(ctx.GLTaskMutex); ctx.GU_GLTaskQueue.push(task);
		}
	}

	class SmartGLID {
	public:
		SmartGLID(GLObjectType type, GLenum bufferUsage = GL_STATIC_DRAW) {
			data = std::make_shared<GLObjectData>();
			data->type = type;
			data->bufferUsage = bufferUsage;
			EnqueueGLGenerate(data);
		}

		SmartGLID(const SmartGLID&) = default;
		SmartGLID& operator=(const SmartGLID&) = default;

		~SmartGLID() {
			if (data.use_count() == 1 && data->isReady) {
				EnqueueGLDelete(data);
			}
		}

		template<typename T>
		void uploadBufferData(const std::vector<T>& datavector, GLenum target, GLenum usage, std::function<void()> uploadCallback = {})
		{
			const size_t byteSize = datavector.size() * sizeof(T);
			data->rawUploadData.resize(byteSize);
			if (byteSize != 0) std::memcpy(data->rawUploadData.data(), datavector.data(), byteSize);

			data->bufferTarget = target;
			data->bufferUsage = usage;
			data->callback = std::move(uploadCallback);

			EnqueueGLUpload(data);
		}

		void setVAOLayout(std::function<void()> layoutFunc)
		{
			data->callback = layoutFunc;
			EnqueueGLUpload(data);
		}

		operator GLuint() const { return data->id.load(); }
		bool ready() const { return data->isReady; }

		std::shared_ptr<GLObjectData> data;
	};

	// ========== MAIN THREAD GL TASK PROCESSOR ========== //
	void ProcessQueueGLTasks(double lastTime, int fps, bool limited);

}

#endif
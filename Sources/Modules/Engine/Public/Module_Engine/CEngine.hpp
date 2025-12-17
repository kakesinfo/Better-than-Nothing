#pragma once
#include "IEngineApplication.hpp"

namespace BetterThanNothing
{
	class CRenderCache;
	class CRenderList;
	class CRenderer;
	class CCamera;

	struct VulkanContext;

	class CEngine
	{
	public:
		explicit CEngine(IEngineApplication* application);
		~CEngine() = default;

	private:
		void _onUpdate(float deltaTime);
		void _onRender();

	public:
		void Run();

	private:
		std::unique_ptr<VulkanContext>	m_pVulkanContext;
		std::unique_ptr<CRenderCache>	m_pRenderCache;
		std::unique_ptr<CRenderList>	m_pRenderList;
		std::unique_ptr<CRenderer>		m_pRenderer;
		std::unique_ptr<CCamera>		m_pCamera;

		IEngineApplication*				m_pApplication;
	};
};

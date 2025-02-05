#pragma once

#include "pch.h"

#include "Core.h"

#include "log.h"
#include "Device.h"

namespace Luxel
{
	struct PipelineConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		ui32 subpass = 0;
	};

	class LUXEL_API RenderPipeline
	{
	public:
		RenderPipeline(Device* const d, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		~RenderPipeline();
		RenderPipeline(const RenderPipeline&) = delete;
		void operator=(const RenderPipeline&) = delete;

		static PipelineConfigInfo DefaultPipelineConfigInfo(ui32 width, ui32 height);

	private:
		static std::vector<char> readFile(const std::string& filePath);
		
		void CreateGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		Device* const device;

		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule, fragShaderModule;
	};
}
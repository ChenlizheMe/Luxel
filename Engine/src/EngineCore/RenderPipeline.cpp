#include "pch.h"

#include "RenderPipeline.h"

namespace Luxel
{
	PipelineConfigInfo RenderPipeline::DefaultPipelineConfigInfo(ui32 width, ui32 height)
	{
		Info("Use default pipeline config info.");
		PipelineConfigInfo config{};
		
		// vertex input
		config.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		config.inputAssemblyInfo.pNext = nullptr;
		config.inputAssemblyInfo.flags = 0;

		// viewport
		config.viewport.x = 0.f;
		config.viewport.y = 0.f;
		config.viewport.width = static_cast<float>(width);
		config.viewport.height = static_cast<float>(height);
		config.viewport.minDepth = 0.f;
		config.viewport.maxDepth = 1.f;

		config.scissor.offset = { 0, 0 };
		config.scissor.extent = { width, height };
		
		config.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		config.viewportInfo.viewportCount = 1;
		config.viewportInfo.scissorCount = 1;
		config.viewportInfo.pViewports = &config.viewport;
		config.viewportInfo.pScissors = &config.scissor;

		// rasterization
		config.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		config.rasterizationInfo.depthClampEnable = VK_FALSE;
		config.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		config.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		config.rasterizationInfo.lineWidth = 1.0f;
		config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		config.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		config.rasterizationInfo.depthBiasEnable = VK_FALSE;
		config.rasterizationInfo.depthBiasConstantFactor = 0.f;
		config.rasterizationInfo.depthBiasClamp = 0.f;
		config.rasterizationInfo.depthBiasSlopeFactor = 0.f;

		// multisample
		config.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		config.multisampleInfo.sampleShadingEnable = VK_FALSE;
		config.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config.multisampleInfo.minSampleShading = 1.0f;
		config.multisampleInfo.pSampleMask = nullptr;
		config.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		config.multisampleInfo.alphaToOneEnable = VK_FALSE;

		// color blend
		config.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		config.colorBlendAttachment.blendEnable = VK_FALSE;
		config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		config.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		config.colorBlendInfo.logicOpEnable = VK_FALSE;
		config.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		config.colorBlendInfo.attachmentCount = 1;
		config.colorBlendInfo.pAttachments = &config.colorBlendAttachment;
		config.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		config.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		config.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		config.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		config.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		config.depthStencilInfo.depthTestEnable = VK_TRUE;
		config.depthStencilInfo.depthWriteEnable = VK_TRUE;
		config.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		config.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		config.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		config.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		config.depthStencilInfo.stencilTestEnable = VK_FALSE;
		config.depthStencilInfo.front = {};  // Optional
		config.depthStencilInfo.back = {};   // Optional

		config.pipelineLayout = nullptr;
		config.renderPass = nullptr;
		config.subpass = 0;

		return config;
	}

	std::vector<char> RenderPipeline::readFile(const std::string& filePath)
	{
		std::ifstream file{
			filePath, std::ios::ate | std::ios::binary
		};

		if (!file.is_open()) {
			Fatal("The file:", filePath, "does not exist.");
			throw std::runtime_error("failed to open file.");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		return buffer;
	}

	void RenderPipeline::CreateGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo)
	{
		Info("Create graphics pipeline.");

		if (configInfo.pipelineLayout == VK_NULL_HANDLE) {
			Error("Cannot create graphics pipeline: no pipelineLayout provided.");
			throw std::runtime_error("Cannot create graphics pipeline: no pipelineLayout provided.");
		}
		if (configInfo.renderPass == VK_NULL_HANDLE) {
			Error("Cannot create graphics pipeline: no renderPass provided.");
			throw std::runtime_error("Cannot create graphics pipeline: no renderPass provided.");
		}

		auto vert = readFile(vertPath);
		auto frag = readFile(fragPath);
		Info("Vertex shader size:", vert.size());
		Info("Fragment shader size:", frag.size());

		CreateShaderModule(vert, &vertShaderModule);
		CreateShaderModule(frag, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shadersCreateInfo[2];
		shadersCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shadersCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shadersCreateInfo[0].module = vertShaderModule;
		shadersCreateInfo[0].pName = "main";
		shadersCreateInfo[0].pNext = nullptr;
		shadersCreateInfo[0].pSpecializationInfo = nullptr;

		shadersCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shadersCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shadersCreateInfo[1].module = fragShaderModule;
		shadersCreateInfo[1].pName = "main";
		shadersCreateInfo[1].pNext = nullptr;
		shadersCreateInfo[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shadersCreateInfo;
		pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineCreateInfo.pViewportState = &configInfo.viewportInfo;
		pipelineCreateInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineCreateInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineCreateInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineCreateInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineCreateInfo.pDynamicState = nullptr;

		// pipelineCreateInfo.layout = configInfo.pipelineLayout;
		// pipelineCreateInfo.renderPass = configInfo.renderPass;
		// pipelineCreateInfo.subpass = configInfo.subpass;

		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pNext = nullptr;

		if (device == nullptr || device->GetDevice() == VK_NULL_HANDLE) {
			Error("Invalid Vulkan device.");
			throw std::runtime_error("Invalid Vulkan device.");
		}
		if (vkCreateGraphicsPipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			Error("Failed to create graphics pipeline.");
			throw std::runtime_error("Failed to create graphics pipeline.");
		}
	}

	void RenderPipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const ui32*>(code.data());

		if (vkCreateShaderModule(device->GetDevice(), &shaderModuleCreateInfo, nullptr, shaderModule) != VK_SUCCESS) {
			Error("Failed to create shader module.");
			throw std::runtime_error("Feiled to create shader module.");
		}
	}

	RenderPipeline::RenderPipeline(Device* const d, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo) : device{d}
	{
		CreateGraphicsPipeline(vertPath, fragPath, configInfo);
	}

	RenderPipeline::~RenderPipeline()
	{
		Info("Destory render pipeline.");
		vkDestroyShaderModule(device->GetDevice(), vertShaderModule, nullptr);
		vkDestroyShaderModule(device->GetDevice(), fragShaderModule, nullptr);
		vkDestroyPipeline(device->GetDevice(), graphicsPipeline, nullptr);
	}
}

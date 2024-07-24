#include "engine.h"
#include "stb_image.h"

using namespace ae::gui;


ImTextureID Engine::createTexture(const uint16_t width, const uint16_t height)
{
    vk::BufferCreateInfo bufferInfo{
        .size = vk::DeviceSize(width * height * 4),
        .usage = vk::BufferUsageFlagBits::eTransferSrc
    };
    vma::AllocationCreateInfo bufferAllocInfo{
        .usage = vma::MemoryUsage::eCpuOnly
    };
    auto buffer = _allocator.createBuffer(bufferInfo, bufferAllocInfo);


    vk::ImageCreateInfo imageCreateInfo{
            .flags = vk::ImageCreateFlags(),
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D(width, height, 1),
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
    };

    vma::AllocationCreateInfo allocationCreateInfo{
        .usage = vma::MemoryUsage::eGpuOnly
    };
    auto image = _allocator.createImage(imageCreateInfo, allocationCreateInfo);

    vk::ImageViewCreateInfo imageViewCreateInfo{
    .image = image.first,
    .viewType = vk::ImageViewType::e2D,
    .format = vk::Format::eR8G8B8A8Unorm,
    .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    vk::ImageView imageView = _device.createImageView(imageViewCreateInfo);

    vk::SamplerCreateInfo samplerCreateInfo{
        .flags = vk::SamplerCreateFlags(),
        .magFilter = vk::Filter::eNearest,
        .minFilter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eNearest,
        .addressModeU = vk::SamplerAddressMode::eClampToEdge,
        .addressModeV = vk::SamplerAddressMode::eClampToEdge,
        .addressModeW = vk::SamplerAddressMode::eClampToEdge,
        .mipLodBias = 0.0f,
        .anisotropyEnable = false,
        .maxAnisotropy = 1.0f,
        .compareEnable = false,
        .compareOp = vk::CompareOp::eNever,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eFloatOpaqueWhite };
    vk::Sampler sampler = _device.createSampler(samplerCreateInfo);

    vk::CommandPool pool = _device.createCommandPool(vk::CommandPoolCreateInfo{
        .queueFamilyIndex = _graphicsQueueFamily });
    vk::CommandBuffer cbuffer = _device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
        .commandPool = pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1 }).front();

    cbuffer.begin(vk::CommandBufferBeginInfo());
    vk::AccessFlags sourceAccessMask = vk::AccessFlagBits();
    vk::PipelineStageFlags sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    vk::AccessFlags destinationAccessMask = vk::AccessFlagBits::eTransferWrite;
    vk::PipelineStageFlags destinationStage = vk::PipelineStageFlagBits::eTransfer;
    vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
    vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
    vk::ImageMemoryBarrier    imageMemoryBarrier{
        .srcAccessMask = sourceAccessMask,
        .dstAccessMask = destinationAccessMask,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eTransferDstOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image.first,
        .subresourceRange = imageSubresourceRange };
    cbuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);
    vk::BufferImageCopy copyRegion{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1},
        .imageExtent = vk::Extent3D(width, height, 1)
    };
    cbuffer.copyBufferToImage(buffer.first, image.first, vk::ImageLayout::eTransferDstOptimal, { copyRegion });
    //vk::CopyCommandTransformInfoQCOM aaa{ .transform = vk::SurfaceTransformFlagBitsKHR::eRotate90 };
    //vk::BufferImageCopy2 cp{
    //    .pNext = static_cast<void*>(&aaa),
    //    .bufferOffset = 0,
    //    .bufferRowLength = 0,
    //    .bufferImageHeight = 0,
    //    .imageSubresource = {
    //        .aspectMask = vk::ImageAspectFlagBits::eColor,
    //        .mipLevel = 0,
    //        .baseArrayLayer = 0,
    //        .layerCount = 1},
    //    .imageExtent = vk::Extent3D(width, height, 1)
    //};
    //std::vector<vk::BufferImageCopy2> ii = { cp };
    //vk::CopyBufferToImageInfo2 i{
    //    .srcBuffer = buffer.first,
    //    .dstImage = image.first,
    //    .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
    //    .regionCount = 1,
    //    .pRegions = ii.data()
    //};
    //cbuffer.copyBufferToImage2(i);
    vk::ImageMemoryBarrier imageMemoryBarrier2{
        .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        .dstAccessMask = vk::AccessFlagBits::eShaderRead,
        .oldLayout = vk::ImageLayout::eTransferDstOptimal,
        .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image.first,
        .subresourceRange = imageSubresourceRange };
    cbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imageMemoryBarrier2);
    cbuffer.end();

    vk::DescriptorSet textureID = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    _textures.insert(std::pair<ImTextureID, Texture>(textureID, Texture{
        .size = vk::Extent2D(width, height),
        .buffer = buffer.first,
        .bufferAllocation = buffer.second,
        .image = image.first,
        .imageAllocation = image.second,
        .imageView = imageView,
        .imageSampler = sampler,
        .commandPool = pool,
        .commandBuffer = cbuffer
    }));
    return textureID;
}

void Engine::freeTexture(const Texture& texture)
{
    ImGui_ImplVulkan_RemoveTexture(texture.descriptorID);
    _device.freeCommandBuffers(texture.commandPool, { texture.commandBuffer });
    _device.destroyCommandPool(texture.commandPool);
    _device.destroySampler(texture.imageSampler);
    _device.destroyImageView(texture.imageView);
    _allocator.destroyImage(texture.image, texture.imageAllocation);
    _allocator.destroyBuffer(texture.buffer, texture.bufferAllocation);
}

void Engine::removeTexture(const ImTextureID& textureID)
{
    Texture texture = _textures[textureID];
    freeTexture(texture);
    _textures.erase(textureID);
}

void Engine::fillTextureFromBuffer(const ImTextureID& textureID, const unsigned char* src)
{
    Texture texture = _textures[textureID];
    void* data = _allocator.mapMemory(texture.bufferAllocation);

    unsigned char* pImageMemory = static_cast<unsigned char*>(data);
    unsigned char* pSrc = const_cast<unsigned char*>(src);
    for (uint32_t row = 0; row < texture.size.height; row++)
    {
        for (uint32_t col = 0; col < texture.size.width; col++)
        {
            pImageMemory[col * 4] = *(src++);
            pImageMemory[col * 4 + 1] = *(src++);
            pImageMemory[col * 4 + 2] = *(src++);
            pImageMemory[col * 4 + 3] = *(src++);
        }
//        pImageMemory += texture.rowPitch;
        pImageMemory += 4 * texture.size.width;
    }
    _allocator.unmapMemory(texture.bufferAllocation);

    vk::Fence fence = _device.createFence(vk::FenceCreateInfo());
    _graphicsQueue.submit(vk::SubmitInfo{
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &texture.commandBuffer }, fence);
    while (vk::Result::eTimeout == _device.waitForFences(fence, VK_TRUE, 100000000));
    _device.destroyFence(fence);
}

ImTextureID Engine::createTextureFromFile(const string& filename)
{
    int width, height;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, 0, 4);
    if (data == nullptr)
        throw std::runtime_error("Unable to load file");
    ImTextureID textureID = createTexture(width, height);
    fillTextureFromBuffer(textureID, data);
    stbi_image_free(data);
    return textureID;
}
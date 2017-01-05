#include <geVuSG/SceneManager.h>

#include <geVuSG/Drawable.h>

using namespace ge::vusg;
using namespace ge::vu;
using namespace ge::sg;
using namespace std;

ge::vusg::SceneManager::SceneManager(ge::vu::DeviceContextShared & deviceContext)
  :deviceContext(deviceContext) {
}

DrawableShared ge::vusg::SceneManager::getDrawable(ge::sg::Mesh * mesh) {
  auto drawableIt = drawables.find(mesh);
  if (drawableIt != drawables.end()) {
    return drawableIt->second;
  }

  processMesh(mesh);
  return drawables.find(mesh)->second;
}

vk::DescriptorSet ge::vusg::SceneManager::getDescriptor(vk::PipelineLayout pipelineLayout, ge::sg::MaterialImageComponent * img){
  auto desc = descriptors.find(make_tuple(pipelineLayout, img));
  if (desc != descriptors.end()) {
    return desc->second;
  }

  


  return descriptors.find(make_tuple(pipelineLayout, img))->second;
}

void ge::vusg::SceneManager::processMesh(ge::sg::Mesh * mesh) {
  deviceContext->info("SceneManager::processMesh", mesh);
  DrawableShared drawable = make_shared<Drawable>();
  VertexArrayShared vao = make_shared<VertexArray>();

  for (auto &a : mesh->attributes) {
    auto buffer = make_shared<Buffer>(deviceContext);
    
    // @todo handle stride and offset
    assert(a->offset == 0);
    assert(a->stride == 0);
    
    if (a->semantic == AttributeDescriptor::Semantic::indices) {
      buffer->createIndexBuffer(a->size);
      vao->setIndexBuffer(buffer);

      drawable->indexed = true;
      drawable->vertexCount = a->size / a->getSize(a->type);
    } else {
      buffer->createVertexBuffer(a->size);
      if (a->semantic == AttributeDescriptor::Semantic::position) {        
        vao->addVertexBuffer(semanticToBinding(a->semantic), buffer, 0);
        if (!drawable->indexed)drawable->vertexCount = a->size / (a->getSize(a->type)*a->numComponents);
      }

      if (a->semantic == AttributeDescriptor::Semantic::normal) {
        vao->addVertexBuffer(semanticToBinding(a->semantic), buffer, 0);
      }

      if (a->semantic == AttributeDescriptor::Semantic::texcoord) {
        vao->addVertexBuffer(semanticToBinding(a->semantic), buffer, 0);
      }
    }
    buffer->setSubData(a->data.get(), a->size, 0);
    buffers[a.get()] = buffer;
  }

  drawable->vertexArray = vao;
  drawable->primitive = sgPrimitiveToVkPrimitive(mesh->primitive);

  drawables[mesh] = drawable;

  deviceContext->info(drawable->vertexCount);
}

int ge::vusg::SceneManager::semanticToBinding(ge::sg::AttributeDescriptor::Semantic semantic) {
  switch (semantic) {
  case ge::sg::AttributeDescriptor::Semantic::position: return 0;
  case ge::sg::AttributeDescriptor::Semantic::normal: return 1;
  case ge::sg::AttributeDescriptor::Semantic::texcoord: return 2;
  case ge::sg::AttributeDescriptor::Semantic::binormal: return 3;
  case ge::sg::AttributeDescriptor::Semantic::tangent: return 4;
  case ge::sg::AttributeDescriptor::Semantic::color: return 5;
  }
  return -1;
}

vk::PrimitiveTopology ge::vusg::SceneManager::sgPrimitiveToVkPrimitive(ge::sg::Mesh::PrimitiveType type){
  switch(type) {
  
  case ge::sg::Mesh::PrimitiveType::LINES: return vk::PrimitiveTopology::eLineList;  
  case ge::sg::Mesh::PrimitiveType::LINE_STRIP: return vk::PrimitiveTopology::eLineStrip;
  case ge::sg::Mesh::PrimitiveType::PATCH: return vk::PrimitiveTopology::ePatchList;
  case ge::sg::Mesh::PrimitiveType::POINTS: return vk::PrimitiveTopology::ePointList;  
  
  case ge::sg::Mesh::PrimitiveType::TRIANGLES: return vk::PrimitiveTopology::eTriangleList;
  case ge::sg::Mesh::PrimitiveType::TRIANGLE_FAN: return vk::PrimitiveTopology::eTriangleFan;
  case ge::sg::Mesh::PrimitiveType::TRIANGLE_STRIP: return vk::PrimitiveTopology::eTriangleStrip;
  
  case ge::sg::Mesh::PrimitiveType::LINE_LOOP:
  case ge::sg::Mesh::PrimitiveType::POLYGON:
  case ge::sg::Mesh::PrimitiveType::QUADS:
  case ge::sg::Mesh::PrimitiveType::QUAD_STRIP:
  case ge::sg::Mesh::PrimitiveType::UNKNOWN: 
  default:
    // unsupported primitives
    return vk::PrimitiveTopology::ePointList;
  }
}